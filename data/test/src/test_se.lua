local test = require("test")

---@class test.Module.SoundEffect : test.Base
local M = {}

lstg.FileManager.AddSearchPath("deep/")

function M:onCreate()
    local pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadSound("se:ok00", "res/se_ok00.wav")
    lstg.SetResourceStatus(pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 5, "se:ok00")
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("Test BAD SoundEffect", M)
