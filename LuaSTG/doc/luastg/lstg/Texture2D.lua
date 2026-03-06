---@diagnostic disable: missing-return, unused-local

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Texture2D
local M = {}

--------------------------------------------------------------------------------
--- 纹理信息
--- Texture info

---@return integer
function M:getWidth()
end

---@return integer
function M:getHeight()
end

---@param default_sampler lstg.KnownSamplerState
function M:setDefaultSampler(default_sampler)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

--- 从文件创建纹理  
--- 参数 `mipmap_levels` 决定了 mipmap 级别数量  
--- * 不提供参数或值为 1 时：仅包含原始纹理作为第一级  
--- * 值大于 1 时：原始纹理作为第一级，从第二级开始纹理宽、高缩小为上一级的一半  
--- * 值为 0 时：自动生成从原始纹理到最小的 1x1 纹理的所有级别  
---@param path string  
---@param mipmap_levels integer?
---@return lstg.Texture2D
function M.createFromFile(path, mipmap_levels)
end

return M
