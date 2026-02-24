---@diagnostic disable: missing-return, unused-local

--------------------------------------------------------------------------------
--- 类
--- Class

--- 文本布局渲染器，API 设计上类似 `lstg.SpriteRenderer`。
--- 
--- 文本布局渲染器内部维护着渲染所需的纹理等资源。
--- 渲染器会追踪文本布局的内容变化，然后更新这些内部资源。
--- 
--- 因此：
--- **请为每个文本布局创建一个单独的文本渲染器，请勿复用单个文本渲染器，否则频繁的内部资源重建会降低性能。**
--- 
--- Text layout renderer — an API design similar to lstg.SpriteRenderer.
--- 
--- The text layout renderer internally manages resources required for rendering, such as textures.
--- It monitors changes to the text layout’s content and updates these internal resources accordingly.
--- 
--- Therefore:
--- **Please create a dedicated text renderer for each text layout—do not reuse a single text renderer,
--- as frequent internal resource reconstruction will degrade performance.**
---@class lstg.TextRenderer : lstg.Renderer
local TextRenderer = {}

--------------------------------------------------------------------------------
--- 设置
--- Setup

--- 设置用于绘制的文本布局。
--- 
--- **请为每个文本布局创建一个单独的文本渲染器，请勿复用单个文本渲染器，否则频繁的内部资源重建会降低性能。**
--- 
--- Set the text layout to be used for rendering.
--- 
--- **Please create a dedicated text renderer for each text layout—do not reuse a single text renderer,
--- as frequent internal resource reconstruction will degrade performance.**
---@param text_layout lstg.TextLayout
---@return lstg.TextRenderer self
function TextRenderer:setTextLayout(text_layout)
end

--------------------------------------------------------------------------------
--- 变换
--- Transform

---@param x number
---@param y number
---@return lstg.TextRenderer self
function TextRenderer:setPosition(x, y)
end

---@param x number
---@param y number?
---@return lstg.TextRenderer self
function TextRenderer:setScale(x, y)
end

---@param rot number
---@return lstg.TextRenderer self
function TextRenderer:setRotation(rot)
end

---@param x number
---@param y number
---@param rot number? default to 0
---@param hscale number? default to 1.0
---@param vscale number? default to hscale
---@return lstg.TextRenderer self
function TextRenderer:setTransform(x, y, rot, hscale, vscale)
end

--- 设置渲染锚点，作用类似于 Sprite 的“中心位置”。渲染锚点的默认位置是 (0, 0)，即左上角。
--- 
--- Set the rendering anchor point, which functions similarly to a Sprite’s “pivot point” or “origin”.
--- The default anchor point is (0, 0), i.e., the top-left corner.
---@param x number
---@param y number
---@return lstg.TextRenderer self
---@overload fun(self:lstg.TextRenderer, anchor:lstg.Vector2): lstg.TextRenderer
function TextRenderer:setAnchor(x, y)
end

--------------------------------------------------------------------------------
--- 渲染状态
--- Render state

---@param c1 lstg.Color
---@param c2 lstg.Color
---@param c3 lstg.Color
---@param c4 lstg.Color
---@return lstg.TextRenderer self
---@overload fun(self:lstg.TextRenderer, color:lstg.Color): lstg.TextRenderer
function TextRenderer:setColor(c1, c2, c3, c4)
end

---@param blend lstg.BlendMode
---@return lstg.TextRenderer self
function TextRenderer:setLegacyBlendState(blend)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@param text_layout lstg.TextLayout?
---@return lstg.TextRenderer
function TextRenderer.create(text_layout)
end

return TextRenderer
