local test = require("test")

---@class test.Module.HGEFont : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    
    lstg.LoadFont("hgefont:hgefont", "res/hgefont.fnt", false)

    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 7, "hgefont:hgefont")
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
    lstg.RenderText("hgefont:hgefont", "114514AABB", window.width / 2, window.height / 2, 1, 0 + 0)
end

test.registerTest("test.Module.HGEFont", M)
