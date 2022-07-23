local test = require("test")

---@class test.Module.FileSystem : test.Base
local M = {}

function M:onCreate()
    local list = lstg.FileManager.EnumFiles("src/")
    for _, v in ipairs(list) do
        lstg.Print(v[1], v[2])
    end
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.FileSystem", M)
