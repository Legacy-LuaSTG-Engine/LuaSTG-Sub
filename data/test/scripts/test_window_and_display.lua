local Display = require("lstg.Display")
local Window = require("lstg.Window")
local FrameStyle = require("lstg.Window.FrameStyle")
local SwapChain = require("lstg.SwapChain")
local ScalingMode = require("lstg.SwapChain.ScalingMode")
local test = require("test")

---@class test.Module.WindowAndDisplay : test.Base
local M = {}

function M:onCreate()
    local last = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTTF("body", "C:/Windows/Fonts/msyh.ttc", 0, 24)
    lstg.SetResourceStatus(last)
    self.main_window = Window.getMain()
    self.main_swap_chain = SwapChain.getMain()
    self.has_key_down = false
end

function M:onDestroy()
    lstg.RemoveResource("global", 8, "body")
end

function M:onUpdate()
    local displays = Display.getAll()
    local Keyboard = lstg.Input.Keyboard
    if Keyboard.GetKeyState(Keyboard.A) then
        self.has_key_down = true
        self.main_swap_chain:setSize(window.width, window.height)
    elseif Keyboard.GetKeyState(Keyboard.S) then
        self.has_key_down = true
        self.main_swap_chain:setSize(1280, 720)
    elseif Keyboard.GetKeyState(Keyboard.D) then
        self.has_key_down = true
        self.main_swap_chain:setSize(640, 360)
    elseif Keyboard.GetKeyState(Keyboard.Q) then
        self.has_key_down = true
        self.main_window:setWindowed(window.width, window.height, FrameStyle.borderless)
    elseif Keyboard.GetKeyState(Keyboard.W) then
        self.has_key_down = true
        self.main_window:setWindowed(window.width, window.height, FrameStyle.fixed)
    elseif Keyboard.GetKeyState(Keyboard.E) then
        self.has_key_down = true
        self.main_window:setWindowed(window.width, window.height, FrameStyle.normal)
    elseif Keyboard.GetKeyState(Keyboard.O) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setWindowCornerPreference(false)
        end
    elseif Keyboard.GetKeyState(Keyboard.D1) then
        self.has_key_down = true
        --self.main_window:setWindowed(window.width, window.height, FrameStyle.normal, displays[1])
        self.main_window:setFullscreen(displays[1])
    elseif Keyboard.GetKeyState(Keyboard.D2) then
        self.has_key_down = true
        if displays[2] then
            --self.main_window:setWindowed(window.width, window.height, FrameStyle.normal, displays[2])
            self.main_window:setFullscreen(displays[2])
        end
    elseif Keyboard.GetKeyState(Keyboard.P) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setWindowCornerPreference(true)
        end
    elseif Keyboard.GetKeyState(Keyboard.N) then
        self.has_key_down = true
        self.main_window:setCursorVisibility(true)
    elseif Keyboard.GetKeyState(Keyboard.M) then
        self.has_key_down = true
        self.main_window:setCursorVisibility(false)
    elseif Keyboard.GetKeyState(Keyboard.K) then
        self.has_key_down = true
        self.main_swap_chain:setScalingMode(ScalingMode.stretch)
    elseif Keyboard.GetKeyState(Keyboard.L) then
        self.has_key_down = true
        self.main_swap_chain:setScalingMode(ScalingMode.aspect_ratio)
    elseif Keyboard.GetKeyState(Keyboard.V) then
        self.has_key_down = true
        self.main_swap_chain:setVSyncPreference(not self.main_swap_chain:getVSyncPreference())
    elseif Keyboard.GetKeyState(Keyboard.D9) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setTitleBarAutoHidePreference(true)
        end
    elseif Keyboard.GetKeyState(Keyboard.D8) then
        self.has_key_down = true
        local ext = self.main_window:queryInterface("lstg.Window.Windows11Extension")
        if ext then
            ext:setTitleBarAutoHidePreference(false)
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
    info("main window:")
    local wsz = self.main_window:getClientAreaSize()
    info("    size: [%d x %d]", wsz.width, wsz.height)
    info("    display scale: %.2f", tostring(self.main_window:getDisplayScale()))
    info("    cursor visibility: %s", tostring(self.main_window:getCursorVisibility()))
    info("main swap chain:")
    info("    vsync: %s", tostring(self.main_swap_chain:getVSyncPreference()))
    local list = Display.getAll()
    for i, display in ipairs(list) do
        info("display %d:", i)
        info("    friendly name: %s", display:getFriendlyName())
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
