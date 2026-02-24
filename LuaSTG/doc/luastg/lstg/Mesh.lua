---@diagnostic disable: unused-local, missing-return, duplicate-set-field

local PrimitiveTopology = require("lstg.PrimitiveTopology")

--------------------------------------------------------------------------------
--- 迁移指南

-- 在 LuaSTG Sub v0.18.4-beta 中添加了实验性的 lstg.Mesh 对象和渲染方法 lstg.RenderMesh
-- 在 LuaSTG Sub v0.22.0 中，新的 lstg.Mesh 对象取代了旧版本的实验性对象
-- 旧版本的对象方法已作兼容性处理，目前版本仍然可以使用，但将会在未来移除

--@param color lstg.Color
--function M:setAllVertexColor(color)
--end

--- 设置顶点坐标  
-- index 是顶点的索引，从 0 开始  
--@param index number
--@param x number
--@param y number
--@param z number
--function M:setVertexPosition(index, x, y, z)
--end

-- 设置顶点纹理坐标  
-- index 是顶点的索引，从 0 开始  
-- u, v 是归一化纹理坐标，取值范围 [0.0, 1.0]  
--@param index number
--@param u number
--@param v number
--function M:setVertexCoords(index, u, v)
--end

-- 设置顶点纹理坐标  
-- index 是顶点的索引，从 0 开始  
--@param index number
--@param color lstg.Color
--function M:setVertexColor(index, color)
--end

--@param vertex_count number
--@param index_count number
--@return lstg.experiment.Mesh
--function lstg.MeshData(vertex_count, index_count)
--end

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Mesh
local Mesh = {}

--------------------------------------------------------------------------------
--- 网格信息
--- Mesh info

---@return integer
function Mesh:getVertexCount()
end

---@return integer
function Mesh:getIndexCount()
end

---@return lstg.PrimitiveTopology
function Mesh:getPrimitiveTopology()
end

---@return boolean
function Mesh:isReadOnly()
end

--------------------------------------------------------------------------------
--- 低级网格数据操作
--- Low-level Mesh data operation

--- 设置顶点数据  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@see lstg.Mesh.setPosition
---@see lstg.Mesh.setUv
---@see lstg.Mesh.setColor
---@param vertex_index integer
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh
function Mesh:setVertex(vertex_index, x, y, z, u, v, color)
end

--- 设置顶点数据  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@see lstg.Mesh.setPosition
---@see lstg.Mesh.setUv
---@see lstg.Mesh.setColor
---@param vertex_index integer
---@param x number
---@param y number
---@param u number
---@param v number
---@param color lstg.Color
---@return lstg.Mesh
function Mesh:setVertex(vertex_index, x, y, u, v, color)
end

--- 设置顶点数据  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@see lstg.Mesh.setPosition
---@see lstg.Mesh.setUv
---@see lstg.Mesh.setColor
---@param vertex_index integer
---@param x number
---@param y number
---@param z number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number
---@return lstg.Mesh
function Mesh:setVertex(vertex_index, x, y, z, u, v, r, g, b, a)
end

--- 设置顶点数据  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@see lstg.Mesh.setPosition
---@see lstg.Mesh.setUv
---@see lstg.Mesh.setColor
---@param vertex_index integer
---@param x number
---@param y number
---@param u number
---@param v number
---@param r number
---@param g number
---@param b number
---@param a number
---@return lstg.Mesh
function Mesh:setVertex(vertex_index, x, y, u, v, r, g, b, a)
end

--- 设置顶点位置坐标  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@param vertex_index integer 从 0 开始  
---@param x number
---@param y number
---@param z number? 默认为 0.0  
---@return lstg.Mesh
function Mesh:setPosition(vertex_index, x, y, z)
end

--- 设置顶点纹理坐标  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@param vertex_index integer 从 0 开始  
---@param u number 取值范围 [0.0, 1.0]  
---@param v number 取值范围 [0.0, 1.0]  
---@return lstg.Mesh
function Mesh:setUv(vertex_index, u, v)
end

--- 设置顶点颜色  
--- 顶点颜色的用法由着色器决定  
--- 警告：由于历史原因，LuaSTG 的颜色空间管理十分混乱  
--- 警告：目前 LuaSTG 基本上按照 sRGB 颜色空间计算和渲染  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@param vertex_index integer 从 0 开始  
---@param r number 取值范围 [0.0, 1.0]  
---@param g number 取值范围 [0.0, 1.0]  
---@param b number 取值范围 [0.0, 1.0]  
---@param a number? 取值范围 [0.0, 1.0]，默认为 1.0  
---@return lstg.Mesh
function Mesh:setColor(vertex_index, r, g, b, a)
end

--- 设置顶点颜色  
--- 顶点颜色的用法由着色器决定  
--- 警告：由于历史原因，LuaSTG 的颜色空间管理十分混乱  
--- 警告：目前 LuaSTG 基本上按照 sRGB 颜色空间计算和渲染  
--- 警告：`lstg.Color` 的精度只有 8 位无符号整数/通道，而不是 32 位浮点数/通道，请留意精度问题  
--- 以下情况会抛出错误：  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@param vertex_index integer 从 0 开始  
---@param color lstg.Color
---@return lstg.Mesh
function Mesh:setColor(vertex_index, color)
end

--- 设置顶点索引  
--- 以下情况会抛出错误：  
--- * 未启用顶点索引（创建网格对象时 `index_count` 为 0 或未填写该字段）  
--- * 顶点索引的索引（`index_index`）超出范围  
--- * 顶点索引（`vertex_index`）超出范围  
--- * 网格对象处于只读状态  
---@param index_index integer 从 0 开始  
---@param vertex_index integer 从 0 开始  
---@return lstg.Mesh
function Mesh:setIndex(index_index, vertex_index)
end

--------------------------------------------------------------------------------
--- 高级网格数据操作
--- High-level Mesh data operation

---@return lstg.Mesh.VertexWriter
function Mesh:createVertexWriter()
end

---@return lstg.Mesh.IndexWriter
function Mesh:createIndexWriter()
end

---@return lstg.Mesh.PrimitiveWriter
function Mesh:createPrimitiveWriter()
end

--------------------------------------------------------------------------------
--- 网格操作
--- Mesh operations

--- 提交内容到图形设备（显卡/显示适配器）  
--- 注意：对于只读状态的网格，调用该成员函数会抛出错误  
---@return boolean
function Mesh:commit()
end

--- 网格对象将变成只读状态，并释放为了动态提交内容所需的内存  
--- 未提交的内容会被直接丢弃  
function Mesh:setReadOnly()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@class lstg.Mesh.CreateOptions
local CreateOptions = {
    --- 顶点数量  
    ---@type integer
    vertex_count = 0,

    --- 顶点索引数量  
    --- 不提供该字段或将值设为 0 时，不启用顶点索引功能  
    ---@type integer?
    index_count = 0,

    --- 是否开启顶点索引压缩  
    --- 默认情况下，开启顶点索引压缩  
    --- 未开启时，使用 32 位整数作为顶点索引  
    --- 开启后，使用 16 位整数作为顶点索引，此时 `vertex_count` 最大值为 65536  
    ---@type boolean?
    vertex_index_compression = true;

    --- 是否开启顶点颜色压缩  
    --- 默认情况下，开启顶点颜色压缩  
    --- 未开启时，使用 R32G32B32A32 FLOAT 格式储存顶点颜色  
    --- 开启后，使用 B8G8R8A8 UNORM 格式储存顶点颜色  
    ---@type boolean?
    vertex_color_compression = true,

    --- 图元拓扑  
    --- 默认情况下，使用三角形列表（`lstg.PrimitiveTopology.triangle_list`）  
    ---@see lstg.PrimitiveTopology
    ---@type lstg.PrimitiveTopology?
    primitive_topology = PrimitiveTopology.triangle_list,
}

--- 示例代码：  
--- ```lua
--- local Mesh = require("lstg.Mesh")
--- local PrimitiveTopology = require("lstg.PrimitiveTopology")
--- local mesh = Mesh.create({
---     vertex_count = 10,
---     index_count = 24,
---     primitive_topology = PrimitiveTopology.triangle_list,
--- })
--- ```
---@param options lstg.Mesh.CreateOptions
---@return lstg.Mesh
function Mesh.create(options)
end

return Mesh
