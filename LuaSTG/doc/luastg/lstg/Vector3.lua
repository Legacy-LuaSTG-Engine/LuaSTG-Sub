---@diagnostic disable: missing-return, unused-local, duplicate-set-field

--------------------------------------------------------------------------------
--- 类
--- Class

---@class lstg.Vector3
---@operator add(lstg.Vector3): lstg.Vector3
---@operator add(number): lstg.Vector3
---@operator sub(lstg.Vector3): lstg.Vector3
---@operator sub(number): lstg.Vector3
---@operator mul(lstg.Vector3): lstg.Vector3
---@operator mul(number): lstg.Vector3
---@operator div(lstg.Vector3): lstg.Vector3
---@operator div(number): lstg.Vector3
---@operator unm: lstg.Vector3
local Vector3 = {

    ---@type number
    x = 0.0,

    ---@type number
    y = 0.0,

    ---@type number
    z = 0.0,

}

--------------------------------------------------------------------------------
--- 方法
--- Methods

---@return number
function Vector3:length()
end

---@return lstg.Vector3 self
function Vector3:normalize()
end

---@return lstg.Vector3 newValue
function Vector3:normalized()
end

---@param v lstg.Vector3
---@return number
function Vector3:dot(v)
end

--------------------------------------------------------------------------------
--- 静态方法
--- Static methods

---@return lstg.Vector3
function Vector3.create()
end

---@param x number
---@param y number
---@param z number
---@return lstg.Vector3
function Vector3.create(x, y, z)
end

return Vector3
