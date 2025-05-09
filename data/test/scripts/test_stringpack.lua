local test = require("test")

---@class test.Module.string_pack : test.Base
local M = {}

function M:onCreate()
    for k, v in pairs(string) do
        lstg.Print(k, v)
    end
    local str = string.pack("<BBBB", 1, 4, 16, 64)
    local v1, v2, v3, v4 = string.unpack("<BBBB", str)
    lstg.Print(v1, v2, v3, v4)
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.string_pack", M)
