local test = require("test")
local imgui = require("imgui")

---@class test.base.ApplicationRestart : test.Base
local M = {}

local TITLE = "Base: ApplicationRestart"

function M:onCreate()
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TITLE) then
        if ImGui.Button("Restart") then
            lstg.RestartWithCommandLineArguments(lstg.args)
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.audio.ApplicationRestart", M, TITLE)
