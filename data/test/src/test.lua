
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
    ["empty"] = T,
}

---@type test.Base
local current_test = T

---@param name string
---@param cls test.Base
function M.registerTest(name, cls)
    tests[name] = cls
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
            for k, v in pairs(tests) do
                if ImGui.Button(k) then
                    current_test:onDestroy()
                    current_test = {}
                    setmetatable(current_test, { __index = v })
                    current_test:onCreate()
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

return M
