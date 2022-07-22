local test = require("test")

---@class test.Module.Log : test.Base
local M = {}

function M:onCreate()
    lstg.SystemLog("aaaa sss")
    lstg.Print(1, 2, 3, true, false, "hhhh", "xxxx", nil, 1, nil, 2)
    print("a", 1, 2, 3, true, false, "hhhh", "xxxx", nil, 1, nil, 2)
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.Log", M)
