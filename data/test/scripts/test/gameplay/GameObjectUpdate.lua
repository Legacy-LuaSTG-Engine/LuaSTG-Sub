local test = require("test")
local task = require("task")
local TaskManager = require("task.Manager")
local lstg = require("lstg")

local GROUP_ENEMY_BULLET = 1
local GROUP_PLAYER = 2

local function registerGameObjectClass(c)
    local f = function() end
    c[1] = c.init or f
    c[2] = c.del or f
    c[3] = c.frame or f
    c[4] = c.render or lstg.DefaultRenderFunc
    c[5] = c.colli or f
    c[6] = c.kill or f
    assert(type(c[1]) == "function")
    assert(type(c[2]) == "function")
    assert(type(c[3]) == "function")
    assert(type(c[4]) == "function")
    assert(type(c[5]) == "function")
    assert(type(c[6]) == "function")
    c.is_class = true
end

local Follower = {}
function Follower:init(parent)
    self.img = "block"
    self.hscale = 50 / 256
    self.vscale = 50 / 256
    self.parent = parent
    if lstg.IsValid(self.parent) then
        self.x = self.parent.x
        self.y = self.parent.y - 100
    else
        lstg.Del(self)
    end
end
function Follower:late_frame()
    if lstg.IsValid(self.parent) then
        self.x = self.parent.x
        self.y = self.parent.y - 100
    else
        lstg.Del(self)
    end
end

local Runner = {}
function Runner:init()
    self.y = window.height / 2
    lstg.SetV(self, window.width / 4 / 60, 0, true)
    self.img = "block"
    self.hscale = 60 / 256
    self.vscale = 60 / 256
end

registerGameObjectClass(Follower)
registerGameObjectClass(Runner)

---@class test.gameplay.GameObjectUpdate : test.Base
local M = {}

function M:onCreate()
    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTexture("block", "res/block.png", true)
    lstg.LoadImage("block", "block", 0, 0, 256, 256)
    lstg.SetResourceStatus(last_pool)

    lstg.SetBound(0, window.width, 0, window.height)
    lstg.ResetPool()
    self.objects = {}
    self.tasks = TaskManager.create()
    self.tasks:add(function()
        while true do
            local runner = lstg.New(Runner)
            local follower = lstg.New(Follower, runner)
            table.insert(self.objects, runner)
            table.insert(self.objects, follower)
            task.wait(60)
        end
    end)
end

function M:onDestroy()
    lstg.ResetPool()

    lstg.RemoveResource("global", 2, "block")
    lstg.RemoveResource("global", 1, "block")
end

function M:dispatchLateFrame()
    local ol = self.objects
    local n0 = #self.objects
    local n = n0
    for i = n0, 1, -1 do
        local o = ol[i]
        if lstg.IsValid(o) then
            local f = o[1].late_frame
            if f then
                f(o)
            end
        else
            o = ol[n]
            ol[n] = nil
            n = n - 1
        end
    end
end

function M:onUpdate()
    self.tasks:resume()
    lstg.AfterFrame(2)
    lstg.ObjFrame(2)
    self:dispatchLateFrame()
    ---@diagnostic disable-next-line: param-type-mismatch, missing-parameter
    lstg.CollisionCheck({
        {GROUP_PLAYER, GROUP_ENEMY_BULLET},
    })
    lstg.BoundCheck(2)
end

function M:onRender()
    window:applyCameraV()
    lstg.ObjRender()
end

test.registerTest("test.gameplay.GameObjectUpdate", M, "Gameplay: GameObject Update")
