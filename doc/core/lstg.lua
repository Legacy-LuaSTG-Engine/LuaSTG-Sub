--------------------------------------------------------------------------------
--- LuaSTG Sub
--- 璀境石
--------------------------------------------------------------------------------

---@class lstg
local M = {}
lstg = M

--------------------------------------------------------------------------------
--- 命令行参数  
--- Command line arguments  

-- 可用的外部命令行参数  
-- 1、--log-window      启用引擎日志窗口  
-- 2、--log-window-wait 关闭程序后不立即关闭引擎日志窗口  
-- Available external command line arguments  
-- 1、--log-window      Enable engine log window  
-- 2、--log-window-wait Do not close the engine log window immediately after closing the program  

--- 命令行参数  
--- Command line arguments  
---@type string[]
M.args = {}

--------------------------------------------------------------------------------
--- 游戏循环流程

-- 1. 初始化游戏框架，启动 Lua 虚拟机
-- 2. 加载 launch 初始化脚本（可选）
-- 3. 加载游戏引擎
-- 4. 按照 core.lua -> main.lua -> src/main.lua 的顺序搜索入口点文件脚本并加载
--    只会加载第一个找到的脚本，比如同时存在 main.lua、src/main.lua，只加载 main.lua
-- 5. 执行 GameInit，开始游戏循环
-- 6. 按照 FrameFunc -> RenderFunc -> FrameFunc -> ... 的顺序进行游戏循环
-- 7. 结束游戏循环，执行 GameExit
-- 8. 卸载所有资源，关闭游戏引擎，关闭 Lua 虚拟机，关闭游戏框架

--------------------------------------------------------------------------------
--- 全局回调函数（定义在全局，供引擎定期调用）

---游戏循环开始前调用一次
function GameInit()
end

---游戏循环中每帧调用一次，在RenderFunc之前
---@return boolean @返回true时结束游戏循环
function FrameFunc()
	return false
end

---游戏循环中每帧调用一次，在FrameFunc之后
function RenderFunc()
end

---游戏循环结束后，退出前调用一次
function GameExit()
end

---窗口失去焦点的时候被调用
function FocusLoseFunc()
end

---窗口获得焦点的时候被调用
function FocusGainFunc()
end

return M
