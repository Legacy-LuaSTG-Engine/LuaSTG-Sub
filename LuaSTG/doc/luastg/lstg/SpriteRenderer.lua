---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.SpriteRenderer : lstg.SpriteBaseRenderer
local SpriteRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param x number
---@param y number
function SpriteRenderer:setPosition(x, y)
end

---@param x number
---@param y number?
function SpriteRenderer:setScale(x, y)
end

---@param r number
function SpriteRenderer:setRotation(r)
end

---@param x number
---@param y number
---@param rot number? default to 0
---@param hscale number? default to 1.0
---@param vscale number? default to hscale
function SpriteRenderer:setTransform(x, y, rot, hscale, vscale)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.SpriteRenderer
function SpriteRenderer.create()
end

---@param sprite lstg.Sprite
---@return lstg.SpriteRenderer
function SpriteRenderer.create(sprite)
end

return SpriteRenderer
