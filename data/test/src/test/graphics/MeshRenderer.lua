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

function M:initMesh()
    self.mesh = Mesh.create({
        vertex_count = 4,
        index_count = 6,
    })

    local white = lstg.Color(255, 255, 255, 255)
    self.mesh:setVertex(0, -0.5,  0.5, 0.0, 0.0, white)
    self.mesh:setVertex(1,  0.5,  0.5, 1.0, 0.0, white)
    self.mesh:setVertex(2,  0.5, -0.5, 1.0, 1.0, white)
    self.mesh:setVertex(3, -0.5, -0.5, 0.0, 1.0, white)

    self.mesh:setIndex(0, 0)
    self.mesh:setIndex(1, 1)
    self.mesh:setIndex(2, 2)
    self.mesh:setIndex(3, 0)
    self.mesh:setIndex(4, 2)
    self.mesh:setIndex(5, 3)

    self.mesh:commit()

    self.texture = Texture2D.createFromFile("res/block.png", 0)
    self.mesh_renderer = MeshRenderer.create(self.mesh, self.texture)
end

function M:updateMesh()
    self.mesh_renderer:setPosition(window.width / 2, window.height / 2)
    self.mesh_renderer:setScale(480, 480, 480)
    self.mesh_renderer:setRotationYawPitchRoll(math.rad((self.timer / 3) % 360), math.rad(self.timer % 360), 0)
end

function M:onCreate()
    self.timer = 0
    self:initMesh()

    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    load_image("image_2", "res/image_2.jpg")
    lstg.SetResourceStatus(last_pool)
end

function M:onDestroy()
    unload_image("global", "image_2")
end

function M:onUpdate()
    self.timer = self.timer + 1
    self:updateMesh()

    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin("Graphics: MeshRenderer") then
    end
    ImGui.End()
end

function M:onRender()
    window:applyCamera3D()
    lstg.Render("image_2", window.width / 2, window.height / 2, 0, window.height / 2160)
    self.mesh_renderer:draw()
end

test.registerTest("test.graphics.MeshRenderer", M, "Graphics: MeshRenderer")
