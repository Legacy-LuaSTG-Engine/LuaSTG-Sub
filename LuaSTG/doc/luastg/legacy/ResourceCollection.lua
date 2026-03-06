
---@class lstg.ResourceCollection
local M = {}

--------------------------------------------------------------------------------
--- create texture

---@param texture_name string
---@param path string
---@return lstg.ResourceTexture
---@diagnostic disable-next-line: duplicate-set-field
function M:createTextureFromFile(texture_name, path)
    ---@diagnostic disable-next-line: missing-return
end

---@param texture_name string
---@param path string
---@param mipmap boolean
---@return lstg.ResourceTexture
---@diagnostic disable-next-line: duplicate-set-field
function M:createTextureFromFile(texture_name, path, mipmap)
    ---@diagnostic disable-next-line: missing-return
end

--------------------------------------------------------------------------------
--- create sprite

---@param sprite_name string
---@param texture string | lstg.ResourceTexture
---@return lstg.ResourceSprite
---@diagnostic disable-next-line: duplicate-set-field
function M:createSprite(sprite_name, texture)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param width number
---@param height number
---@return lstg.ResourceSprite
---@diagnostic disable-next-line: duplicate-set-field
function M:createSprite(sprite_name, texture, x, y, width, height)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param width number
---@param height number
---@param a number
---@param b number
---@return lstg.ResourceSprite
---@diagnostic disable-next-line: duplicate-set-field
function M:createSprite(sprite_name, texture, x, y, width, height, a, b)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param width number
---@param height number
---@param a number
---@param b number
---@param rect boolean
---@return lstg.ResourceSprite
---@diagnostic disable-next-line: duplicate-set-field
function M:createSprite(sprite_name, texture, x, y, width, height, a, b, rect)
    ---@diagnostic disable-next-line: missing-return
end

--------------------------------------------------------------------------------
--- create sprite sequence

---@param sprite_sequence_name string
---@param sprite_list lstg.ResourceSprite[]
---@param interval number
---@return lstg.ResourceSpriteSequence
---@diagnostic disable-next-line: duplicate-set-field
function M:createSpriteSequence(sprite_sequence_name, sprite_list, interval)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_sequence_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param sprite_width number
---@param sprite_height number
---@param sprite_columns number
---@param sprite_rows number
---@param interval number
---@return lstg.ResourceSpriteSequence
---@diagnostic disable-next-line: duplicate-set-field
function M:createSpriteSequence(sprite_sequence_name, texture, x, y, sprite_width, sprite_height, sprite_columns, sprite_rows, interval)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_sequence_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param sprite_width number
---@param sprite_height number
---@param sprite_columns number
---@param sprite_rows number
---@param interval number
---@param a number
---@param b number
---@return lstg.ResourceSpriteSequence
---@diagnostic disable-next-line: duplicate-set-field
function M:createSpriteSequence(sprite_sequence_name, texture, x, y, sprite_width, sprite_height, sprite_columns, sprite_rows, interval, a, b)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_sequence_name string
---@param texture string | lstg.ResourceTexture
---@param x number
---@param y number
---@param sprite_width number
---@param sprite_height number
---@param sprite_columns number
---@param sprite_rows number
---@param interval number
---@param a number
---@param b number
---@param rect boolean
---@return lstg.ResourceSpriteSequence
---@diagnostic disable-next-line: duplicate-set-field
function M:createSpriteSequence(sprite_sequence_name, texture, x, y, sprite_width, sprite_height, sprite_columns, sprite_rows, interval, a, b, rect)
    ---@diagnostic disable-next-line: missing-return
end

--------------------------------------------------------------------------------
--- remove

---@param texture string | lstg.ResourceTexture
function M:removeTexture(texture)
end

---@param sprite string | lstg.ResourceSprite
function M:removeSprite(sprite)
end

---@param sprite_sequence string | lstg.ResourceSpriteSequence
function M:removeSpriteSequence(sprite_sequence)
end

--------------------------------------------------------------------------------
--- get resource

---@param texture_name string
---@return lstg.ResourceTexture
function M:getTexture(texture_name)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_name string
---@return lstg.ResourceSprite
function M:getSprite(sprite_name)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_sequence_name string
---@return lstg.ResourceSpriteSequence
function M:getSpriteSequence(sprite_sequence_name)
    ---@diagnostic disable-next-line: missing-return
end

--------------------------------------------------------------------------------
--- is resource exist

---@param texture_name string
---@return boolean
function M:isTextureExist(texture_name)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_name string
---@return boolean
function M:isSpriteExist(sprite_name)
    ---@diagnostic disable-next-line: missing-return
end

---@param sprite_sequence_name string
---@return boolean
function M:isSpriteSequenceExist(sprite_sequence_name)
    ---@diagnostic disable-next-line: missing-return
end

return M
