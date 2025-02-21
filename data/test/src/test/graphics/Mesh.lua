local test = require("test")
local imgui = require("imgui")
local Mesh = require("lstg.Mesh")

---@class test.graphics.Mesh : test.Base
local M = {}

function M:onCreate()
    self.x = 0
    self.mesh = Mesh.create({
        vertex_count = 4,
        index_count = 6,
    })
end

function M:onDestroy()
end

function M:onUpdate()
    self.x = self.x + 0.01

    local white = lstg.Color(255, 255, 255, 255)
    self.mesh:setVertex(0
        , self.x, 1.0
        , 0.0, 0.0
        , white
    )
    self.mesh:setVertex(0
        , self.x + 1.0, 1.0
        , 1.0, 0.0
        , white
    )
    self.mesh:setVertex(0
        , self.x + 1.0, 0.0
        , 1.0, 1.0
        , white
    )
    self.mesh:setVertex(0
        , self.x, 0.0
        , 0.0, 1.0
        , white
    )

    self.mesh:setIndex(0, 0)
    self.mesh:setIndex(1, 1)
    self.mesh:setIndex(2, 2)
    self.mesh:setIndex(3, 0)
    self.mesh:setIndex(4, 2)
    self.mesh:setIndex(5, 3)

    self.mesh:commit()

    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Graphics: Mesh") then
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.graphics.Mesh", M, "Graphics: Mesh")
