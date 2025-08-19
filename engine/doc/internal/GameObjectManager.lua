---@diagnostic disable: missing-return, unused-local

---@class lstg
local lstg = require("lstg")

---@return lstg.GameObject[]
function lstg.ObjTable()
end

---@param class lstg.Class
---@return lstg.GameObject
function lstg._New(class)
end

---@param object lstg.GameObject
function lstg._Del(object)
end

---@param object lstg.GameObject
function lstg._Kill(object)
end

---@return number
function lstg._UpdateListFirst()
end

---@param id number
---@return number
function lstg._UpdateListNext(id)
end

---@param group_id number
---@return number
function lstg._DetectListFirst(group_id)
end

---@param group_id number
---@param id number
---@return number
function lstg._DetectListNext(group_id, id)
end
