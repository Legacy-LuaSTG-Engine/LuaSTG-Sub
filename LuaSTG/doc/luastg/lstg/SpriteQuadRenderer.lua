---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.SpriteQuadRenderer : lstg.SpriteBaseRenderer
local SpriteQuadRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param x3 number
---@param y3 number
---@param x4 number
---@param y4 number
function SpriteQuadRenderer:setQuad(x1, y1, x2, y2, x3, y3, x4, y4)
end

---@param x1 number
---@param y1 number
---@param z1 number
---@param x2 number
---@param y2 number
---@param z2 number
---@param x3 number
---@param y3 number
---@param z3 number
---@param x4 number
---@param y4 number
---@param z4 number
function SpriteQuadRenderer:setQuad(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.SpriteQuadRenderer
function SpriteQuadRenderer.create()
end

---@param sprite lstg.Sprite
---@return lstg.SpriteQuadRenderer
function SpriteQuadRenderer.create(sprite)
end

return SpriteQuadRenderer
