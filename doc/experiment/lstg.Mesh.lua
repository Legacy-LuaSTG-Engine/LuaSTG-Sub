
---@class lstg.Mesh
local M = {}

---@return number
function M:getVertexCount()
end

---@return number
function M:getIndexCount()
end

---@param color lstg.Color
function M:setAllVertexColor(color)
end

--- 设置顶点索引  
--- index 是顶点索引的索引，从 0 开始  
--- value 是顶点索引的值，从 0 开始  
---@param index number
---@param value number
function M:setIndex(index, value)
end

--- 设置顶点  
--- index 是顶点的索引，从 0 开始  
--- u, v 是归一化纹理坐标，取值范围 [0.0, 1.0]  
---@param index number
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param color lstg.Color
function M:setVertex(index, x, y, z, u, v, color)
end

--- 设置顶点坐标  
--- index 是顶点的索引，从 0 开始  
---@param index number
---@param x number
---@param y number
---@param z number
function M:setVertexPosition(index, x, y, z)
end

--- 设置顶点纹理坐标  
--- index 是顶点的索引，从 0 开始  
--- u, v 是归一化纹理坐标，取值范围 [0.0, 1.0]  
---@param index number
---@param u number
---@param v number
function M:setVertexCoords(index, u, v)
end

--- 设置顶点纹理坐标  
--- index 是顶点的索引，从 0 开始  
---@param index number
---@param color lstg.Color
function M:setVertexColor(index, color)
end

---@param vertex_count number
---@param index_count number
---@return lstg.Mesh
function lstg.MeshData(vertex_count, index_count)
end
