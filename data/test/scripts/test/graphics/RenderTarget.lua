local test = require("test")
local imgui = require("imgui")
local RenderTarget = require("lstg.RenderTarget")
local Texture2D = require("lstg.Texture2D")
local Sprite = require("lstg.Sprite")
local SpriteRenderer = require("lstg.SpriteRenderer")

---@class test.graphics.RenderTarget : test.Base
local M = {}

function M:onCreate()
    self.timer = 0
    self.tex = Texture2D.createFromFile("res/hgefont.png")
    self.sp1 = Sprite.create(self.tex, 0, 0, 256, 256)
    self.renderer1 = SpriteRenderer.create(self.sp1)
    self.rt = RenderTarget.create(320, 240)
    self.sp2 = Sprite.create(self.rt:getTexture(), 0, 0, 320, 240)
    self.renderer2 = SpriteRenderer.create(self.sp2)
end

function M:onDestroy()
end

function M:onUpdate()
    self.timer = self.timer + 1
    -----@diagnostic disable-next-line: undefined-field
    --local ImGui = imgui.ImGui
    --if ImGui.Begin("Graphics: RenderTarget") then
    --    local width, height = self.texture:getWidth(), self.texture:getHeight()
    --    ImGui.Text(("size: %d x %d"):format(width, height))
    --end
    --ImGui.End()
end

function M:onRender()
    lstg.PushRenderTarget(self.rt)
    lstg.RenderClear(lstg.Color(16, 0, 0, 0))

    lstg.SetViewport(0, 320, 0, 240)
    lstg.SetScissorRect(0, 320, 0, 240)
    lstg.SetOrtho(0, 320, 0, 240)
    lstg.SetFog()
    lstg.SetZBufferEnable(0)

    self.renderer1:setColor(lstg.Color(255, 0, 0, 0))
    self.renderer1:setLegacyBlendState("mul+alpha")
    self.renderer1:setTransform(320 / 2, 240 / 2, self.timer / 10)
    self.renderer1:draw()

    lstg.PopRenderTarget()

    window:applyCameraV()

    self.renderer2:setColor(lstg.Color(255, 255, 255, 255))
    self.renderer2:setLegacyBlendState("mul+alpha")
    self.renderer2:setTransform(320, 240)
    self.renderer2:draw()
end

test.registerTest("test.graphics.RenderTarget", M, "Graphics: RenderTarget")
