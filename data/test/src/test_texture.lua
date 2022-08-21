local test = require("test")

local function load_texture(f)
    lstg.LoadTexture("tex:" .. f, "res/" .. f, false)
    local w, h = lstg.GetTextureSize("tex:" .. f)
    lstg.LoadImage("img:" .. f, "tex:" .. f, 0, 0, w, h)
end
local function unload_texture(f)
    lstg.RemoveResource("global", 2, "img:" .. f)
    lstg.RemoveResource("global", 1, "tex:" .. f)
end

---@class test.Module.Texture : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    load_texture("sRGB.png")
    load_texture("linear.png")
    load_texture("block.png")
    load_texture("block.qoi")

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    unload_texture("sRGB.png")
    unload_texture("linear.png")
    unload_texture("block.png")
    unload_texture("block.qoi")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    local scale = 0.5
    lstg.Render("img:sRGB.png", window.width / 4 * 1, window.height / 2, 0, scale)
    lstg.Render("img:linear.png", window.width / 4 * 3, window.height / 2, 0, scale)
    scale = 0.5
    lstg.Render("img:block.png", window.width / 4 * 2, window.height / 4 * 1, 0, scale)
    lstg.Render("img:block.qoi", window.width / 4 * 2, window.height / 4 * 3, 0, scale)
end

test.registerTest("test.Module.Texture", M)
