local type = type
local math = require("math")
local lstg = require("lstg")
local _New = lstg._New
function lstg.New(class, ...)
    local o, init = _New(class)
    if init then
        class[1](o, ...)
    end
    return o
end
local _Del = lstg._Del
function lstg.Del(o, ...)
    if _Del(o) then
        o[1][2](o, ...)
    end
end
local _Kill = lstg._Kill
function lstg.Kill(o, ...)
    if _Kill(o) then
        o[1][6](o, ...)
    end
end
local _UpdateListFirst = lstg._UpdateListFirst
local _UpdateListNext = lstg._UpdateListNext
local _DetectListFirst = lstg._DetectListFirst
local _DetectListNext = lstg._DetectListNext
local objects = lstg.ObjTable()
function lstg.ObjList(group)
    if group < 0 or group >= 16 then
        local id = _UpdateListFirst()
        return function()
            if id == 0 then
                return nil, nil
            else
                local i, o = id, objects[id]
                id = _UpdateListNext(id)
                return i, o
            end
        end
    else
        local id = _DetectListFirst(group)
        return function()
            if id == 0 then
                return nil, nil
            else
                local i, o = id, objects[id]
                id = _DetectListNext(group, id)
                return i, o
            end
        end
    end
end
local _sin = lstg.sin
local _cos = lstg.cos
function lstg.SetV(o, v, a, update_rot)
    o.vx = v * _cos(a)
    o.vy = v * _sin(a)
    if update_rot then
        o.rot = a
    end
end
local sqrt = math.sqrt
local _atan2 = lstg.atan2
function lstg.GetV(o)
    local vx, vy = o.vx, o.vy
    return sqrt(vx * vx + vy * vy), _atan2(vy, vx)
end
local function _dxdy(a, b, c, d)
    if d then
        return c - a, d - b
    elseif type(c) == "number" then
        return b - a.x, c - a.y
    elseif c then
        return c.x - a, c.y - b
    else
        return b.x - a.x, b.y - a.y
    end
end
function lstg.Dist(a, b, c, d)
    local dx, dy = _dxdy(a, b, c, d)
    return sqrt(dx * dx + dy * dy)
end
function lstg.Angle(a, b, c, d)
    local dx, dy = _dxdy(a, b, c, d)
    return _atan2(dy, dx)
end
