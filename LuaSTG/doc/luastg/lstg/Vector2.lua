---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Vector2
---@operator add(lstg.Vector2): lstg.Vector2
---@operator add(number): lstg.Vector2
---@operator sub(lstg.Vector2): lstg.Vector2
---@operator sub(number): lstg.Vector2
---@operator mul(lstg.Vector2): lstg.Vector2
---@operator mul(number): lstg.Vector2
---@operator div(lstg.Vector2): lstg.Vector2
---@operator div(number): lstg.Vector2
---@operator unm: lstg.Vector2
local Vector2 = {

    ---@type number
    x = 0.0,

    ---@type number
    y = 0.0,

}

--------------------------------------------------------------------------------
--- 方法
--- Methods

---@return number
function Vector2:length()
end

---@return number
function Vector2:angle()
end

---@return number
function Vector2:degreeAngle()
end

---@return lstg.Vector2 self
function Vector2:normalize()
end

---@return lstg.Vector2 newValue
function Vector2:normalized()
end

---@param v lstg.Vector2
---@return number
function Vector2:dot(v)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.Vector2
function Vector2.create()
end

---@param x number
---@param y number
---@return lstg.Vector2
function Vector2.create(x, y)
end

return Vector2
