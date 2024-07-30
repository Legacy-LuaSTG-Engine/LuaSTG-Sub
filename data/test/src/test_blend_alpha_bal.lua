local test = require("test")

---@class test.Module.PostEffect.Blend.AlphaBal : test.Base
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
    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    load_image("image_2", "res/image_2.jpg")
    lstg.CreateRenderTarget("rt:white", 256, 256, false)
    lstg.CreateRenderTarget("rt:black", 256, 256, false)
    lstg.LoadImage("sprite:white", "rt:white", 0, 0, 256, 256)
    lstg.LoadImage("sprite:black", "rt:black", 0, 0, 256, 256)
    lstg.SetImageState("sprite:white", "alpha+bal", lstg.Color(255, 255, 255, 255))
    lstg.SetImageState("sprite:black", "alpha+bal", lstg.Color(255, 255, 255, 255))

    lstg.SetResourceStatus(last_pool)

    self.timer = 0
end

function M:onDestroy()
    lstg.RemoveResource("global", 2, "sprite:white")
    lstg.RemoveResource("global", 2, "sprite:black")
    lstg.RemoveResource("global", 1, "rt:white")
    lstg.RemoveResource("global", 1, "rt:black")
    unload_image("global", "image_2")
end

function M:onUpdate()
    self.timer = self.timer + 1
end

function M:onRender()
    window:applyCameraV()

    lstg.PushRenderTarget("rt:white")
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    lstg.PopRenderTarget()

    lstg.PushRenderTarget("rt:black")
    lstg.RenderClear(lstg.Color(255, 0, 0, 0))
    lstg.PopRenderTarget()

    lstg.Render("image_2", window.width / 2, window.height / 2, 0, window.height / 2160)
    lstg.Render("sprite:white", 256, 256)
    lstg.Render("sprite:black", 512, 256)
end

test.registerTest("test.Module.PostEffect.Blend.AlphaBal", M)
