---@type boolean, imgui
local imgui_exist, imgui = pcall(require, "imgui")

---@class test.Base
local T = {}

function T:onCreate() end

function T:onDestroy() end

function T:onUpdate() end

function T:onRender() end

---@class test
local M = {}

---@type table<string, { [1]: string, [2]: test.Base, [3]: string }>
local tests = {
    { "test.Base", T, "test.Base" },
}

---@type test.Base
local current_test = T

---@param text string
function M.cacheDisplayText(text)
    assert(type(text) == "string")
    ---@diagnostic disable-next-line: undefined-field
    if imgui_exist and imgui and imgui.backend and imgui.backend.CacheGlyphFromString then
        ---@diagnostic disable-next-line: undefined-field
        imgui.backend.CacheGlyphFromString(text)
    end
end

---@param name string
---@param test_class test.Base
---@param display_name string?
function M.registerTest(name, test_class, display_name)
    for _, v in ipairs(tests) do
        if v[1] == name then
            assert(false)
            v[2] = test_class
            return
        end
    end
    local entry = { name, test_class, display_name or name }
    table.insert(tests, entry)
    M.cacheDisplayText(entry[3])
end

function M.onCreate()
    current_test = T
    current_test:onCreate()
end

function M.onDestroy()
    current_test:onDestroy()
    current_test = T
end

function M.onUpdate()
    if imgui_exist then
        ---@diagnostic disable-next-line: undefined-field
        local ImGui = imgui.ImGui
        if ImGui.Begin("Select Test") then
            for _, v in ipairs(tests) do
                if ImGui.Button(v[3]) then
                    M.setTest(v[1])
                end
            end
        end
        ImGui.End()
    end
    current_test:onUpdate()
end

function M.onRender()
    current_test:onRender()
end

function M.setTest(name)
    for _, v in ipairs(tests) do
        if v[1] == name then
            current_test:onDestroy()
            current_test = {}
            setmetatable(current_test, { __index = v[2] })
            current_test:onCreate()
            return
        end
    end
end

return M
