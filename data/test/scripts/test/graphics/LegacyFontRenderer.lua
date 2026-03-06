local test = require("test")
local helpers = require("test.helpers")

---@class test.graphics.LegacyFontRenderer : test.Base
local M = {}

function M:onCreate()
    helpers.resourcesLoadingScope("global", function()
        lstg.LoadTTF("sans-test", "C:/Windows/Fonts/msyh.ttc", 0, 32)
    end)

    lstg.FontRenderer.SetFontProvider("sans-test")
    lstg.FontRenderer.SetScale(1, 1)
    local text = "Hello world!"
    for i = 1, text:len() do
        local s = text:sub(i, i)
        local x, y = lstg.FontRenderer.MeasureTextAdvance(s)
        print(("[%d] %s: %.2f, %.2f"):format(i, s, x, y))
    end
    lstg.FontRenderer.GetFontLineHeight()
end

function M:onDestroy()
    lstg.RemoveResource("global", 8, "sans-test")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
end

test.registerTest("test.graphics.LegacyFontRenderer", M, "Graphics: LegacyFontRenderer")
