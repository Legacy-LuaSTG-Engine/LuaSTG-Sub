
local imgui_exist, imgui = pcall(require, "imgui")

---@class test.Base
local T = {}

function T:onCreate() end

function T:onDestroy() end

function T:onUpdate() end

function T:onRender() end

---@class test
local M = {}

---@type table<string, test.Base>
local tests = {
    { "test.Module.Empty", T },
}

---@type test.Base
local current_test = T

---@param name string
---@param cls test.Base
function M.registerTest(name, cls)
    for _, v in ipairs(tests) do
        if v[1] == name then
            assert(false)
            v[2] = cls
            return
        end
    end
    table.insert(tests, { name, cls })
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
        local ImGui = imgui.ImGui
        if ImGui.Begin("Select Test") then
            for _, v in ipairs(tests) do
                if ImGui.Button(v[1]) then
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
