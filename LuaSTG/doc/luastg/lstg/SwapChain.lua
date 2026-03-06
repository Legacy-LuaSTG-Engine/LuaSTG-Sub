---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.SwapChain
local M = {}

--------------------------------------------------------------------------------
--- 交换链设置
--- SwapChain setup

---@return lstg.Display.Size
function M:getSize()
end

---@param width number
---@param height number
---@return boolean
function M:setSize(width, height)
end

---@return boolean allow
function M:getVSyncPreference()
end

---@param allow boolean
function M:setVSyncPreference(allow)
end

---@return lstg.SwapChain.ScalingMode mode
function M:getScalingMode()
end

---@param mode lstg.SwapChain.ScalingMode
function M:setScalingMode(mode)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

--- 获取主交换链  
--- 
--- Get instance of main SwapChain  
---@return lstg.SwapChain
function M.getMain()
end

return M
