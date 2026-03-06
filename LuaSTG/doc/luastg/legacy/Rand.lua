---=====================================
---WALLE512 随机数发生器包装
---作者:Xiliusha
---邮箱:Xiliusha@outlook.com
---=====================================

---@diagnostic disable: missing-return

----------------------------------------
---lstg.Random

---@class lstg.Rand @WELL512随机数发生器
local random = {}

---设置随机数发生器的随机数种子
---@param seed number @随机数种子必须是整数
function random:Seed(seed)
end

---获取随机数发生器状态机当前的随机数种子
---@return number @整数
function random:GetSeed()
end

---随机获得n1到n2范围内的整数
---@param n1 number @必须是整数
---@param n2 number @必须是整数
---@return number @整数
function random:Int(n1, n2)
end

---随机获得f1到f2范围内的浮点数
---@param f1 number
---@param f2 number
---@return number
function random:Float(f1, f2)
end

---随机获得符号，-1、0、1
---@return number
function random:Sign()
end

local M = {}

---构造随机数发生器对象
---@return lstg.Rand
function M.Rand()
end

return M
