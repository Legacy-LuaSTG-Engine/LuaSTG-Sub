local test = require("test")

---@class test.Module.ObjectiveResource : test.Base
local M = {}

function M:onCreate()
    local resource_set = lstg.ResourceManager.getResourceSet("global")
    local texture = resource_set:createTextureFromFile("test:tex:1", "res/block.png")
    local texture_size = texture:getSize()
    lstg.Print(string.format("texture size: %dx%d", texture_size.x, texture_size.y))
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("objective resource", M)
