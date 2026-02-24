---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.RenderTarget
local M = {}

--------------------------------------------------------------------------------
--- 渲染目标信息
--- RenderTarget info

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

--- 创建渲染目标  
---@param width integer  
---@param height integer  
---@return lstg.RenderTarget
function M.create(width, height)
end

return M
