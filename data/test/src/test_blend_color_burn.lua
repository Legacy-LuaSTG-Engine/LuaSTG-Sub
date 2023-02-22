local test = require("test")

---@class test.Module.PostEffect.Blend.ColorBurn : test.Base
local M = {}

local function load_image(name, path)
    lstg.LoadTexture(name, path, false)
    local w, h = lstg.GetTextureSize(name)
    lstg.LoadImage(name, name, 0, 0, w, h)
end

local function unload_image(pool, name)
    lstg.RemoveResource(pool, 2, name)
    lstg.RemoveResource(pool, 1, name)
end

function M:onCreate()
    self.shader = lstg.CreatePostEffectShader("res/blend_color_burn.hlsl")

    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    lstg.CreateRenderTarget("rt:bottom")
    lstg.CreateRenderTarget("rt:top")

    load_image("image_2", "res/image_2.jpg")
    --load_image("image_3", "res/image_3.jpg")

    lstg.SetResourceStatus(last_pool)

    self.timer = 0
end

function M:onDestroy()
    lstg.RemoveResource("global", 1, "rt:bottom")
    lstg.RemoveResource("global", 1, "rt:top")

    unload_image("global", "image_2")
    --unload_image("global", "image_3")
end

function M:onUpdate()
    self.timer = self.timer + 1
end

function M:onRender()
    window:applyCameraV()

    local transparent_black = lstg.Color(0, 0, 0, 0)
    local opaque_white = lstg.Color(255, 255, 255, 255)

    lstg.PushRenderTarget("rt:bottom")
    lstg.RenderClear(transparent_black)
        --lstg.SetImageState("image_2", "", lstg.Color(255, 40, 40, 40))
        lstg.Render("image_2", window.width / 2, window.height / 2, 0, window.height / 2160)
    lstg.PopRenderTarget()

    local channel_value = 128 + 127 * lstg.sin(self.timer)

    lstg.PushRenderTarget("rt:top")
    lstg.RenderClear(transparent_black)
    --lstg.RenderClear(opaque_white)
    lstg.RenderClear(lstg.Color(255, channel_value, channel_value, channel_value))
        --lstg.Render("image_3", window.width / 2, window.height / 2, 0, window.width / 1024)
    lstg.PopRenderTarget()

    self.shader:setTexture("g_render_target", "rt:bottom")
    self.shader:setTexture("g_texture", "rt:top")
    lstg.PostEffect(self.shader, "")
end

test.registerTest("test.Module.PostEffect.Blend.ColorBurn", M)
