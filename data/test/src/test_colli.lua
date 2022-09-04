local test = require("test")

local GROUP_PLAYER = 1
local GROUP_ENEMY_BULLET = 2

local object_class = {
    function() end,
    function() end,
    function() end,
    lstg.DefaultRenderFunc,
    function() end,
    function() end;
    is_class = true,
}

---@class test.Module.CollisionDetection : test.Base
local M = {}

function M:onCreate()
    lstg.SetBound(0, window.width, 0, window.height)
    lstg.ResetPool()
    for _ = 1, 1000 do
        local obj = lstg.New(object_class)
        obj.x = window.width / 2
        obj.y = window.height / 2
        obj.group = GROUP_ENEMY_BULLET
        obj.rect = false
        obj.a = 64
        obj.b = 64
    end
    for _ = 1, 100 do
        local obj = lstg.New(object_class)
        obj.x = window.width / 2
        obj.y = window.height / 2 - 48
        obj.group = GROUP_PLAYER
        obj.rect = false
        obj.a = 32
        obj.b = 32
    end
end

function M:onDestroy()
    lstg.ResetPool()
end

function M:onUpdate()
    lstg.ObjFrame()
    lstg.BoundCheck()
    lstg.CollisionCheck(GROUP_PLAYER, GROUP_ENEMY_BULLET)
    lstg.UpdateXY()
    lstg.AfterFrame()
end

function M:onRender()
    window:applyCameraV()
    lstg.ObjRender()
    lstg.RenderGroupCollider(GROUP_PLAYER, lstg.Color(128, 0, 255, 0))
    lstg.RenderGroupCollider(GROUP_ENEMY_BULLET, lstg.Color(128, 255, 0, 0))
end

test.registerTest("test.Module.CollisionDetection", M)
