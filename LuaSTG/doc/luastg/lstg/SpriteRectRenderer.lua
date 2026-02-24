---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.SpriteRectRenderer : lstg.SpriteBaseRenderer
local SpriteRectRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param left number
---@param right number
---@param bottom number
---@param top number
function SpriteRectRenderer:setRect(left, right, bottom, top)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.SpriteRectRenderer
function SpriteRectRenderer.create()
end

---@param sprite lstg.Sprite
---@return lstg.SpriteRectRenderer
function SpriteRectRenderer.create(sprite)
end

return SpriteRectRenderer
