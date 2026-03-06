---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 接口
--- Interface

---@class lstg.SpriteBaseRenderer : lstg.Renderer
local SpriteBaseRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param sprite lstg.Sprite
function SpriteBaseRenderer:setSprite(sprite)
end

---@param color lstg.Color
function SpriteBaseRenderer:setColor(color)
end

---@param c1 lstg.Color
---@param c2 lstg.Color
---@param c3 lstg.Color
---@param c4 lstg.Color
function SpriteBaseRenderer:setColor(c1, c2, c3, c4)
end

---@param blend lstg.BlendMode
function SpriteBaseRenderer:setLegacyBlendState(blend)
end
