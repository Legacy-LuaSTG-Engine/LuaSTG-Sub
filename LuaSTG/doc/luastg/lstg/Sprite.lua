---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Sprite
local Sprite = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

---@param texture lstg.Texture2D
function Sprite:setTexture(texture)
end

---@param x integer
---@param y integer
---@param width integer
---@param height integer
function Sprite:setTextureRect(x, y, width, height)
end

---@param x integer
---@param y integer
function Sprite:setCenter(x, y)
end

---@param unit_per_pixel number
function Sprite:setUnitPerPixel(unit_per_pixel)
end

---@return lstg.Sprite
function Sprite:clone()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@param texture lstg.Texture2D
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param unit_per_pixel number? default to 1.0
---@return lstg.Sprite
function Sprite.create(texture, x, y, width, height, unit_per_pixel)
end

---@param texture lstg.Texture2D
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param center_x integer
---@param center_y integer
---@param unit_per_pixel number? default to 1.0
---@return lstg.Sprite
function Sprite.create(texture, x, y, width, height, center_x, center_y, unit_per_pixel)
end

return Sprite
