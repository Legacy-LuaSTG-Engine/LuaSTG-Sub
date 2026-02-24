---@diagnostic disable: unused-local, missing-return, duplicate-set-field

--- 顶点索引写入器可以简化顶点索引数据操作  
--- 
--- 示例代码：  
--- 
--- ```lua
--- local mesh = ...
--- mesh:createIndexWriter()
---     :seek(0)
---     :index(0, 1, 3)
---     :index(1, 2, 3)
---     :commit()
--- ```
---@class lstg.Mesh.IndexWriter
local IndexWriter = {}

---@param index_index integer
---@return lstg.Mesh.IndexWriter
function IndexWriter:seek(index_index)
end

---@param vertex_index integer
---@param ... integer
---@return lstg.Mesh.IndexWriter
function IndexWriter:index(vertex_index, ...)
end

---@return lstg.Mesh.IndexWriter
function IndexWriter:commit()
end
