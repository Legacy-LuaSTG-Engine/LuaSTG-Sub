local io = require("io")
local json = require("json")

---@class tool.VersionInfoGenerator.Config
---@field major integer
---@field minor integer
---@field patch integer
---@field package string
---@field organization string

---@param path string
---@return string
local function readFile(path)
    local f = assert(io.open(path, "r"))
    local s = assert(f:read("*a"))
    assert(f:close())
    return s
end

---@param path string
---@param s string
local function writeFile(path, s)
    local f = assert(io.open(path, "w"))
    assert(f:write(s))
    assert(f:close())
    return s
end

---@param template string
---@param config tool.VersionInfoGenerator.Config
---@return string
local function applyVariables(template, config)
    local version_comma_str = ("%d,%d,%d"):format(config.major, config.minor, config.patch)
    local version_str = ("%d.%d.%d"):format(config.major, config.minor, config.patch)
    template = template:gsub("${VERSION_COMMA}", version_comma_str)
    template = template:gsub("${VERSION}", version_str)
    template = template:gsub("${VERSION_MAJOR}", config.major)
    template = template:gsub("${VERSION_MINOR}", config.minor)
    template = template:gsub("${VERSION_PATCH}", config.patch)
    template = template:gsub("${PACKAGE}", config.package)
    template = template:gsub("${ORGNIZATION}", config.organization)
    return template
end

---@class tool.VersionInfoGenerator.File
---@field input string
---@field output string

---@param files tool.VersionInfoGenerator.File[]
local function process(files)
    local config = json.decode(readFile("config.json"))
    print("[I] config:")
    for k, v in pairs(config) do
        print(("[I] - %s: %s"):format(tostring(k), tostring(v)))
    end
    for _, f in ipairs(files) do
        local template = readFile(f.input)
        template = applyVariables(template, config)
        writeFile(f.output, template)
    end
    print(("[I] write %d files"):format(#files))
end

---@type tool.VersionInfoGenerator.File[]
local files = {
    {
        input = "template/resource.rc",
        output = "../../LuaSTG/LuaSTG/Custom/resource.rc",
    },
    {
        input = "template/LuaSTG.manifest",
        output = "../../LuaSTG/LuaSTG/LuaSTG.manifest",
    },
    {
        input = "template/LConfig.h",
        output = "../../LuaSTG/LuaSTG/LConfig.h",
    },
}

process(files)
