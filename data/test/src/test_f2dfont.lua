local test = require("test")

---@class test.Module.Fancy2DFont : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    
    lstg.LoadFont("f2dfont:f2dfont", "res/f2dfont.xml", "res/f2dfont.png", false)

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 7, "f2dfont:f2dfont")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    lstg.RenderText("f2dfont:f2dfont", "114514AABB", window.width / 2, window.height / 2, 1, 0 + 0)
end

test.registerTest("test.Module.Fancy2DFont", M)
