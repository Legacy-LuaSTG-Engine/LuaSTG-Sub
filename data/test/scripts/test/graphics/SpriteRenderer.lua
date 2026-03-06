local test = require("test")
local imgui = require("imgui")
local Sprite = require("lstg.Sprite")
local SpriteRenderer = require("lstg.SpriteRenderer")
local Texture2D = require("lstg.Texture2D")

local function load_image(name, path)
    lstg.LoadTexture(name, path, false)
    local w, h = lstg.GetTextureSize(name)
    lstg.LoadImage(name, name, 0, 0, w, h)
end

local function unload_image(pool, name)
    lstg.RemoveResource(pool, 2, name)
    lstg.RemoveResource(pool, 1, name)
end

---@class test.graphics.SpriteRenderer : test.Base
local M = {}

function M:init1()
    local w, h = self.texture:getWidth(), self.texture:getHeight()
    self.sprite = Sprite.create(self.texture, 0, 0, w, h)
    self.sprite_renderer = SpriteRenderer.create(self.sprite)
end

function M:update1()
    local w_2 = window.width / 2
    local h_2 = window.height / 2
    local y = h_2 + h_2 * math.sin(self.timer / 120)
    self.sprite_renderer:setPosition(w_2, y)
    local s = 2.0 + 1.0 * math.sin(self.timer / 30)
    self.sprite_renderer:setScale(s)
    self.sprite_renderer:setRotation(self.timer / 60)
    local a = 0.5 + 0.5 * math.sin(self.timer / 20)
    local c1 = lstg.Color(255, 255, 255, 255)
    local c2 = lstg.Color(255 * a, 255, 255, 255)
    self.sprite_renderer:setColor(c1, c2, c2, c1)
    if (self.timer % 120) < 60 then
        self.sprite_renderer:setLegacyBlendState("")
    else
        self.sprite_renderer:setLegacyBlendState("mul+add")
    end
    if (self.timer % 360) < 180 then
        self.texture:setDefaultSampler("")
    else
        self.texture:setDefaultSampler("point+clamp")
    end
end

function M:onCreate()
    self.timer = 0
    self.texture = Texture2D.createFromFile("res/image_1.png")
    self:init1()

    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    load_image("image_2", "res/image_2.jpg")
    lstg.SetResourceStatus(last_pool)
end

function M:onDestroy()
    unload_image("global", "image_2")
end

function M:onUpdate()
    self.timer = self.timer + 1
    self:update1()
    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Graphics: SpriteRenderer") then
    end
    ImGui.End()
end

function M:onRender()
    window:applyCamera3D()
    lstg.Render("image_2", window.width / 2, window.height / 2, 0, window.height / 2160)
    self.sprite_renderer:draw()
end

test.registerTest("test.graphics.SpriteRenderer", M, "Graphics: SpriteRenderer")
