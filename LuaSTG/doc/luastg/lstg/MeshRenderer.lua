---@diagnostic disable: unused-local, missing-return, duplicate-set-field

--------------------------------------------------------------------------------
--- 迁移指南

-- 在 LuaSTG Sub v0.18.4-beta 中添加了实验性的 lstg.Mesh 对象和渲染方法 lstg.RenderMesh
-- 在 LuaSTG Sub v0.22.0 中，lstg.MeshRenderer 对象取代渲染方法 lstg.RenderMesh
-- 旧版本的方法已作兼容性处理，目前版本仍然可以使用，但将会在未来移除

--@param tex_name string
--@param blend lstg.BlendMode
--@param mesh_obj lstg.experiment.Mesh
--function lstg.RenderMesh(tex_name, blend, mesh_obj)
--end

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.MeshRenderer : lstg.Renderer
local MeshRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param x number
---@param y number
---@param z number? @default 0.0
function MeshRenderer:setPosition(x, y, z)
end

---@param x number
---@param y number
---@param z number? @default 1.0
function MeshRenderer:setScale(x, y, z)
end

---@param yaw number
---@param pitch number
---@param roll number
function MeshRenderer:setRotationYawPitchRoll(yaw, pitch, roll)
end

---@param mesh lstg.Mesh?
function MeshRenderer:setMesh(mesh)
end

---@param texture lstg.Texture2D?
function MeshRenderer:setTexture(texture)
end

---@param blend lstg.BlendMode
function MeshRenderer:setLegacyBlendState(blend)
end

--------------------------------------------------------------------------------
--- 绘制
--- Draw

function MeshRenderer:draw()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.MeshRenderer
function MeshRenderer.create()
end

---@param mesh lstg.Mesh
---@param texture lstg.Texture2D
---@return lstg.MeshRenderer
function MeshRenderer.create(mesh, texture)
end

return MeshRenderer
