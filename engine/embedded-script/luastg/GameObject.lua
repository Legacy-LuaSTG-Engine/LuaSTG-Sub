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
                return nil
            else
                local o = objects[id]
                id = _UpdateListNext(id)
                return o
            end
        end
    else
        local id = _DetectListFirst(group)
        return function()
            if id == 0 then
                return nil
            else
                local o = objects[id]
                id = _DetectListNext(group, id)
                return o
            end
        end
    end
end
