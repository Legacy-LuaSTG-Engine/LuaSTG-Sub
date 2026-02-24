---=====================================
---Windows CPU时钟包装，高精度计时器
---作者:Xiliusha
---邮箱:Xiliusha@outlook.com
---=====================================

---@diagnostic disable: missing-return

----------------------------------------
---lstg.StopWatch

---@class lstg.StopWatch @毫秒级高精度计时器
local stopwatch = {}

---重置计时器所有状态
function stopwatch:Reset()
end

---暂停计时，需要注意，lstg.StopWatch计时器对象一创建就开始计时了
function stopwatch:Pause()
end

---从暂停中恢复计时
function stopwatch:Resume()
end

---获取流逝的时间，暂停状态下需要先恢复才能正确获取时间
---@return number @以秒为单位的时间长度
function stopwatch:GetElapsed()
end

local M = {}

---构造高精度微秒级计时器
---@return lstg.StopWatch
function M.StopWatch()
end

return M
