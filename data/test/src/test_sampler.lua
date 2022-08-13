local test = require("test")

---@class test.Module.Sampler : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    lstg.LoadTexture("tex:block", "res/block.png")
    lstg.SetTextureSamplerState("tex:block", "linear+wrap")

    lstg.SetResourceStatus(old_pool)

    self.timer = 0
end

function M:onDestroy()
    lstg.RemoveResource("global", 1, "tex:block")
end

function M:onUpdate()
    self.timer = self.timer + 1
end

function M:onRender()
    window:applyCameraV()
    local cx, cy = window.width / 2, window.height / 2
    local cc = lstg.Color(255, 255, 255, 255)
    local dt = self.timer
    lstg.RenderTexture("tex:block", "",
        { cx - 256.0, cy + 256.0, 0.5,   0.0,   0.0 + dt, cc },
        { cx + 256.0, cy + 256.0, 0.5, 256.0,   0.0 + dt, cc },
        { cx + 256.0, cy - 256.0, 0.5, 256.0, 256.0 + dt, cc },
        { cx - 256.0, cy - 256.0, 0.5,   0.0, 256.0 + dt, cc })
end

test.registerTest("test.Module.Sampler", M)
