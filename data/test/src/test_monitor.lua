local test = require("test")

---@class test.Module.Monitor : test.Base
local M = {}

function M:onCreate()
    local l = lstg.ListMonitor()
    for i, v in ipairs(l) do
        lstg.Print(i, v.x, v.y, v.width, v.height)
    end
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
    window:applyCameraV()
end

test.registerTest("test.Module.Monitor", M)
