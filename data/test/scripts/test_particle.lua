local imgui_exist, imgui = pcall(require, "imgui")
local Mouse = lstg.Input.Mouse
local Key = lstg.Input.Keyboard

local function set_touhou_world()
    local hscale = window.width / 384
    local vscale = window.height / 448
    local scale = math.min(hscale, vscale)
    local width = 384 * scale
    local height = 448 * scale
    local dx = (window.width - width) * 0.5
    local dy = (window.height - height) * 0.5
    lstg.SetViewport(dx, dx + width, dy, dy + height)
    lstg.SetScissorRect(dx, dx + width, dy, dy + height)
    lstg.SetOrtho(-192, 192, -224, 224)
    lstg.SetImageScale(1)
    lstg.SetFog()
end
local function get_touhou_world_cursor()
    local hscale = window.width / 384
    local vscale = window.height / 448
    local scale = math.min(hscale, vscale)
    local width = 384 * scale
    local height = 448 * scale
    local dx = (window.width - width) * 0.5
    local dy = (window.height - height) * 0.5
    local mx, my = lstg.GetMousePosition()
    local x = mx - dx
    local y = my - dy
    local xv = -192 + 384 * (x / width)
    local yv = -224 + 448 * (y / height)
    return xv, yv
end

local test = require("test")

---@class test.Module.Particle : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    lstg.LoadTexture("tex:particles", "res/particles.png")
    lstg.LoadImage("img:particle1", "tex:particles", 0, 0, 32, 32)
    lstg.LoadPS("ps:1", "res/ghost_fire_1.psi", "img:particle1")

    lstg.SetResourceStatus(old_pool)

    self.ps = lstg.ParticleSystemData("ps:1")
    self.ps:SetOldBehavior(false)
    self.timer = 0
end

function M:onDestroy()
    lstg.RemoveResource("global", 6, "ps:1")
    lstg.RemoveResource("global", 2, "img:particle1")
    lstg.RemoveResource("global", 1, "tex:particles")
end

function M:onUpdate()
    self.timer = self.timer + 1
    if imgui_exist then
        imgui.backend.ShowParticleSystemEditor(self.ps)
    end
    if Mouse.GetKeyState(Mouse.Right) or Mouse.GetKeyState(Mouse.Left) then
        local mx, my = get_touhou_world_cursor()
        if Mouse.GetKeyState(Mouse.Right) then
            self.ps:setActive(true)
        end
        self.ps:Update(1 / 60, mx, my, self.timer % 360)
    else
        self.ps:Update(1 / 60, 0, 0, self.timer % 360)
    end
end

function M:onRender()
    set_touhou_world()
    self.ps:Render(1)
end

test.registerTest("test.Module.Particle", M)
