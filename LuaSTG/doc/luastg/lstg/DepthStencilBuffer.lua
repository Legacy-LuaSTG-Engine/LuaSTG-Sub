---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.DepthStencilBuffer
local M = {}

--------------------------------------------------------------------------------
--- 深度-模板缓冲区信息
--- DepthStencilBuffer info

---@return integer
function M:getWidth()
end

---@return integer
function M:getHeight()
end

---@return lstg.Texture2D texture
function M:getTexture()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

--- 创建深度-模板缓冲区 
---@param width integer  
---@param height integer  
---@return lstg.DepthStencilBuffer
function M.create(width, height)
end

return M
