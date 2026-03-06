---@diagnostic disable: missing-return, unused-local

--------------------------------------------------------------------------------
--- 类
--- Class

--- 字体库用于储存一个或多个字体文件的信息，它还会将字体族相同的字体合并。
--- 
--- A font collection is used to store information about one or more font files,
--- and it also merges fonts belonging to the same font family.
---@class lstg.FontCollection
local FontCollection = {}

--------------------------------------------------------------------------------
--- 字体库操作
--- Collection operations

--- 将字体文件注册到字体库中。
--- 
--- 你还需要调用 `build` 构建字体库。
--- 
--- Register the font file to the font collection.
--- 
--- You also need to call `build` to update the font collection.
---@see lstg.FontCollection.build
---@param path string
---@return lstg.FontCollection? self
function FontCollection:addFile(path)
end

--- 构建字体库。
--- 
--- Build the font collection.
---@return lstg.FontCollection? self
function FontCollection:build()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.FontCollection
function FontCollection.create()
end

return FontCollection
