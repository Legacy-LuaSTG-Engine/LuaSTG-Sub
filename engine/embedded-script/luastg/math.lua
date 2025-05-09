local math = require("math")
local lstg = require("lstg")

local rad = math.rad
local deg = math.deg
local sin = math.sin
local cos = math.cos
local tan = math.tan
local asin = math.asin
local acos = math.acos
local atan = math.atan
local atan2 = math.atan2 or math.atan

function lstg.sin(x) return sin(rad(x)) end
function lstg.cos(x) return cos(rad(x)) end
function lstg.tan(x) return tan(rad(x)) end
function lstg.asin(x) return deg(asin(x)) end
function lstg.acos(x) return deg(acos(x)) end
function lstg.atan(...) return deg(atan(...)) end
function lstg.atan2(y, x) return deg(atan2(y, x)) end
