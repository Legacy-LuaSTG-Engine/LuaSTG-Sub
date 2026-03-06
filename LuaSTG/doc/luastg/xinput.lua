--------------------------------------------------------------------------------
--- Windows 平台手柄输入拓展
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

---@class xinput
local M = {}

--------------------------------------------------------------------------------
--- 迁移指南

-- LuaSTG Ex Plus 曾经存在过一个 XInputManager
-- LuaSTG Sub 对其进行重新设计

-- lstg.XInputManager():lstg.XInputManager 方法：
-- 该方法已去除
-- 在 LuaSTG Ex Plus 内曾经用于创建 lstg.XInputManager 对象
-- 但是 LuaSTG Ex Plus 后续更新中该方法已经弃用
-- 因为 lstg.XInputManager 改为作为单例使用
-- 如 GetKeyState 仅需 lstg.XInputManager.GetKeyState()

-- lstg.XInputManager.GetKeyState(index:number, keycode:number):boolean 方法：
-- 该方法已去除，可通过以下代码模拟
-- local xinput = require("xinput")
-- function lstg.XInputManager.GetKeyState(index, keycode)
--     return xinput.getKeyState(index, keycode)
-- end

-- lstg.XInputManager.GetTriggerState(index:number):number, number 方法：
-- 该方法已去除，可通过以下代码模拟
-- local xinput = require("xinput")
-- function lstg.XInputManager.GetTriggerState(index)
--     return xinput.getLeftTrigger(index), xinput.getRightTrigger(index)
-- end

-- lstg.XInputManager.GetThumbState(index:number):number, number, number, number 方法：
-- 该方法已去除，可通过以下代码模拟
-- local xinput = require("xinput")
-- function lstg.XInputManager.GetThumbState(index)
--     return xinput.getLeftThumbX(index)
--     , xinput.getLeftThumbY(index)
--     , xinput.getRightThumbX(index)
--     , xinput.getRightThumbY(index)
-- end

-- lstg.XInputManager.SetMotorSpeed(index:number
-- , low_freq_motor_speed:number
-- , high_freq_motor_speed:number) 方法：
-- 该方法已去除
-- 由于 XInput 本身的问题，该方法不得不去除

-- lstg.XInputManager.GetMotorSpeed(index:number):number, number 方法：
-- 该方法已去除
-- 理由同上

-- lstg.XInputManager.Refresh():number 方法：
-- 通过 xinput.refresh():number 代替

-- lstg.XInputManager.Update() 方法：
-- 通过 xinput.update() 代替
-- 行为变更注意：现在 update 必须显式调用，LuaSTG Sub 并未在引擎中自动调用 update 方法

-- lstg.XInputManager.GetDeviceCount():number 方法：
-- 该方法已去除，通过 xinput.refresh():number 代替

--------------------------------------------------------------------------------
--- 按键常量

--- 仅用于兼容
M.Null          = 0x0000

--- 手柄方向键，上
M.Up            = 0x0001

--- 手柄方向键，下
M.Down          = 0x0002

--- 手柄方向键，左
M.Left          = 0x0004

--- 手柄方向键，右
M.Right         = 0x0008

--- 手柄 start 按键（一般作为菜单键使用）
M.Start         = 0x0010

--- 手柄 back 按键（一般作为返回键使用）
M.Back          = 0x0020

--- 手柄左摇杆按键（按压摇杆）
M.LeftThumb     = 0x0040

--- 手柄右摇杆按键（按压摇杆）
M.RightThumb    = 0x0080

--- 手柄左肩键
M.LeftShoulder  = 0x0100

--- 手柄右肩键
M.RightShoulder = 0x0200

--- 手柄 A 按键
M.A             = 0x1000

--- 手柄 B 按键
M.B             = 0x2000

--- 手柄 X 按键
M.X             = 0x4000

--- 手柄 Y 按键
M.Y             = 0x8000

--------------------------------------------------------------------------------
--- 方法

--- 重新枚举手柄设备，获取手柄输入，并返回手柄数量
---@return number
function M.refresh() end

--- 获取手柄输入，每帧需要且只需调用一次
function M.update() end

--- [LuaSTG Sub v0.17.2 新增]  
--- 判断手柄是否已经连接  
--- 索引从 1 开始  
---@param index number
---@return boolean
function M.isConnected(index) end

--- 根据索引获取手柄按键状态  
--- 索引从 1 开始  
--- 只传递一个 keycode 参数时，自动选择可用的手柄获取按键状态
---@param index number
---@param keycode number
---@return boolean
---@overload fun(keycode:number):boolean
function M.getKeyState(index, keycode) end

--- 根据索引获取手柄左扳机状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取左扳机状态
---@param index number
---@return number
---@overload fun():number
function M.getLeftTrigger(index) end

--- 根据索引获取手柄右扳机状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取右扳机状态
---@param index number
---@return number
---@overload fun():number
function M.getRightTrigger(index) end

--- 根据索引获取手柄左摇杆X轴状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取左摇杆X轴状态
---@param index number
---@return number
---@overload fun():number
function M.getLeftThumbX(index) end

--- 根据索引获取手柄左摇杆Y轴状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取左摇杆Y轴状态
---@param index number
---@return number
---@overload fun():number
function M.getLeftThumbY(index) end

--- 根据索引获取手柄右摇杆X轴状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取右摇杆X轴状态
---@param index number
---@return number
---@overload fun():number
function M.getRightThumbX(index) end

--- 根据索引获取手柄右摇杆Y轴状态  
--- 索引从 1 开始  
--- 不传递参数时，自动选择可用的手柄获取右摇杆Y轴状态
---@param index number
---@return number
---@overload fun():number
function M.getRightThumbY(index) end

return M
