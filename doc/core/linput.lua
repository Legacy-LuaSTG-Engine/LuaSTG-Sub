--------------------------------------------------------------------------------
--- LuaSTG Sub 用户输入
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 移植指南

-- lstg.GetLastChar():string
-- 已经废弃，正在考虑如何更好地支持文本输入（包括输入法的输入）

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
-- 该联机相关的 API 已经被 Xiliusha 移除，源代码也已经丢失
-- 如果有需要请使用旧版本 LuaSTG Ex Plus

--------------------------------------------------------------------------------
--- 输入获取

--- 按键码建议查看微软的 VKEY 文档
---@param vkey number
---@return boolean
function lstg.GetKeyState(vkey)
end

---@return number @vkey
function lstg.GetLastKey()
end

--- 以窗口左下角为原点，x 轴向右，y 轴向上
---@return number, number
function lstg.GetMousePosition()
end

--- 0、1、2 分别代表鼠标左、中、右键
---@param index number
---@return boolean
function lstg.GetMouseState(index)
end

--- [LuaSTG Ex Plus 新增]
--- 注意：由于 fancy2d 设计上的原因，该方法每帧需要且仅需调用一次
---@return number
function lstg.GetMouseWheelDelta()
end

--------------------------------------------------------------------------------
--- 平台拓展（即将移除）

--- [LuaSTG Ex Plus 新增]  
--- 注意：该 API 未来可能被移除，并作为平台拓展提供  
--- 按键码建议查看微软的 DirectInput 文档  
---@param dinput_san_code number
---@return boolean
function lstg.GetKeyboardState(dinput_san_code)
end

--- [LuaSTG Ex Plus 新增]  
--- 注意：该 API 未来可能被移除，并作为平台拓展提供  
--- 按键码建议查看微软的 VKEY 文档  
---@param vkey number
---@return boolean
function lstg.GetAsyncKeyState(vkey)
end
