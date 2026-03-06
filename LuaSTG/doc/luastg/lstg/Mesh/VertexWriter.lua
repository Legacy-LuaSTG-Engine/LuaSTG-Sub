---@diagnostic disable: unused-local, missing-return, duplicate-set-field

--- 顶点写入器可以简化顶点数据操作  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- local white = lstg.Color(255, 255, 255, 255)
--- mesh:createVertexWriter()
---     :seek(0)
---     :vertex():position(0.0, 0.0):uv(0.0, 1.0):color(white)
---     :vertex():position(0.5, 1.0):uv(0.5, 0.0):color(white)
---     :vertex():position(1.0, 0.0):uv(1.0, 1.0):color(white)
---     :commit()
--- ```
---@class lstg.Mesh.VertexWriter
local VertexWriter = {}

---@param vertex_index integer
---@return lstg.Mesh.VertexWriter
function VertexWriter:seek(vertex_index)
end

---@return lstg.Mesh.VertexWriter
function VertexWriter:vertex()
end

---@see lstg.Mesh.setVertex
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number
---@return lstg.Mesh.VertexWriter
function VertexWriter:vertex(x, y, z, u, v, r, g, b, a)
end

---@see lstg.Mesh.setVertex
---@param x number
---@param y number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number
---@return lstg.Mesh.VertexWriter
function VertexWriter:vertex(x, y, u, v, r, g, b, a)
end

---@see lstg.Mesh.setVertex
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh.VertexWriter
function VertexWriter:vertex(x, y, z, u, v, color)
end

---@see lstg.Mesh.setVertex
---@param x number
---@param y number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh.VertexWriter
function VertexWriter:vertex(x, y, u, v, color)
end

---@see lstg.Mesh.setPosition
---@param x number
---@param y number
---@param z number?
---@return lstg.Mesh.VertexWriter
function VertexWriter:position(x, y, z)
end

---@see lstg.Mesh.setUv
---@param u number
---@param v number
---@return lstg.Mesh.VertexWriter
function VertexWriter:uv(u, v)
end

---@see lstg.Mesh.setColor
---@param r number
---@param g number
---@param b number
---@param a number?
---@return lstg.Mesh.VertexWriter
function VertexWriter:color(r, g, b, a)
end

---@see lstg.Mesh.setColor
---@param color lstg.Color
---@return lstg.Mesh.VertexWriter
function VertexWriter:color(color)
end

---@return lstg.Mesh.VertexWriter
function VertexWriter:commit()
end
