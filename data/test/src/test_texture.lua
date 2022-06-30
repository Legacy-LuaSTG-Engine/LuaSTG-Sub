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

    load_texture("wrigglewalk")
    load_texture("xiaosuiguwalk")

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    unload_texture("wrigglewalk")
    unload_texture("xiaosuiguwalk")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    lstg.Render("img:wrigglewalk", window.width / 4 * 1, window.height / 2, 0, 2)
    lstg.Render("img:xiaosuiguwalk", window.width / 4 * 3, window.height / 2, 0, 2)
end

test.registerTest("test.Module.Texture", M)
