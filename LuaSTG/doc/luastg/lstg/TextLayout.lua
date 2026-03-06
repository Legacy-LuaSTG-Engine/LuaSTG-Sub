---@diagnostic disable: missing-return, unused-local

--------------------------------------------------------------------------------
--- 类
--- Class

--- 文本布局储存了文本、字体、排版、样式、光栅化所需的信息。
---
--- 它可以用于显示大段文本，并处理自动换行、文本对齐等排版逻辑。
---
--- 渲染大段文本通常是一个复杂且耗时的操作，而文本布局可以将最终的结果光栅化为图像，降低渲染时的复杂度。
--- 
--- 文本布局用空间换时间，付出的代价是需要使用大量的内存和显存。
--- 
--- 这也意味着文本布局并不适合所有的应用场景：
--- 它应该用于文本内容非常多，但不会创建太多实例，且文本内容、布局、样式不会频繁变化的情况。
---
--- Text layout stores information required for rendering text — including the
--- text content, font, typesetting parameters, styling, and rasterization settings.
---
--- It can be used to display large blocks of text and handles typographic logic
--- such as line wrapping and text/paragraph alignment.
---
--- Rendering large blocks of text is typically a complex and time-consuming operation;
--- text layout mitigates this by rasterizing the final result into an image,
--- thereby reducing rendering complexity.
--- 
--- Text layout employs a space-for-time trade-off,
--- at the cost of requiring substantial memory and GPU memory (VRAM).
--- 
--- This also means that text layout is not suitable for all application scenarios:
--- it is intended for cases where the text content is extensive,
--- but only a limited number of layout instances are created —
--- and where the text content, layout, and styling remain relatively static
--- (i.e., do not change frequently).
---@class lstg.TextLayout
local TextLayout = {}

--------------------------------------------------------------------------------
--- 基本设置
--- Basic setting

---@param text string
---@return lstg.TextLayout self
function TextLayout:setText(text)
end

--- 设置文本布局使用的字体库。默认情况下使用的字体库为系统字体库。
--- 
--- 如果不传递 `font_collection` 参数，则恢复为使用默认的系统字体库。
--- 
--- Set the font collection to be used by the text layout.
--- By default, the system font collection is used.
--- 
--- If the `font_collection` parameter is not provided,
--- it reverts to using the default system font collection.
---@param font_collection lstg.FontCollection?
---@return lstg.TextLayout self
function TextLayout:setFontCollection(font_collection)
end

--- 设置文本布局使用的字体族，比如“Noto Sans SC”。
--- 
--- 虽然支持使用中文或者其他语言的字体族名称，但建议尽量使用英文名称。
--- 
--- Set the font family to be used by the text layout — for example, “Noto Sans SC”.
--- 
--- Although font family names in Chinese or other languages are supported,
--- it is recommended to use English names whenever possible.
---@param font_family_name string
---@return lstg.TextLayout self
function TextLayout:setFontFamily(font_family_name)
end

--- 设置字号，以像素为单位。
--- 
--- Set the font size, in pixels.
---@param font_size integer
---@return lstg.TextLayout self
function TextLayout:setFontSize(font_size)
end

--- 设置字重，即字体笔画的粗细。
--- 
--- 建议使用标准的字重枚举值，使用方式可以参考示例代码。
--- 
--- Set the font weight, i.e., the thickness of the font strokes.
--- 
--- It is recommended to use standard font weight enumeration values;
--- please refer to the example code for usage.
--- 
--- ```lua
--- local FontWeight = require("lstg.FontWeight")
--- text_layout:setFontWeight(FontWeight.normal)
--- ```
---@param font_weight lstg.FontWeight | integer
---@return lstg.TextLayout self
function TextLayout:setFontWeight(font_weight)
end

--- 设置字体样式。
--- 
--- “斜体（italic）”是专门设计的字体样式。
--- 如果需要显示斜体，且字体支持，优先使用该样式。
--- 
--- “倾斜（oblique）”可以用于不提供专门的斜体样式的字体。
--- 该样式在绘制文本时，强制倾斜文本，即“伪斜体”。
--- 
--- 请使用标准的字体样式枚举值，使用方式可以参考示例代码。
--- 
--- Set the font style.
--- 
--- `italic` is a specially designed font style —
--- when italic rendering is desired and the font supports it, this style should be preferred.
--- 
--- `oblique`, on the other hand, is intended for fonts that do not provide a dedicated italic variant;
--- it forcibly slants the glyphs during rendering, resulting in a “synthetic” or “faux” italic.
--- 
--- Please use standard font style enumeration values; refer to the example code for usage guidance.
--- 
--- ```lua
--- local FontStyle = require("lstg.FontStyle")
--- text_layout:setFontStyle(FontStyle.normal)
--- ```
---@param font_style lstg.FontStyle
---@return lstg.TextLayout self
function TextLayout:setFontStyle(font_style)
end

--- 设置字体的宽度样式，某些字体族会提供“窄”样式或“宽”样式的变体。
--- 
--- 请使用标准的字体宽度样式枚举值，使用方式可以参考示例代码。
--- 
--- Set the font width (stretch) style.
--- Some font families provide variant styles such as condensed (narrow) or expanded (wide).
--- 
--- Please use standard font width enumeration values; refer to the example code for usage guidance.
--- 
--- ```lua
--- local FontWidth = require("lstg.FontWidth")
--- text_layout:setFontWidth(FontWidth.normal)
--- ```
---@param font_width lstg.FontWidth
---@return lstg.TextLayout self
function TextLayout:setFontWidth(font_width)
end

--- 设置布局尺寸，以像素为单位。布局尺寸还会影响最终光栅化时产生的图像尺寸。
--- 
--- Set the layout dimensions, in pixels.
--- It also affect the size of the resulting rasterized image.
---@param width integer
---@param height integer
---@return lstg.TextLayout self
---@overload fun(self:lstg.TextLayout, size: lstg.Vector2)
function TextLayout:setLayoutSize(width, height)
end

--- 设置文本的对齐方式。一般情况下，该选项控制的是水平方向上的对齐方式。
--- 
--- `end` 是 Lua 语言的关键字，所以枚举值 `end` 不得不追加一个下划线变为 `end_`。
--- 
--- 请使用标准的文本对齐方式枚举值，使用方式可以参考示例代码。
--- 
--- Set the text alignment. By default, this option controls horizontal alignment.
--- 
--- Since `end` is a reserved keyword in Lua,
--- the enumeration value `end` must be suffixed with an underscore to become `end_`.
--- 
--- Please use standard text alignment enumeration values; refer to the example code for usage guidance.
--- 
--- ```lua
--- local TextAlignment = require("lstg.TextAlignment")
--- text_layout:setTextAlignment(TextAlignment.start)
--- ```
---@param text_alignment lstg.TextAlignment
---@return lstg.TextLayout self
function TextLayout:setTextAlignment(text_alignment)
end

--- 设置段落的对齐方式。一般情况下，该选项控制的是垂直方向上的对齐方式。
--- 
--- `end` 是 Lua 语言的关键字，所以枚举值 `end` 不得不追加一个下划线变为 `end_`。
--- 
--- 请使用标准的段落对齐方式枚举值，使用方式可以参考示例代码。
--- 
--- Set the paragraph alignment. By default, this option controls vertical alignment.
--- 
--- Since `end` is a reserved keyword in Lua,
--- the enumeration value `end` must be suffixed with an underscore to become `end_`.
--- 
--- Please use standard paragraph alignment enumeration values; refer to the example code for usage guidance.
--- 
--- ```lua
--- local ParagraphAlignment = require("lstg.ParagraphAlignment")
--- text_layout:setParagraphAlignment(ParagraphAlignment.center)
--- ```
---@param paragraph_alignment lstg.ParagraphAlignment
---@return lstg.TextLayout self
function TextLayout:setParagraphAlignment(paragraph_alignment)
end

--- 设置文本颜色。默认的文本颜色为白色。
--- 
--- Set the text color. The default text color is white.
---@param color lstg.Color | lstg.Vector4
---@return lstg.TextLayout self
function TextLayout:setTextColor(color)
end

--------------------------------------------------------------------------------
--- 描边设置
--- Stroke setting

--- 设置描边宽度。默认的描边宽度是 0，即禁用描边。
--- 
--- Set the stroke width. The default stroke width is 0, which disables stroking.
---@param stroke_width number
---@return lstg.TextLayout self
function TextLayout:setStrokeWidth(stroke_width)
end

--- 设置描边颜色。默认的描边颜色为黑色。
--- 
--- Set the stroke color. The default stroke color is black.
---@param color lstg.Color | lstg.Vector4
---@return lstg.TextLayout self
function TextLayout:setStrokeColor(color)
end

--------------------------------------------------------------------------------
--- 光栅化
--- Rasterization

--- 执行光栅化，生成图像缓存。
--- 
--- 请检查返回值，如果返回 `nil` 则表示发生了一些内部错误。可以查看日志获取详细原因。
--- 
--- Perform rasterization to generate an image cache.
--- 
--- Please check the return value:
--- if it is `nil`, an internal error has occurred. Refer to the logs for detailed error information.
---@return lstg.TextLayout? self
function TextLayout:build()
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.TextLayout
function TextLayout.create()
end

return TextLayout
