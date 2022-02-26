---=====================================
---fancy2d 颜色包装
---作者:Xiliusha
---邮箱:Xiliusha@outlook.com
---=====================================

----------------------------------------
---lstg.Color

---@class lstg.Color
local f2dcolor = {
	---alpha channel [[0~255]]
	a = 0,
	---red channel [[0~255]]
	r = 0,
	---green channel [[0~255]]
	g = 0,
	---blue channel [[0~255]]
	b = 0,
	---32bit color [[0x00000000~0xFFFFFFFF]]
	argb = 0,
	--[==[
	---hue [[0~360]]
	h = 0.0,
	---saturation [[0~100]]
	s = 0.0,
	---value [[0~100]]
	v = 0.0,
	--]==]
}

---返回颜色的4个分量，或者通过4个分量设置颜色
---@overload fun():number, number, number, number
---@overload fun(argb:number)
---@overload fun(a:number, r:number, g:number, b:number)
---@return number, number, number, number
function f2dcolor:ARGB()
end

--[==[
---返回颜色的4个分量，或者通过4个分量设置颜色，其中alpha取值为[[0~100]]
---@overload fun():number, number, number, number
---@overload fun(a:number, h:number, s:number, v:number)
---@return number, number, number, number
function color:AHSV()
end
--]==]

---构造颜色对象
---@overload fun(argb:number):lstg.Color
---@overload fun(a:number, r:number, g:number, b:number):lstg.Color
---@return lstg.Color
function lstg.Color(a, r, g, b)
end
