---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Vector4
---@operator add(lstg.Vector4): lstg.Vector4
---@operator add(number): lstg.Vector4
---@operator sub(lstg.Vector4): lstg.Vector4
---@operator sub(number): lstg.Vector4
---@operator mul(lstg.Vector4): lstg.Vector4
---@operator mul(number): lstg.Vector4
---@operator div(lstg.Vector4): lstg.Vector4
---@operator div(number): lstg.Vector4
---@operator unm: lstg.Vector4
local Vector4 = {

    ---@type number
    x = 0.0,

    ---@type number
    y = 0.0,

    ---@type number
    z = 0.0,

    ---@type number
    w = 0.0,

}

--------------------------------------------------------------------------------
--- 方法
--- Methods

---@return number
function Vector4:length()
end

---@return lstg.Vector4 self
function Vector4:normalize()
end

---@return lstg.Vector4 newValue
function Vector4:normalized()
end

---@param v lstg.Vector4
---@return number
function Vector4:dot(v)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.Vector4
function Vector4.create()
end

---@param x number
---@param y number
---@param z number
---@param w number
---@return lstg.Vector4
function Vector4.create(x, y, z, w)
end

return Vector4
