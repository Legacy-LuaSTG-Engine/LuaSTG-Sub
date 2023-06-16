local test = require("test")

---@class test.Module.ObjectiveResource : test.Base
local M = {}

function M:onCreate()
    self.timer = 0

    local resource_collection = lstg.ResourceManager.getResourceCollection("global")

    self.texture = resource_collection:createTextureFromFile("test:tex:1", "res/block.png")
    local texture_size = self.texture:getSize()
    lstg.Print(string.format("texture size: %dx%d", texture_size.x, texture_size.y))

    self.img1 = resource_collection:createSprite("test:img:1", self.texture,   0,   0, 128, 128)
    self.img2 = resource_collection:createSprite("test:img:2", self.texture, 128,   0, 128, 128)
    self.img3 = resource_collection:createSprite("test:img:3", self.texture,   0, 128, 128, 128)
    self.img4 = resource_collection:createSprite("test:img:4", self.texture, 128, 128, 128, 128)

    local sequence = {
        self.img1,
        self.img1,
        self.img1,
        --
        self.img2,
        self.img2,
        self.img2,
        --
        self.img3,
        self.img3,
        --
        self.img4,
    }

    self.ani1 = resource_collection:createSpriteSequence("test:ani:1", sequence, 8)

    lstg.Print(tostring(resource_collection:getTexture("test:tex:1")))
    lstg.Print(tostring(resource_collection:getSprite("test:img:1")))
    lstg.Print(tostring(resource_collection:getSpriteSequence("test:ani:1")))
end

function M:onDestroy()
    local resource_collection = lstg.ResourceManager.getResourceCollection("global")

    resource_collection:removeTexture(self.texture)

    resource_collection:removeSprite(self.img1)
    resource_collection:removeSprite(self.img2)
    resource_collection:removeSprite(self.img3)
    resource_collection:removeSprite(self.img4)

    resource_collection:removeSpriteSequence(self.ani1)
end

function M:onUpdate()
    self.timer = self.timer + 1
end

function M:onRender()
    window:applyCameraV()
    lstg.RenderAnimation("test:ani:1", self.timer, window.width / 2, window.height / 2)
end

test.registerTest("objective resource", M)
