local imgui = require("imgui")

local v1 = imgui.ImVec4();
assert(v1.x == 0.0)
assert(v1.y == 0.0)
assert(v1.z == 0.0)
assert(v1.w == 0.0)

local v2 = imgui.ImVec4(114.514, 19.19, 1.0, 2.0)
assert(math.abs(v2.x - 114.514) < 0.00001)
assert(math.abs(v2.y - 19.19) < 0.00001)
assert(math.abs(v2.z - 1.0) < 0.00001)
assert(math.abs(v2.w - 2.0) < 0.00001)

local v3 = imgui.ImVec4()
v3.x = 114.514
v3.y = 19.19
v3.z = 1.0
v3.w = 2.0
assert(math.abs(v3.x - 114.514) < 0.00001)
assert(math.abs(v3.y - 19.19) < 0.00001)
assert(math.abs(v3.z - 1.0) < 0.00001)
assert(math.abs(v3.w - 2.0) < 0.00001)
assert(v1 ~= v3)
assert(v2 == v3)

assert(v1 ~= nil)
assert(nil ~= v1)
assert(v1 ~= false)
assert(false ~= v1)
assert(v1 ~= 0)
assert(0 ~= v1)
assert(v1 ~= "0")
assert("0" ~= v1)
assert(v1 ~= {})
assert({} ~= v1)
