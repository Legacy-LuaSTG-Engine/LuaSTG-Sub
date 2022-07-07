local test = require("test")

---@class test.Module.Mesh : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTexture("tex:linear", "res/linear.png", false)
    lstg.SetTextureSamplerState("tex:linear", "linear+wrap")
    lstg.SetResourceStatus(old_pool)

    self.timer = -1
    self.mesh = lstg.MeshData(6, 12)

    self.mesh:setIndex(0, 0)
    self.mesh:setIndex(1, 3)
    self.mesh:setIndex(2, 1)

    self.mesh:setIndex(3, 0)
    self.mesh:setIndex(4, 2)
    self.mesh:setIndex(5, 3)

    self.mesh:setIndex(6, 2)
    self.mesh:setIndex(7, 5)
    self.mesh:setIndex(8, 3)

    self.mesh:setIndex(9, 2)
    self.mesh:setIndex(10, 4)
    self.mesh:setIndex(11, 5)

    local x, y = window.width / 2, window.height / 2
    local w, h = lstg.GetTextureSize("tex:linear")

    self.mesh:setVertex(0, x - 512, y + 512, 0,    0 / w,    0 / h, 0xFFFFFFFF)
    self.mesh:setVertex(1, x - 512, y      , 0,    0 / w,  512 / h, 0xFFFFFFFF)
    self.mesh:setVertex(2, x      , y      , 0,  512 / w,  512 / h, 0xFFFFFFFF)
    self.mesh:setVertex(3, x      , y - 512, 0,  512 / w, 1024 / h, 0xFFFFFFFF)
    self.mesh:setVertex(4, x + 512, y + 512, 0, 1024 / w,    0 / h, 0xFFFFFFFF)
    self.mesh:setVertex(5, x + 512, y      , 0, 1024 / w,  512 / h, 0xFFFFFFFF)
end

function M:onDestroy()
    lstg.RemoveResource("global", 1, "tex:linear")
end

function M:onUpdate()
    self.timer = self.timer + 1
    local w, h = lstg.GetTextureSize("tex:linear")
    self.mesh:setVertexCoords(0,    0 / w,    0 / h + self.timer / h)
    self.mesh:setVertexCoords(1,    0 / w,  512 / h + self.timer / h)
    self.mesh:setVertexCoords(2,  512 / w,  512 / h + self.timer / h)
    self.mesh:setVertexCoords(3,  512 / w, 1024 / h + self.timer / h)
    self.mesh:setVertexCoords(4, 1024 / w,    0 / h + self.timer / h)
    self.mesh:setVertexCoords(5, 1024 / w,  512 / h + self.timer / h)
end

function M:onRender()
    window:applyCameraV()
    lstg.RenderMesh("tex:linear", "", self.mesh)
end

test.registerTest("test.Module.Mesh", M)
