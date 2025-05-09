local logger = require("logger")

---@class property_map.Configuration.ClassMember
---@field package name string 成员名称
---@field package name_in_lua string Lua中的成员名称
---@field package value number 枚举值
---@field package final_state number 【内部使用】最终状态值

---@class property_map.Configuration
---@field package namespace string[] C++命名空间，列表为空则生成到全局
---@field package class_name string 枚举类名称
---@field package class_members property_map.Configuration.ClassMember[] 枚举类成员
---@field package function_name string 转换函数名称

---@class property_map
local property_map = {}

local magic_value = 0x0d000721

---@class property_map.Configuration
local Configuration = {}
---@package
function Configuration:constructor()
    self.namespace = {}
    self.class_name = ""
    self.class_members = {}
    self.function_name = ""
end
---@param ... string C++命名空间，列表为空则生成到全局
---@return property_map.Configuration
function Configuration:setNamespace(...)
    local args = {...}
    for i, v in ipairs(args) do
        assert(type(v) == "string", string.format("invalid namespace identifier #%d ('%s')", i, v))
    end
    self.namespace = args
    return self
end
---@param class_name string
---@return property_map.Configuration
function Configuration:setClassName(class_name)
    self.class_name = class_name
    return self
end
---@param name string 成员名称
---@param value number? 枚举值
---@param name_in_lua string? Lua中的成员名称
---@return property_map.Configuration
function Configuration:addClassMember(name, value, name_in_lua)
    ---@type property_map.Configuration.ClassMember
    local class_member = {
        name = name,
        name_in_lua = name_in_lua or name,
        value = value or magic_value,
        final_state = magic_value,
    }
    table.insert(self.class_members, class_member)
    return self
end
---@param function_name string 转换函数名称
---@return property_map.Configuration
function Configuration:setFunctionName(function_name)
    self.function_name = function_name
    return self
end

local metatable = { __index = Configuration }

---@return property_map.Configuration
function property_map.Configuration()
    ---@type property_map.Configuration
    ---@diagnostic disable-next-line: missing-fields
    local instance = {}
    setmetatable(instance, metatable)
    instance:constructor()
    return instance
end

---@param s string
---@return fun(): number?, string?
local function characters(s)
    local l = s:len()
    local i = 0
    return function()
        i = i + 1
        if i <= l then
            return i, s:sub(i, i)
        else
            return nil, nil
        end
    end
end

--- 生成头文件
---@param configuration property_map.Configuration
---@param file_name? string 输出文件名，默认与类名相同
---@param directory? string 输出文件夹，默认位于当前工作目录
local function generateHeaderFile(configuration, file_name, directory)
    -- 计算最小值，用于未知值

    local min_value = 0x7fffffff
    for _, member in ipairs(configuration.class_members) do
        min_value = math.min(min_value, member.value)
    end

    -- 打开文件

    local has_namespace = #configuration.namespace > 0
    local auto_file_name = file_name or configuration.class_name
    local macro_name = ""
    if has_namespace then
        macro_name = string.format("_%s_%s_h_", table.concat(configuration.namespace, "_"), auto_file_name)
    else
        macro_name = string.format("_%s_h_", auto_file_name)
    end
    local f = assert(io.open(string.format("%s/%s.hpp", directory or ".", auto_file_name), "w"))
    local function writef(fmt, ...)
        f:write(string.format(fmt .. "\n", ...))
    end

    -- 写入头文件

    writef("// This file was generated through a code generator, DO NOT edit it, please edit the code generator")
    writef("")
    writef("#ifndef %s", macro_name)
    writef("#define %s", macro_name)
    writef("")
    if has_namespace then
        writef("namespace %s {", table.concat(configuration.namespace, "::"))
        writef("")
    end
    writef("enum class %s : int {", configuration.class_name)
    writef("    __unknown__ = %d,", min_value - 1)
    for _, member in ipairs(configuration.class_members) do
        writef("    %s = %d,", member.name, member.value)
    end
    writef("};")
    writef("")
    writef("%s %s(char const* const key, size_t const len) noexcept;", configuration.class_name, configuration.function_name)
    writef("")
    if has_namespace then
        writef("}")
        writef("")
    end
    writef("#endif // %s", macro_name)

    logger.info("write header file: %s.hpp", auto_file_name)
end

---@param configuration property_map.Configuration
---@param file_name? string 输出文件名，默认与类名相同
---@param directory? string 输出文件夹，默认位于当前工作目录
function property_map.build(configuration, file_name, directory)
    ---------------------------------------- 构建状态机 ----------------------------------------

    -- 键排序

    local class_members = {}
    for _, class_member in ipairs(configuration.class_members) do
        table.insert(class_members, class_member)
    end
    table.sort(class_members, function(a, b)
        return a.name_in_lua < b.name_in_lua
    end)

    -- 自动生成枚举值

    local auto_increment_value = 0
    for _, class_member in ipairs(class_members) do
        if class_member.value == magic_value then
            class_member.value = auto_increment_value
            auto_increment_value = auto_increment_value + 1
        end
    end
    logger.info("total class members: %d + 1 (default unknown)", #class_members)

    -- 计算最大键长

    local max_key_length = 0
    local total_key_length = 0
    for _, member in ipairs(class_members) do
        max_key_length = math.max(max_key_length, member.name_in_lua:len())
        total_key_length = total_key_length + member.name_in_lua:len()
    end
    logger.info("max key length: %d", max_key_length)
    logger.info("total key length: %d", total_key_length)

    -- 准备状态机数据结构

    ---@class property_map.StateMetadata
    ---@field character string 字符
    ---@field last_state number 上一个状态值
    ---@field next_state number 下一个状态值
    ---@field class_members property_map.Configuration.ClassMember[] 关联的枚举类成员

    ---@type property_map.StateMetadata[][]
    local groups = {}
    for i = 1, max_key_length do
        groups[i] = {}
    end

    -- 生成状态转换

    local auto_state_value = 0
    for _, member in ipairs(class_members) do
        for i, character in characters(member.name_in_lua) do
            local found = false
            for _, entry in ipairs(groups[i]) do
                if entry.character == character and (i == 1 or entry.last_state == member.final_state) then
                    member.final_state = entry.next_state
                    table.insert(entry.class_members, member)
                    found = true
                    break
                end
            end
            if not found then
                ---@type property_map.StateMetadata
                local entry = {
                    character = character,
                    last_state = magic_value,
                    next_state = magic_value,
                    class_members = { member },
                }
                if i == 1 then
                    auto_state_value = auto_state_value + 1
                    entry.last_state = 0
                    entry.next_state = auto_state_value
                else
                    auto_state_value = auto_state_value + 1
                    entry.last_state = member.final_state
                    entry.next_state = auto_state_value
                end
                member.final_state = entry.next_state
                table.insert(groups[i], entry)
            end
        end
    end
    logger.info("total state: %d", auto_state_value)

    ---------------------------------------- 生成头文件 ----------------------------------------

    generateHeaderFile(configuration, file_name, directory)

    ---------------------------------------- 生成源文件 ----------------------------------------

    -- 状态值排序

    table.sort(class_members, function(a, b)
        return a.final_state < b.final_state
    end)

    -- 打开文件

    local has_namespace = #configuration.namespace > 0
    local auto_file_name = file_name or configuration.class_name
    local f = assert(io.open(string.format("%s/%s.cpp", directory or ".", auto_file_name), "w"))
    local function writef(fmt, ...)
        f:write(string.format(fmt .. "\n", ...))
    end

    -- 写入源文件

    writef("// This file was generated through a code generator, DO NOT edit it, please edit the code generator")
    writef("")
    writef("#include \"%s.hpp\"", auto_file_name)
    writef("")
    if has_namespace then
        writef("namespace %s {", table.concat(configuration.namespace, "::"))
        writef("")
    end
    writef("%s %s(char const* const key, size_t const len) noexcept {", configuration.class_name, configuration.function_name)
    writef("    if (key == nullptr || len == 0 || len > %d) {", max_key_length)
    writef("        return %s::__unknown__;", configuration.class_name)
    writef("    }")
    writef("    int state{ 0 };")
    writef("    #define make_condition(last_state, character) ((last_state << 8) | static_cast<int>(character))")
    writef("    for (size_t i = 0; i < len; i += 1) {")
    writef("        switch (i) {")
    for i, group in ipairs(groups) do
        writef("        case %d:", i - 1) -- C/C++ 索引从 0 开始
        writef("            switch (make_condition(state, key[i])) {")
        for _, entry in ipairs(group) do
            local names = {}
            for _, member in ipairs(entry.class_members) do
                table.insert(names, member.name_in_lua)
            end
            table.sort(names)
            writef("            case make_condition(%d, '%s'): state = %d; continue; // -> %s", entry.last_state, entry.character, entry.next_state, table.concat(names, ", "))
        end
        writef("            default: return %s::__unknown__;", configuration.class_name)
        writef("            }")
    end
    writef("        default: return %s::__unknown__;", configuration.class_name)
    writef("        }")
    writef("    }")
    writef("    #undef make_condition")
    writef("    switch (state) {")
    for _, member in ipairs(class_members) do
        writef("    case %d: return %s::%s;", member.final_state, configuration.class_name, member.name)
    end
    writef("    default: return %s::__unknown__;", configuration.class_name)
    writef("    }")
    writef("}")
    writef("")
    if has_namespace then
        writef("}")
    end

    logger.info("write source file: %s.cpp", auto_file_name)
end

return property_map
