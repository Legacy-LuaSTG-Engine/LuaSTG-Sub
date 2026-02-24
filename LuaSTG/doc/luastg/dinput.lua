--------------------------------------------------------------------------------
--- Windows 平台 DirectInput 拓展
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 迁移指南

-- LuaSTG Plus 和 LuaSTG Ex Plus 简单粗暴地将 DirectInput 输入映射到
-- 0x92 (146) 到 0xB2 (177) 和 0xDF (223) 到 0xFE (254)
-- 不仅摇杆灵敏度不可修改，而且没有照顾到不同的设备
-- LuaSTG Sub 则将 DirectInput 的 API 以平台拓展库的方式暴露给开发者

-- 注意：DirectInput 拓展不会枚举已经支持 XInput 的设备
-- 要获取 XInput 设备的输入，请使用 XInput 拓展库

--------------------------------------------------------------------------------
--- 方法

---@diagnostic disable: missing-return

---@class dinput
local M = {}

--- 重新枚举设备，返回设备数量  
--- DirectInput 重新枚举设备的过程非常耗时，该方法应仅在需要的时候调用  
--- dinput 不可用或无可用设备时返回 0  
---@return number
function M.refresh()
end

--- 获得设备数量  
--- dinput 不可用或无可用设备时返回 0  
---@return number
function M.count()
end

--- 获取设备输入，每帧需要且只需调用一次
function M.update()
end

--- 获得设备的模拟量输入的取值范围  
--- 可用于解析 dinput.RawState  
--- 如果最小值与最大值相等则代表该模拟量不存在（与设备有关）  
--- 索引从 1 开始  
--- dinput 不可用时返回 nil  
---@param index number
---@return dinput.AxisRange
function M.getAxisRange(index)
    ---@class dinput.AxisRange
    local t = {
        XMin = 0,  YMin = 0,  ZMin = 0,
        XMax = 0,  YMax = 0,  ZMax = 0,
        RxMin = 0, RyMin = 0, RzMin = 0,
        RxMax = 0, RyMax = 0, RzMax = 0,
        Slider0Min = 0, Slider1Min = 0,
        Slider0Max = 0, Slider1Max = 0,
    }
    return t
end

--- 参考 https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)  
--- 索引从 1 开始  
--- dinput 不可用时返回 nil  
---@param index number
---@return dinput.RawState
function M.getRawState(index)
    ---@class dinput.RawState
    local t = {
        lX = 0,
        lY = 0,
        lZ = 0,
        lRx = 0,
        lRy = 0,
        lRz = 0,
        ---@type number[]
        rglSlider = { 0, 0 },
        --- 不指向任何方向时为 0xFFFF
        ---@type number[]
        rgdwPOV = { 0, 0, 0, 0 },
        ---@type boolean[]
        rgbButtons = {
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
        },
    }
    return t
end

return M
