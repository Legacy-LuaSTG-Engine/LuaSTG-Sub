--------------------------------------------------------------------------------
--- LuaSTG Sub 数学适配库
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 移植指南

-- lstg.SampleBezier
-- 已经被 Xiliusha 移除，源代码也已经丢失
-- 如果有需要请使用旧版本 LuaSTG Ex Plus

--------------------------------------------------------------------------------
--- 角度制三角函数

---@param v number
---@return number
function M.sin(v) end

---@param v number
---@return number
function M.cos(v) end

---@param v number
---@return number
function M.asin(v) end

---@param v number
---@return number
function M.acos(v) end

---@param v number
---@return number
function M.tan(v) end

---@param v number
---@return number
function M.atan(v) end

---@param y number
---@param x number
---@return number
function M.atan2(y, x) end

return M
