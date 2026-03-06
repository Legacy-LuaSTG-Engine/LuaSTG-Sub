local test = require("test")
local imgui = require("imgui")
local Sprite = require("lstg.Sprite")
local SpriteRenderer = require("lstg.SpriteRenderer")
local Texture2D = require("lstg.Texture2D")

local TITLE = "Graphics: Sprite Center"

local function loadImage(name, path)
    lstg.LoadTexture(name, path, false)
    local w, h = lstg.GetTextureSize(name)
    lstg.LoadImage(name, name, 0, 0, w, h)
end

local function unloadImage(pool, name)
    lstg.RemoveResource(pool, 2, name)
    lstg.RemoveResource(pool, 1, name)
end

---@class test.graphics.SpriteCenter : test.Base
local M = {}

function M:init1()
    self.texture = Texture2D.createFromFile("res/block.png")
    local w, h = self.texture:getWidth(), self.texture:getHeight()
    self.sprite = Sprite.create(self.texture, 0, 0, w, h, self.cx, self.cy)
    self.sprite_renderer = SpriteRenderer.create(self.sprite)
end

function M:update1()
end

function M:onCreate()
    self.timer = 0
    self.cx = 0
    self.cy = 0
    self:init1()

    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    loadImage("block", "res/block.png")
    lstg.SetImageCenter("block", self.cx, self.cy)
    lstg.SetResourceStatus(last_pool)
end

function M:onDestroy()
    unloadImage("global", "block")
end

function M:onUpdate()
    self.timer = self.timer + 1
    self:update1()
    local w, h = self.texture:getWidth(), self.texture:getHeight()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TITLE) then
        local cx_updated = false
        local cy_updated = false
        cx_updated, self.cx = ImGui.SliderFloat("Center X", self.cx, 0, w)
        cy_updated, self.cy = ImGui.SliderFloat("Center Y", self.cy, 0, h)
        if cx_updated or cy_updated then
            self.sprite:setCenter(self.cx, self.cy)
            lstg.SetImageCenter("block", self.cx, self.cy)
        end
    end
    ImGui.End()
end

function M:onRender()
    window:applyCamera3D()
    lstg.Render("block", window.width *0.7, window.height / 2, 0, 1)
    self.sprite_renderer:setTransform(window.width * 0.25, window.height / 2, 0, 1)
    self.sprite_renderer:draw()
end

test.registerTest("test.graphics.SpriteCenter", M, TITLE)
