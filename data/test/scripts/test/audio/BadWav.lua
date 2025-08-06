local test = require("test")

---@class test.audio.BadWav : test.Base
local M = {}

function M:onCreate()
    local pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    pcall(function()
        lstg.LoadSound("se:ok00", "assets/se/se_ok00.wav")
    end)
    lstg.LoadSound("se:ok00", "assets/se/se_ok00_fixed.wav")
    lstg.SetResourceStatus(pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 5, "se:ok00")
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.audio.BadWav", M, "Audio: Bad WAV")
