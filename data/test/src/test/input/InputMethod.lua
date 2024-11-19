local utf8 = require("utf8")
local test = require("test")
local imgui = require("imgui")
local Window = require("lstg.Window")

local function refreshGlyphCache(str)
    ---@diagnostic disable-next-line: undefined-field
    imgui.backend.CacheGlyphFromString(str)
end

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
    self.text_input_ext = self.main_window:queryInterface("lstg.Window.TextInputExtension")
    self.cursor = 0
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TEST_NAME) then
        if self.input_method_ext then
            self.input_method_ext:setInputMethodPosition(0, 0)
            local input_method_b1 = self.input_method_ext:isInputMethodEnabled()
            local input_method_e, input_method_b2 = ImGui.Checkbox("Input Method", input_method_b1)
            if input_method_e and (input_method_b1 ~= input_method_b2) then
                self.input_method_ext:setInputMethodEnabled(input_method_b2)
            end
        end
        ImGui.Separator()
        if self.text_input_ext then
            local text_input_b1 = self.text_input_ext:isEnabled()
            local text_input_e, text_input_b2 = ImGui.Checkbox("Text Input", text_input_b1)
            if text_input_e and (text_input_b1 ~= text_input_b2) then
                self.text_input_ext:setEnabled(text_input_b2)
            end
            if ImGui.Button("Clear##Text Input State") then
                self.text_input_ext:clear()
            end
            local text = self.text_input_ext:toString()
            refreshGlyphCache(text)
            ImGui.SameLine()
            self.cursor = self.text_input_ext:getCursorPosition()
            local codes = {}
            for _, c in utf8.codes(text) do
                table.insert(codes, c)
            end
            local first_part = {}
            for i = 1, self.cursor do
                first_part[i] = utf8.char(codes[i])
            end
            local second_part = {}
            for i = self.cursor + 1, #codes do
                second_part[i - self.cursor] = utf8.char(codes[i])
            end
            ImGui.Text("Text Input Buffer: " .. table.concat(first_part))
            ImGui.SameLine()
            ImGui.Text("|")
            ImGui.SameLine()
            ImGui.Text(table.concat(second_part))
            ImGui.Text("Text Input Cursor: " .. self.cursor)
            ImGui.SameLine()
            if ImGui.Button("<##Text Input Cursor") then
                self.text_input_ext:addCursorPosition(-1)
            end
            ImGui.SameLine()
            if ImGui.Button(">##Text Input Cursor") then
                self.text_input_ext:addCursorPosition(1)
            end
            ImGui.SameLine()
            if ImGui.Button("x##Text Input Cursor") then
                self.text_input_ext:remove()
            end
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.input.InputMethod", M, TEST_NAME)
