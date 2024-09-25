local Display = require("lstg.Display")
local Window = require("lstg.Window")
local FrameStyle = require("lstg.Window.FrameStyle")
local test = require("test")

---@class test.Module.WindowAndDisplay : test.Base
local M = {}

function M:onCreate()
    local last = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTTF("body", "C:/Windows/Fonts/msyh.ttc", 0, 24)
    lstg.SetResourceStatus(last)
    self.main_window = Window.getMain()
    self.has_key_down = false
end

function M:onDestroy()
    lstg.RemoveResource("global", 8, "body")
end

function M:onUpdate()
    local Keyboard = lstg.Input.Keyboard
    if Keyboard.GetKeyState(Keyboard.Q) then
        self.has_key_down = true
        self.main_window:setStyle(FrameStyle.borderless)
    elseif Keyboard.GetKeyState(Keyboard.W) then
        self.has_key_down = true
        self.main_window:setStyle(FrameStyle.fixed)
    elseif Keyboard.GetKeyState(Keyboard.E) then
        self.has_key_down = true
        self.main_window:setStyle(FrameStyle.normal)
    elseif Keyboard.GetKeyState(Keyboard.O) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setWindowCornerPreference(false)
        end
    elseif Keyboard.GetKeyState(Keyboard.P) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setWindowCornerPreference(true)
        end
    else
        self.has_key_down = false
    end
end

function M:onRender()
    window:applyCameraV()
    local message = ""
    local function info(fmt, ...)
        message = message .. string.format(fmt, ...) .. "\n"
    end
    local list = Display.getAll()
    for i, display in ipairs(list) do
        info("display %d:", i)
        local sz1 = display:getSize()
        local pos1 = display:getPosition()
        local rc1 = display:getRect()
        info("    size: [%d x %d]", sz1.width, sz1.height)
        info("    position: (%d, %d)", pos1.x, pos1.y)
        info("    rect: [%d, %d, %d, %d] (%d, %d)", rc1.left, rc1.top, rc1.right, rc1.bottom, rc1.right - rc1.left, rc1.bottom - rc1.top)
        local sz2 = display:getWorkAreaSize()
        local pos2 = display:getWorkAreaPosition()
        local rc2 = display:getWorkAreaRect()
        info("    work area size: [%d x %d]", sz2.width, sz2.height)
        info("    work area position: (%d, %d)", pos2.x, pos2.y)
        info("    work area rect: [%d, %d, %d, %d] (%d, %d)", rc2.left, rc2.top, rc2.right, rc2.bottom, rc2.right - rc2.left, rc2.bottom - rc2.top)
        info("    display scale: %.2f", display:getDisplayScale())
        info("    primary: %s", tostring(display:isPrimary()))
    end
    local x, y = 0, window.height -- window.width / 2, window.height / 2
    lstg.RenderTTF("body", message, x, x, y, y, 0 + 0, lstg.Color(255, 0, 0, 0), 2)
end

test.registerTest("test.Module.WindowAndDisplay", M)
