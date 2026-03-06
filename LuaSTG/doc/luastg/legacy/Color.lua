---@diagnostic disable: missing-return, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

--- 4 通道，每通道 8 位色深，32 位颜色  
--- 
--- 4 channels, 8-bits per channel, 32-bits color  
--- 
--- ```c
--- union Color {
---     struct ColorComponents {
---         uint8_t b;
---         uint8_t g;
---         uint8_t r;
---         uint8_t a;
---     } components;
---     uint32_t argb;
--- };
--- ```
---@class lstg.Color
local Color = {}

--- Alpha（透明度）通道，0 到 255 的整数，值越小越透明  
--- 
--- Alpha channel, from 0 to 255 integer,
--- smaller value is more transparent
Color.a = 0

--- 红色通道，0 到 255 的整数  
--- 
--- Red channel, from 0 to 255 integer  
Color.r = 0

--- 绿色通道，0 到 255 的整数  
--- 
--- Green channel, from 0 to 255 integer  
Color.g = 0

--- 蓝色通道，0 到 255 的整数  
--- 
--- Blue channel, from 0 to 255 integer  
Color.b = 0

--- 32 位整数，16 进制格式为 AARRGGBB    
--- 
--- 32-bits integer, in AARRGGBB hexadecimal format  
Color.argb = 0x00000000

--- 无参数调用，返回 a、r、g、b 通道值  
--- 
--- Call without parameters, returns a, r, g, b channel values  
---@see lstg.Color.a
---@see lstg.Color.r
---@see lstg.Color.g
---@see lstg.Color.b
---@return number a
---@return number r
---@return number g
---@return number b
function Color:ARGB()
end

---@see lstg.Color.argb
---@param argb number
function Color:ARGB(argb)
end

---@see lstg.Color.a
---@see lstg.Color.r
---@see lstg.Color.g
---@see lstg.Color.b
---@param a number
---@param r number
---@param g number
---@param b number
function Color:ARGB(a, r, g, b)
end



local M = {}

---@see lstg.Color.argb
---@param argb number
---@return lstg.Color
function M.Color(argb)
end

---@see lstg.Color.a
---@see lstg.Color.r
---@see lstg.Color.g
---@see lstg.Color.b
---@param a number
---@param r number
---@param g number
---@param b number
---@return lstg.Color
function M.Color(a, r, g, b)
end

return M
