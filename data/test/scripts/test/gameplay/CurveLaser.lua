local test = require("test")
local helpers = require("test.helpers")
local task = require("task")
local TaskManager = require("task.Manager")
local lstg = require("lstg")
local imgui = require("imgui")

local sin = lstg.sin
local cos = lstg.cos
local COLOR_WHITE = lstg.Color(255, 255, 255, 255)
local COLOR_RED = lstg.Color(255, 255, 0, 0)
local COLOR_GREEN = lstg.Color(128, 0, 255, 0)

---@class test.gameplay.CurveLaser : test.Base
local M = {}

function M:onCreate()
    helpers.resourcesLoadingScope("global", function()
        lstg.LoadTexture("primitives", "assets/texture/primitives.png")
        lstg.LoadImage("circle", "primitives", 16, 16, 32, 32)
    end)
    self.laser = lstg.BentLaserData()
    self.timer = 0
    self.mouse_x = 0
    self.mouse_y = 0
    self.mouse_primary = false
    self.show_collider = false
    self.node_count = 256
end

function M:onDestroy()
    lstg.RemoveResource("global", 2, "circle")
    lstg.RemoveResource("global", 1, "primitives")
end

function M:onUpdate()
    self.timer = self.timer + 1
    self.mouse_x, self.mouse_y = lstg.GetMousePosition()
    self.mouse_primary = lstg.GetMouseState(0)
    self:ui()
    local cx, cy = window.width / 2, window.height / 2
    local r = 100
    local a = self.timer
    self.laser:Update(cx + r * cos(a), cy + r * sin(a), a + 90, self.node_count, 16)
    if self.mouse_primary then
        local _, points = self.laser:Cut(self.mouse_x, self.mouse_y, 16)
        for _, p in ipairs(points) do
            lstg.Log(2, ("cut %f,%f"):format(p.x, p.y))
        end
    end
end

function M:ui()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Gameplay: CurveLaser") then
        _, self.show_collider = ImGui.Checkbox("Show Collider", self.show_collider)
        _, self.node_count = ImGui.SliderInt("Node Count (Laser Length)", self.node_count, 2, 512)
    end
    ImGui.End()
end

function M:onRender()
    window:applyCameraV()
    self.laser:Render("primitives", "", COLOR_WHITE, 0, 240, 256, 16, 1)
    if self.show_collider then
        self.laser:RenderCollider(COLOR_GREEN)
    end
    if self.mouse_primary then
        lstg.SetImageState("circle", "", COLOR_RED)
        lstg.Render("circle", self.mouse_x, self.mouse_y)
    end
end

return M
