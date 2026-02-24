--------------------------------------------------------------------------------
--- LuaSTG Sub 用户输入
--- LuaSTG Sub user input
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 移植指南
--- porting guide

-- 从 LuaSTG Sub v0.15.2 开始，键盘和鼠标输入不再通过 DirectInput 获取，而是通过窗口消息
-- 原因是微软已将 DirectInput 的键盘和鼠标输入标记为弃用
-- Since LuaSTG Sub v0.15.2, keyboard and mouse input is no longer
-- obtained through DirectInput, but through window messages
-- The reason is that Microsoft has marked DirectInput's
-- keyboard and mouse input as deprecated

-- lstg.GetLastChar():string
-- 已经废弃，正在考虑如何更好地支持文本输入（包括输入法的输入）
-- Deprecated, TODO: how to better support text input (including IME)

-- lstg.GetKeyboardState(dinput_san_code:number):boolean
-- 该平台拓展 API 已经废弃
-- This platform extension API has been deprecated and removed

-- lstg.GetAsyncKeyState(vkey:number):boolean
-- 该平台拓展 API 已经废弃
-- This platform extension API has been deprecated and removed

-- lstg.CreateInputDevice
-- lstg.ReleaseInputDevice
-- lstg.AddInputAlias
-- lstg.ClearInputAlias
-- lstg.ResetInput
-- lstg.BindInput
-- lstg.GetVKeyStateEx
-- lstg.ConnectTo
-- lstg.ReceiveData
-- lstg.SendData
-- 联机相关的 API 已经被移除
-- 如果有需要请使用旧版本 LuaSTG Ex Plus
-- The connection-related API has been removed
-- If necessary, please use LuaSTG Ex Plus

--------------------------------------------------------------------------------
--- 兼容性 API
--- Compatibility API

--- 获取键盘按键状态  
--- Get keyboard key state  
---@param vkey number https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
---@return boolean
function M.GetKeyState(vkey)
end

--- 以窗口左下角为原点，x 轴向右，y 轴向上  
--- Bottom-left of the window as the origin, the x-axis is to the right, and the y-axis is up  
---@return number, number
function M.GetMousePosition()
end

--- 0、1、2 分别代表鼠标左、中、右键  
--- 3、4 分别代表鼠标 X1、X2 功能键  
--- 0, 1, 2 Represents the left, middle, and right mouse buttons  
--- 3, 4 Represents the X1, X2 mouse buttons  
---@param index number
---@return boolean
function M.GetMouseState(index)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.15.2 更改]  
--- 获取鼠标滚轮增量  
--- [LuaSTG Ex Plus v?.?.? Add]  
--- [LuaSTG Sub v0.15.2 Changed]  
--- Get the mouse wheel increment
---@return number
function M.GetMouseWheelDelta()
end

---@deprecated
---@return number
function M.GetLastKey()
end

--------------------------------------------------------------------------------
--- 键盘与鼠标
--- Keyboard and mouse

---@class lstg.Input
local Input = {}

Input.Keyboard = require("legacy.Input.Keyboard")
Input.Mouse = require("legacy.Input.Mouse")

M.Input = Input

return M
