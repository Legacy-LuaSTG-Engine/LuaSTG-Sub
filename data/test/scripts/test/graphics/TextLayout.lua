local test = require("test")
local imgui = require("imgui")
local Sprite = require("lstg.Sprite")
local SpriteRenderer = require("lstg.SpriteRenderer")
local TextLayout = require("lstg.TextLayout")
local FontWeight = require("lstg.FontWeight")
local FontStyle = require("lstg.FontStyle")
local TextAlignment = require("lstg.TextAlignment")
local ParagraphAlignment = require("lstg.ParagraphAlignment")

local TITLE = "Graphics: TextLayout"

---@class test.graphics.TextLayout : test.Base
local M = {}

function M:onCreate()
    self.text_layout = TextLayout.create()
        :setText("")
        :setFontFamilyName("微软雅黑")
        :setFontSize(64)
        :setFontWeight(FontWeight.bold)
        :setFontStyle(FontStyle.normal)
        :setTextAlignment(TextAlignment.center)
        :setParagraphAlignment(ParagraphAlignment.center)
        :setLayoutSize(600, 80)
    self.text_layout:build()
    local t = self.text_layout:getTexture()
    local w, h = t:getWidth(), t:getHeight()
    self.sprite = Sprite.create(self.text_layout:getTexture(), 0, 0, w, h, w / 2, h / 2, 0.5)
    self.renderer = SpriteRenderer.create(self.sprite)
    self.timer = 0

    self.text_layout_large = TextLayout.create()
        :setText(lstg.LoadTextFile("assets/text/article1.txt"))
        :setFontFamilyName("Noto Sans SC")
        :setFontSize(16)
        :setFontWeight(FontWeight.normal)
        :setFontStyle(FontStyle.normal)
        :setTextAlignment(TextAlignment.start)
        :setParagraphAlignment(ParagraphAlignment.start)
        :setLayoutSize(1280, 720)
    self.text_layout_large:build()
    self.sprite_large = Sprite.create(self.text_layout_large:getTexture(), 0, 0, 1280, 720, 0, 0)
    self.renderer_large = SpriteRenderer.create(self.sprite_large)
    self.renderer_large:setTransform(0, 720)
end

function M:onDestroy()
end

function M:onUpdate()
    self.timer = self.timer + 1

    self.text_layout
        :setText(("当前计数器：%d"):format(self.timer))
        :build()

    self.renderer:setTransform(window.width / 2, window.height / 2, math.rad(self.timer))
end

function M:onRender()
    window:applyCameraV()
    self.renderer_large:draw()
    self.renderer:draw()
end

test.registerTest("test.graphics.TextLayout", M, TITLE)
