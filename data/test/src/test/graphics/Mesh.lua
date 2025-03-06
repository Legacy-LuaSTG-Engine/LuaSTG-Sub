local test = require("test")
local imgui = require("imgui")
local Mesh = require("lstg.Mesh")

local function load_image(name, path)
    lstg.LoadTexture(name, path, false)
    local w, h = lstg.GetTextureSize(name)
    lstg.LoadImage(name, name, 0, 0, w, h)
end

local function unload_image(pool, name)
    lstg.RemoveResource(pool, 2, name)
    lstg.RemoveResource(pool, 1, name)
end

---@class test.graphics.Mesh : test.Base
local M = {}

function M:onCreate()
    self.x = 0
    self.mesh = Mesh.create({
        vertex_count = 4,
        index_count = 6,
    })

    local last_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    load_image("image_2", "res/image_2.jpg")
    lstg.SetResourceStatus(last_pool)
end

function M:onDestroy()
    unload_image("global", "image_2")
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
    window:applyCameraV()

    lstg.Render("image_2", window.width / 2, window.height / 2, 0, window.height / 2160)
end

test.registerTest("test.graphics.Mesh", M, "Graphics: Mesh")
