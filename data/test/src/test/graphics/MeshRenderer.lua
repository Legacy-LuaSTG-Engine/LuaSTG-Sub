local test = require("test")
local imgui = require("imgui")
local Mesh = require("lstg.Mesh")
local MeshRenderer = require("lstg.MeshRenderer")
local Texture2D = require("lstg.Texture2D")

local function load_image(name, path)
    lstg.LoadTexture(name, path, false)
    local w, h = lstg.GetTextureSize(name)
    lstg.LoadImage(name, name, 0, 0, w, h)
end

local function unload_image(pool, name)
    lstg.RemoveResource(pool, 2, name)
    lstg.RemoveResource(pool, 1, name)
end

---@class test.graphics.MeshRenderer : test.Base
local M = {}

function M:onCreate()
    self.timer = 0
    self.texture = Texture2D.createFromFile("res/block.png", 0)
    self.mesh = Mesh.create({
        vertex_count = 4,
        index_count = 6,
    })
    self.mesh_renderer = MeshRenderer.create(self.mesh, self.texture)
end

function M:onDestroy()
end

function M:onUpdate()
    self.timer = self.timer + 1

    local white = lstg.Color(255, 255, 255, 255)
    self.mesh:setVertex(0
        , -0.5, 0.5
        , 0.0, 0.0
        , white
    )
    self.mesh:setVertex(1
        , 0.5, 0.5
        , 1.0, 0.0
        , white
    )
    self.mesh:setVertex(2
        , 0.5, -0.5
        , 1.0, 1.0
        , white
    )
    self.mesh:setVertex(3
        , -0.5, -0.5
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

    self.mesh_renderer:setPosition(window.width / 2, window.height / 2)
    self.mesh_renderer:setScale(16, 16, 16)
    self.mesh_renderer:setRotationYawPitchRoll(0, 0, 0)

    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Graphics: MeshRenderer") then
    end
    ImGui.End()
end

function M:onRender()
    window:applyCameraV()

    self.mesh_renderer:draw()
end

test.registerTest("test.graphics.MeshRenderer", M, "Graphics: MeshRenderer")
