local test = require("test")
local imgui = require("imgui")
local Window = require("lstg.Window")

local TEST_NAME = "Input: Input Method"

---@class test.input.InputMethod : test.Base
local M = {}

function M:onCreate()
    local old = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    -- load resources
    lstg.SetResourceStatus(old)
    self.main_window = assert(Window.getMain(), "main window is required")
    self.input_method_ext = self.main_window:queryInterface("lstg.Window.InputMethodExtension")
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TEST_NAME) then
        if self.input_method_ext then
            local enabled = self.input_method_ext:isInputMethodEnabled()
            ImGui.Text(("Input Method State: %s"):format(tostring(enabled)))
            if ImGui.Button("Enable") then
                self.input_method_ext:setInputMethodEnabled(true)
            end
            if ImGui.Button("Disable") then
                self.input_method_ext:setInputMethodEnabled(false)
            end
        else
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.input.InputMethod", M, TEST_NAME)
