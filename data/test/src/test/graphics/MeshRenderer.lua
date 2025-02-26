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

---@param cube lstg.Mesh
function M:initCube1(cube)
    local white = lstg.Color(255, 255, 255, 255)

    -- 正面

    cube:setVertex((4 * 0) + 0, -0.5,  0.5, -0.5, 0.0, 0.0, white)
    cube:setVertex((4 * 0) + 1,  0.5,  0.5, -0.5, 1.0, 0.0, white)
    cube:setVertex((4 * 0) + 2,  0.5, -0.5, -0.5, 1.0, 1.0, white)
    cube:setVertex((4 * 0) + 3, -0.5, -0.5, -0.5, 0.0, 1.0, white)

    cube:setIndex((6 * 0) + 0, (4 * 0) + 0)
    cube:setIndex((6 * 0) + 1, (4 * 0) + 1)
    cube:setIndex((6 * 0) + 2, (4 * 0) + 2)
    cube:setIndex((6 * 0) + 3, (4 * 0) + 0)
    cube:setIndex((6 * 0) + 4, (4 * 0) + 2)
    cube:setIndex((6 * 0) + 5, (4 * 0) + 3)

    -- 背面

    cube:setVertex((4 * 1) + 0,  0.5,  0.5, 0.5, 0.0, 0.0, white)
    cube:setVertex((4 * 1) + 1, -0.5,  0.5, 0.5, 1.0, 0.0, white)
    cube:setVertex((4 * 1) + 2, -0.5, -0.5, 0.5, 1.0, 1.0, white)
    cube:setVertex((4 * 1) + 3,  0.5, -0.5, 0.5, 0.0, 1.0, white)

    cube:setIndex((6 * 1) + 0, (4 * 1) + 0)
    cube:setIndex((6 * 1) + 1, (4 * 1) + 1)
    cube:setIndex((6 * 1) + 2, (4 * 1) + 2)
    cube:setIndex((6 * 1) + 3, (4 * 1) + 0)
    cube:setIndex((6 * 1) + 4, (4 * 1) + 2)
    cube:setIndex((6 * 1) + 5, (4 * 1) + 3)

    -- 左面

    cube:setVertex((4 * 2) + 0, -0.5,  0.5,  0.5, 0.0, 0.0, white)
    cube:setVertex((4 * 2) + 1, -0.5,  0.5, -0.5, 1.0, 0.0, white)
    cube:setVertex((4 * 2) + 2, -0.5, -0.5, -0.5, 1.0, 1.0, white)
    cube:setVertex((4 * 2) + 3, -0.5, -0.5,  0.5, 0.0, 1.0, white)

    cube:setIndex((6 * 2) + 0, (4 * 2) + 0)
    cube:setIndex((6 * 2) + 1, (4 * 2) + 1)
    cube:setIndex((6 * 2) + 2, (4 * 2) + 2)
    cube:setIndex((6 * 2) + 3, (4 * 2) + 0)
    cube:setIndex((6 * 2) + 4, (4 * 2) + 2)
    cube:setIndex((6 * 2) + 5, (4 * 2) + 3)

    -- 右面

    cube:setVertex((4 * 3) + 0, 0.5,  0.5, -0.5, 0.0, 0.0, white)
    cube:setVertex((4 * 3) + 1, 0.5,  0.5,  0.5, 1.0, 0.0, white)
    cube:setVertex((4 * 3) + 2, 0.5, -0.5,  0.5, 1.0, 1.0, white)
    cube:setVertex((4 * 3) + 3, 0.5, -0.5, -0.5, 0.0, 1.0, white)

    cube:setIndex((6 * 3) + 0, (4 * 3) + 0)
    cube:setIndex((6 * 3) + 1, (4 * 3) + 1)
    cube:setIndex((6 * 3) + 2, (4 * 3) + 2)
    cube:setIndex((6 * 3) + 3, (4 * 3) + 0)
    cube:setIndex((6 * 3) + 4, (4 * 3) + 2)
    cube:setIndex((6 * 3) + 5, (4 * 3) + 3)

    -- 提交

    self.cube:commit()
end

---@param cube lstg.Mesh
function M:initCube2(cube)
    local white = lstg.Color(255, 255, 255, 255)

    local v = cube:createVertexWriter()
    local i = cube:createIndexWriter()
    -- 正面
    v:vertex():position(-0.5,  0.5, -0.5):uv(0.0, 0.0):color(white)
    v:vertex():position( 0.5,  0.5, -0.5):uv(1.0, 0.0):color(white)
    v:vertex():position( 0.5, -0.5, -0.5):uv(1.0, 1.0):color(white)
    v:vertex():position(-0.5, -0.5, -0.5):uv(0.0, 1.0):color(white)
    i:index(
        (4 * 0) + 0,
        (4 * 0) + 1,
        (4 * 0) + 2,
        (4 * 0) + 0,
        (4 * 0) + 2,
        (4 * 0) + 3
    )
    -- 背面
    v:vertex():position( 0.5,  0.5, 0.5):uv(0.0, 0.0):color(white)
    v:vertex():position(-0.5,  0.5, 0.5):uv(1.0, 0.0):color(white)
    v:vertex():position(-0.5, -0.5, 0.5):uv(1.0, 1.0):color(white)
    v:vertex():position( 0.5, -0.5, 0.5):uv(0.0, 1.0):color(white)
    i:index(
        (4 * 1) + 0,
        (4 * 1) + 1,
        (4 * 1) + 2,
        (4 * 1) + 0,
        (4 * 1) + 2,
        (4 * 1) + 3
    )
    -- 左面
    v:vertex():position(-0.5,  0.5,  0.5):uv(0.0, 0.0):color(white)
    v:vertex():position(-0.5,  0.5, -0.5):uv(1.0, 0.0):color(white)
    v:vertex():position(-0.5, -0.5, -0.5):uv(1.0, 1.0):color(white)
    v:vertex():position(-0.5, -0.5,  0.5):uv(0.0, 1.0):color(white)
    i:index(
        (4 * 2) + 0,
        (4 * 2) + 1,
        (4 * 2) + 2,
        (4 * 2) + 0,
        (4 * 2) + 2,
        (4 * 2) + 3
    )
    -- 右面
    v:vertex():position(0.5,  0.5, -0.5):uv(0.0, 0.0):color(white)
    v:vertex():position(0.5,  0.5,  0.5):uv(1.0, 0.0):color(white)
    v:vertex():position(0.5, -0.5,  0.5):uv(1.0, 1.0):color(white)
    v:vertex():position(0.5, -0.5, -0.5):uv(0.0, 1.0):color(white)
    i:index(
        (4 * 3) + 0,
        (4 * 3) + 1,
        (4 * 3) + 2,
        (4 * 3) + 0,
        (4 * 3) + 2,
        (4 * 3) + 3
    )
    -- 提交
    cube:commit()
end

function M:initMesh()
    local white = lstg.Color(255, 255, 255, 255)

    self.mesh = Mesh.create({
        vertex_count = 4,
        index_count = 6,
    })

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

    self.cube = Mesh.create({
        vertex_count = 6 * 4, -- 6个面，每个面4个顶点
        index_count = 6 * 2 * 3, -- 6个面，每个面2个三角形，每个三角形3个顶点（索引）
    })
    self:initCube2(self.cube)

    self.cube_renderer = MeshRenderer.create(self.cube, self.texture)
end

function M:updateMesh()
    self.mesh_renderer:setPosition(window.width / 4, window.height / 2)
    self.mesh_renderer:setScale(480, 480, 480)
    self.mesh_renderer:setRotationYawPitchRoll(math.rad((self.timer / 3) % 360), math.rad(self.timer % 360), 0)

    self.cube_renderer:setPosition(window.width * 3 / 4, window.height / 2)
    self.cube_renderer:setScale(480, 480, 480)
    self.cube_renderer:setRotationYawPitchRoll(math.rad((self.timer / 3) % 360), math.rad(self.timer % 360), 0)
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
    lstg.SetZBufferEnable(1)
    lstg.ClearZBuffer(1)
    self.mesh_renderer:draw()
    self.cube_renderer:draw()
    lstg.SetZBufferEnable(0)
end

test.registerTest("test.graphics.MeshRenderer", M, "Graphics: MeshRenderer")
