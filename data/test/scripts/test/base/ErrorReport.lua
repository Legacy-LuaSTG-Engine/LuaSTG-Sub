local test = require("test")
local imgui = require("imgui")

---@class test.base.ErrorReport : test.Base
local M = {}

local TITLE = "Base: Error Report"

function M:onCreate()
    ---@diagnostic disable-next-line: undefined-field
    self.buffer = imgui.ImGuiTextBuffer()
    self.buffer:resize(1024)
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TITLE) then
        ImGui.InputTextMultiline("Reason", self.buffer)
        if ImGui.Button("BOMB!!!") then
            error(self.buffer:c_str())
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.audio.ErrorReport", M, TITLE)
