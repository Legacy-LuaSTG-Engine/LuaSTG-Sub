---@class findtable
local M = {}

local DEBUG = true

--------------------------------------------------------------------------------
--- import

local ipairs = ipairs
local pairs = pairs

local strrep = string.rep
local strfmt = string.format

local insert = table.insert
local remove = table.remove
local tsort  = table.sort

--------------------------------------------------------------------------------
--- helper

local indent_value = "  "

---@param depth number
---@return string
local function indent(depth)
    return strrep(indent_value, depth)
end

local function printf(fmt, ...)
    print(strfmt(fmt, ...))
end

--------------------------------------------------------------------------------
--- generator

---@class findtable.config.entry
local _ = {
    lua_id = "",
    enum_id = "",
    enum_value = 0,
}

---@class findtable.config
local _ = {
    ---@type string[]
    cpp_namespace = {},
    func_name = "",
    enum_name = "",
    ---@type findtable.config.entry[]
    enum_entry = {},
}

---@param lua_id string
---@param enum_id string
---@param enum_value number
---@return findtable.config.entry
---@overload fun(lua_id:string):findtable.config.entry
---@overload fun(lua_id:string, enum_id:string):findtable.config.entry
function M.makeEntry(lua_id, enum_id, enum_value)
    local ret = {
        lua_id = lua_id,
        enum_id = enum_id or lua_id,
        enum_value = false,
    }
    if enum_value then
        ret.enum_value = enum_value
    end
    return ret
end

---@param cfg findtable.config
---@return string
function M.makeEnumClass(cfg)
    local buffer = {}
    local function fmtins(s, ...)
        insert(buffer, strfmt(s, ...))
    end
    
    fmtins("namespace %s {\n", table.concat(cfg.cpp_namespace, "::"))
    fmtins("    enum class %s {\n", cfg.enum_name)
    fmtins("        __NOT_FOUND = -1,\n")
    ---@type findtable.config.entry[]
    local name_list = {}
    for _, v in ipairs(cfg.enum_entry) do
        table.insert(name_list, v)
    end
    table.sort(name_list, function(a, b)
        return a.lua_id < b.lua_id
    end)
    for i, v in ipairs(name_list) do
        fmtins("        %s = %d,\n", v.enum_id, v.enum_value or i)
    end
    fmtins("    };\n")
    fmtins("}\n")
    
    local ret = table.concat(buffer)
    if DEBUG then
        print(ret)
    end
    return ret
end

---@param cfg findtable.config
---@return string
function M.makeFindFunctionDeclaration(cfg)
    local buffer = {}
    local function fmtins(s, ...)
        insert(buffer, strfmt(s, ...))
    end
    
    fmtins("namespace %s {\n", table.concat(cfg.cpp_namespace, "::"))
    fmtins("    %s %s(const char* key);\n", cfg.enum_name, cfg.func_name)
    fmtins("}\n")
    
    local ret = table.concat(buffer)
    if DEBUG then
        print(ret)
    end
    return ret
end

---@param cfg findtable.config
---@return string
function M.makeEnumClassAndFindFunctionDeclaration(cfg)
    local buffer = {}
    local function fmtins(s, ...)
        insert(buffer, strfmt(s, ...))
    end
    
    fmtins("namespace %s {\n", table.concat(cfg.cpp_namespace, "::")) -- {
    
    ---- enum ----
    
    fmtins("    enum class %s {\n", cfg.enum_name)
    fmtins("        __NOT_FOUND = -1,\n")
    ---@type findtable.config.entry[]
    local name_list = {}
    for _, v in ipairs(cfg.enum_entry) do
        table.insert(name_list, v)
    end
    table.sort(name_list, function(a, b)
        return a.lua_id < b.lua_id
    end)
    for i, v in ipairs(name_list) do
        fmtins("        %s = %d,\n", v.enum_id, v.enum_value or i)
    end
    fmtins("    };\n")
    
    ---- func ----
    
    fmtins("    %s %s(const char* key);\n", cfg.enum_name, cfg.func_name)
    
    fmtins("}\n") -- }
    
    local ret = table.concat(buffer)
    if DEBUG then
        print(ret)
    end
    return ret
end

---@param cfg findtable.config
---@return string
function M.makeFindFunction2(cfg)
    local buffer = {}
    local function fmtins(s, ...)
        insert(buffer, strfmt(s, ...))
    end
    
    local nshead = ""
    if #cfg.cpp_namespace > 0 then
        nshead = table.concat(cfg.cpp_namespace, "::") .. "::"
    end
    
    local g_name = string.format("g_%s%s%s", table.concat(cfg.cpp_namespace), cfg.enum_name, cfg.func_name)
    
    fmtins("static const std::unordered_map<std::string_view, %s%s> %s = {\n", nshead, cfg.enum_name, g_name)
    ---@type table<string, findtable.config.entry>
    local name_set = {}
    for _, v in ipairs(cfg.enum_entry) do
        name_set[v.lua_id] = v
    end
    ---@type findtable.config.entry[]
    local name_list = {}
    for _, v in pairs(name_set) do
        table.insert(name_list, v)
    end
    table.sort(name_list, function(a, b)
        return a.lua_id < b.lua_id
    end)
    for _, v in ipairs(name_list) do
        fmtins("  {\"%s\",%s%s::%s},\n", v.lua_id, nshead, cfg.enum_name, v.enum_id)
    end
    fmtins("};\n")
    fmtins("%s%s %s%s(const char* key) {\n", nshead, cfg.enum_name, nshead, cfg.func_name)
    fmtins("  auto it = %s.find(key);\n", g_name)
    fmtins("  if (it != %s.end()) return it->second;\n", g_name)
    fmtins("  return %s%s::__NOT_FOUND;\n", nshead, cfg.enum_name)
    fmtins("}\n")
    
    -- 合并缓冲区为字符串并返回
    local ret = table.concat(buffer)
    if DEBUG then
        print(ret)
    end
    return ret
end

---@param cfg findtable.config
---@return string
function M.makeFindFunction(cfg)
    -- 对重复的进行警告
    
    ---@type table<string, findtable.config.entry>
    local entry_map = {}
    for _, v in ipairs(cfg.enum_entry) do
        if entry_map[v.lua_id] then
            printf("[W] duplicate entry [%q, %q]", v.lua_id, v.enum_id)
        end
        entry_map[v.lua_id] = v
    end
    
    -- 按字典序排序
    
    ---@type findtable.config.entry[]
    local dict_list = {}
    for _, v in ipairs(cfg.enum_entry) do
        insert(dict_list, v)
    end
    tsort(dict_list, function (a, b)
        return a.lua_id < b.lua_id
    end)
    if DEBUG then
        printf("----------------------------------------")
        for i, v in ipairs(dict_list) do
            printf("[%d] %q -> %q", i, v.lua_id, v.enum_id)
        end
    end
    
    -- 生成字典树
    
    ---@param entry_list findtable.config.entry[]
    local function make_dict_tree_map(entry_list)
        local depth = 0
        ---@param src findtable.config.entry[]
        local function procf(src)
            local ret = {}
            depth = depth + 1
            for _, v in ipairs(src) do
                local ch = string.sub(v.lua_id, depth, depth) -- 取索引处的字母
                local len = string.len(v.lua_id) -- 属性长度
                ret[ch] = ret[ch] or {}
                if len > depth then
                    table.insert(ret[ch], v)
                else
                    ret[ch][0] = v -- 已经是目标值了，放到“0”位缓存起来（防止下面计算属性数量的时候也算在里面）
                end
            end
            local _ret = {}
            for ch, v in pairs(ret) do
                if #v > 0 then
                    _ret[ch] = procf(v) -- 对于还是有多个属性的子表，递归处理
                    _ret[ch][0] = ret[ch][0] -- 记得补上“0”位的值
                else
                    _ret[ch] = v -- 没有多个属性，直接保存
                end
            end
            depth = depth - 1
            return _ret
        end
        return procf(entry_list)
    end
    local dict_tree_map = make_dict_tree_map(dict_list)
    if DEBUG then
        local function print_dict_tree_map(treedata)
            local index = 1
            local depth = 0
            local function procf(tree)
                depth = depth + 1
                for k, v in pairs(tree) do
                    if k == 0 then
                        printf("%02d:%s%s", index, strrep("   |", depth - 1), v.lua_id)
                        index = index + 1
                    else
                        procf(v)
                    end
                end
                depth = depth - 1
            end
            procf(treedata)
        end
        print("----------------------------------------")
        print_dict_tree_map(dict_tree_map)
    end
    
    -- 将字典树处理为方便生成代码的树结构
    
    local function make_dict_tree(tree_data)
        local function procf(tree)
            local ret = {}
            for k, v in pairs(tree) do
                if k == 0 then
                    k = "\0" -- 对于“0”索引进行特殊处理，转为字符串（方便做字典序排序）
                    table.insert(ret, { k, v }) -- 已经是目标值
                else
                    table.insert(ret, { k, procf(v) }) -- 有多个条目，递归处理
                end
            end
            -- 按字典序排序，方便编译器为switch语句优化
            table.sort(ret, function(a, b)
                return a[1] < b[1]
            end)
            return ret
        end
        return procf(tree_data)
    end
    local dict_tree = make_dict_tree(dict_tree_map)
    if DEBUG then
        local function print_dict_tree(treedata)
            local depth = 0
            local function procf(tree)
                for _, v in ipairs(tree) do
                    if v[1] == "\0" then
                        printf("%s~:", indent(depth))
                        printf("%s%s", indent(depth + 1), v[2].lua_id)
                    else
                        printf("%s%s:", indent(depth), v[1])
                        depth = depth + 1
                        procf(v[2])
                        depth = depth - 1
                    end
                end
            end
            return procf(treedata)
        end
        print("----------------------------------------")
        print_dict_tree(dict_tree)
    end
    
    -- 生成
    
    local buffer = {}
    local function fmtins(s, ...)
        insert(buffer, strfmt(s, ...))
    end
    
    -- 哈希函数头
    local nshead = ""
    if #cfg.cpp_namespace > 0 then
        nshead = table.concat(cfg.cpp_namespace, "::") .. "::"
    end
    fmtins("%s%s %s%s(const char* key) {\n", nshead, cfg.enum_name, nshead, cfg.func_name)
    
    -- 生成哈希用的C语言switch屎山
    local function layer_switch(treedata)
        local depth = 1 -- 生成的代码缩进层级
        local index = 0 -- 键索引
        local function procf(tree)
            -- switch头
            fmtins("%sswitch(key[%d]) {\n", indent(depth), index)
            -- 内部的各个case
            depth = depth + 1
            for _, node in ipairs(tree) do
                -- 先生成case
                if node[1] == "\0" then
                    fmtins("%scase '\\0':\n", indent(depth))-- 特殊处理
                    -- 已经是目标值
                    --fmtins("%sreturn %d;\n", indent(depth + 1), property.hash)
                    fmtins("%sreturn %s%s::%s;\n", indent(depth + 1), nshead, cfg.enum_name, node[2].enum_id)
                else
                    fmtins("%scase '%s':\n", indent(depth), node[1])
                    -- 里面还有别的层级，递归生成
                    depth = depth + 1
                    index = index + 1
                    procf(node[2])
                    index = index - 1
                    depth = depth - 1
                    fmtins("%sbreak;\n", indent(depth + 1))
                end
            end
            depth = depth - 1
            -- 收尾
            fmtins("%s}\n", indent(depth))
        end
        procf(treedata)
    end
    layer_switch(dict_tree)
    
    -- 收尾工作
    fmtins("%sreturn %s%s::__NOT_FOUND;\n", indent(1), nshead, cfg.enum_name)
    fmtins("}\n")
    
    -- 合并缓冲区为字符串并返回
    local ret = table.concat(buffer)
    if DEBUG then
        print(ret)
    end
    return ret
end

---@param name string
---@param cfgs findtable.config[]
function M.makeSource(name, cfgs)
    local generator_info = {
        "Write by Kuanlan (Kuanlan@outlook.com)",
        "Minimum perfect hash function for Lua language binding.",
        "This source file is generated by code generator.",
        "Don't modify this file directly, please modify the generator configuration.",
    }
    
    local header = {}
    local source = {}
    
    -- hpp file
    for _, s in ipairs(generator_info) do
        table.insert(header, "// ")
        table.insert(header, s)
        table.insert(header, "\n")
    end
    table.insert(header, "\n")
    table.insert(header, "#pragma once\n")
    table.insert(header, "\n")
    --table.insert(header, string.format("#include <cstdint>\n"))
    --table.insert(header, "\n")
    for _, cfg in ipairs(cfgs) do
        --table.insert(header, M.makeEnumClass(cfg))
        --table.insert(header, "\n")
        --table.insert(header, M.makeFindFunctionDeclaration(cfg))
        --table.insert(header, "\n")
        table.insert(header, M.makeEnumClassAndFindFunctionDeclaration(cfg))
        table.insert(header, "\n")
    end
    
    -- cpp file
    for _, s in ipairs(generator_info) do
        table.insert(source, "// ")
        table.insert(source, s)
        table.insert(source, "\n")
    end
    table.insert(source, "\n")
    table.insert(source, string.format("#include \"%s.hpp\"\n", name))
    table.insert(source, "\n")
    --table.insert(source, string.format("#include <string_view>\n"))
    --table.insert(source, string.format("#include <unordered_map>\n"))
    --table.insert(source, "\n")
    for _, cfg in ipairs(cfgs) do
        table.insert(source, M.makeFindFunction(cfg))
        --table.insert(source, M.makeFindFunction2(cfg))
        table.insert(source, "\n")
    end
    
    local header_string = table.concat(header)
    local source_string = table.concat(source)
    
    local function write_file(name, buffer)
        local f = io.open(name, "wb")
        f:write(buffer)
        f:close()
    end
    write_file(name .. ".hpp", header_string)
    write_file(name .. ".cpp", source_string)
end

--------------------------------------------------------------------------------
--- export

return M
