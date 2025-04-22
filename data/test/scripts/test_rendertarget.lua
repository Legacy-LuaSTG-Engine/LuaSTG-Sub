local test = require("test")

---@class test.Module.RenderTarget : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.CreateRenderTarget("rt:test1")
    lstg.CreateRenderTarget("rt:test2")
    lstg.SetResourceStatus(old_pool)
    self.press_key = false
end

function M:onDestroy()
    lstg.RemoveResource("global", 1, "rt:test1")
    lstg.RemoveResource("global", 1, "rt:test2")
end

function M:onUpdate()
    local Keyboard = lstg.Input.Keyboard
    if not self.press_key then
        if Keyboard.GetKeyState(Keyboard.D1) then
            window:setSize(1280, 720)
            self.press_key = true
        elseif Keyboard.GetKeyState(Keyboard.D2) then
            window:setSize(1920, 1080)
            self.press_key = true
        end
    else
        if not Keyboard.GetKeyState(Keyboard.D1) and not Keyboard.GetKeyState(Keyboard.D2) then
            self.press_key = false
        end
    end
end

function M:onRender()
end

test.registerTest("test.Module.RenderTarget", M)
