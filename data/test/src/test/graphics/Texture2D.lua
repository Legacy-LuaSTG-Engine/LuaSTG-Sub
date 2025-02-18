local test = require("test")
local imgui = require("imgui")
local Texture2D = require("lstg.Texture2D")

---@class test.graphics.Texture2D : test.Base
local M = {}

function M:onCreate()
    self.texture = Texture2D.createFromFile("res/image_1.png")
end

function M:onDestroy()
end

function M:onUpdate()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Graphics: Texture2D") then
        local width, height = self.texture:getWidth(), self.texture:getHeight()
        ImGui.Text(("size: %d x %d"):format(width, height))
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.graphics.Texture2D", M, "Graphics: Texture2D")
