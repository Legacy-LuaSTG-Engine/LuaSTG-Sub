local test = require("test")

local function load_texture(f)
    lstg.LoadTexture("tex:" .. f, "res/" .. f .. ".png", false)
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

    load_texture("sRGB")
    load_texture("linear")

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    unload_texture("sRGB")
    unload_texture("linear")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    local scale = 0.5
    lstg.Render("img:sRGB", window.width / 4 * 1, window.height / 2, 0, scale)
    lstg.Render("img:linear", window.width / 4 * 3, window.height / 2, 0, scale)
end

test.registerTest("test.Module.Texture", M)
