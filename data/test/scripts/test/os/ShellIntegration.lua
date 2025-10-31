local test = require("test")
local ShellIntegration = require("lstg.ShellIntegration")
local imgui = require("imgui")

---@class test.os.ShellIntegration : test.Base
local M = {}

function M:onCreate()
    self.any_key_down = false
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("OS: ShellIntegration") then
        if ImGui.Button("Open file: config.json") then
            ShellIntegration.openFile("config.json")
        end
        if ImGui.Button("Open directory: userdata") then
            ShellIntegration.openDirectory("userdata")
        end
        if ImGui.Button("Open url: https://www.lua.org") then
            ShellIntegration.openUrl("https://www.lua.org")
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.os.ShellIntegration", M, "OS: ShellIntegration")
