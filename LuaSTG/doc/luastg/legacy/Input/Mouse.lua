--------------------------------------------------------------------------------
--- LuaSTG Sub 鼠标输入
--- LuaSTG Sub mouse input
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

--- [LuaSTG Sub v0.15.6 新增]  
---@class lstg.Input.Mouse
local M = {}

--------------------------------------------------------------------------------
--- 按键码常量
--- 注意：这些常量的值和内部实现有关，请勿直接使用数字值，应该通过常量访问
--- Key code constants
--- Note: The values of these constants are related to the internal implementation,
--- DO NOT use numeric values directly, they should be accessed through constants

M.None = 0
M.Primary = 1
M.Left = 1
M.Middle = 4
M.Secondary = 2
M.Right = 2
M.X1 = 5
M.XButton1 = 5
M.X2 = 6
M.XButton2 = 6

--------------------------------------------------------------------------------
--- 方法
--- Methods

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取鼠标按键状态  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get mouse key state  
---@param code number
---@return boolean
function M.GetKeyState(code)
end

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取鼠标指针位置  
--- 默认情况下，以窗口左下角为原点，x 轴向右，y 轴向上  
--- 当参数 no_y_flip 为 true 时，不翻转 Y 轴  
--- 此时以窗口左上角为原点，x 轴向右，y 轴向下  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get mouse position  
--- By default, bottom-left corner of the window is taken as the origin,  
--- the x-axis is to the right, and the y-axis is up  
--- When the parameter no_y_flip is true, the Y axis is not flipped  
--- At this point, top-left corner of the window is taken as the origin,  
--- the x-axis is to the right, and the y-axis is down  
---@param no_Y_flip boolean
---@return number, number
---@overload fun():number, number
function M.GetPosition(no_Y_flip)
end

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取鼠标滚轮增量，已经除以 120.0  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get the mouse wheel increment, already divided by 120.0  
---@return number
function M.GetWheelDelta()
end

return M
