local test = require("test")
local imgui = require("imgui")
local FontWeight = require("lstg.FontWeight")
local FontStyle = require("lstg.FontStyle")
local FontWidth = require("lstg.FontWidth")
local TextAlignment = require("lstg.TextAlignment")
local ParagraphAlignment = require("lstg.ParagraphAlignment")
local TextLayout = require("lstg.TextLayout")
local TextRenderer = require("lstg.TextRenderer")

local TITLE = "Graphics: TextLayout"

---@class test.graphics.TextLayout : test.Base
local M = {}

function M:onCreate()
    self.timer = 0

    self.text_layout = assert(TextLayout.create()
        :setText("")
        :setFontFamily("微软雅黑")
        :setFontSize(64)
        :setFontWeight(FontWeight.bold)
        :setTextAlignment(TextAlignment.center)
        :setParagraphAlignment(ParagraphAlignment.center)
        :setLayoutSize(600, 80)
        :build()
    )
    self.renderer = TextRenderer.create(self.text_layout)

    self.text_layout_large = assert(TextLayout.create()
        :setText(lstg.LoadTextFile("assets/text/article1.txt"))
        :setFontFamily("Noto Sans SC")
        :setFontSize(16)
        :setLayoutSize(1280, 720)
        :build()
    )
    self.renderer_large = TextRenderer.create(self.text_layout_large)
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
