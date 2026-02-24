--------------------------------------------------------------------------------
--- LuaSTG Sub 程序框架方法
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- 迁移指南

-- 关于 lstg.ShowSplashWindow：
-- 已经 Xiliusha 连根拔掉，目前只留有一个空方法

-- 以下内容已经过时：  
-- 
-- 关于显示模式和刷新率：
-- 屏幕的刷新率为以分数储存的有理数，由分子、分母两个部分组成
-- 如果想正确进入独占全屏，最好通过 lstg.EnumResolutions 枚举支持的显示模式
-- 然后传递给 lstg.ChangeVideoMode
-- 窗口化则不需要传递刷新率参数
-- 
-- 随着画布模式的引入，全屏的内部实现已经交给引擎自动管理

--------------------------------------------------------------------------------
--- 版本信息
--- Version Informations

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取引擎版本号，分别代表 major、minor、patch  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get the engine version numbers, representing major, minor, patch  
---@return number, number, number
function M.GetVersionNumber()
end

--- [LuaSTG Sub v0.15.6 新增]  
--- 获取引擎版本友好名称，例如：LuaSTG Sub v0.10.0-beta  
--- [LuaSTG Sub v0.15.6 Add]  
--- Get the engine version friendly name, for example: LuaSTG Sub v0.10.0-beta  
---@return string
function M.GetVersionName()
end

--------------------------------------------------------------------------------
--- 输出日志

--- 输出一条日志
--- 1, 2, 3, 4, 5, 分别代表 debug, info, warning, error, fatal 共5个级别
---@param level number
---@param msg string
function M.Log(level, msg)
end

--- 输出一条信息，以 info 级别输出日志
---@param msg string
function M.SystemLog(msg)
end

--- 相当于 print 到引擎日志文件，以 info 级别输出日志
---@vararg string
function M.Print(...)
end

--------------------------------------------------------------------------------
--- 初始化方法

--- 初始化方法，仅在 launch 文件内生效，运行时调用该方法将会触发警告消息
--- 设置游戏是否窗口化显示
--- 默认设置为 true，即窗口化显示
---@param windowed boolean
function M.SetWindowed(windowed)
end

--- 初始化方法，仅在 launch 文件内生效，运行时调用该方法将会触发警告消息
--- 设置游戏是否启用垂直同步
--- 默认设置为 true，即开启垂直同步
---@param Vsync boolean
function M.SetVsync(Vsync)
end

--- [LuaSTG Sub v0.11.0 更改]  
--- 初始化方法，仅在 launch 文件内生效，运行时调用该方法将会触发警告消息  
--- 设置游戏窗口分辨率  
--- 默认设置为 640x480  
---@param width number
---@param height number
function M.SetResolution(width, height)
end

--- [LuaSTG Sub v0.11.0 新增]
--- [LuaSTG Sub v0.12.0 更改] 新增 dGPU_trick 参数
--- [LuaSTG Sub v0.19.5 更改] 移除 dGPU_trick 参数
--- 初始化方法，仅在 launch 文件内生效，运行时调用该方法将会触发警告消息
--- 设置引擎创建渲染设备时使用的显卡
---@param gpu string
function M.SetPreferenceGPU(gpu)
end

--------------------------------------------------------------------------------
--- 窗口与显示

--- 设置帧率限制，运行时将不会超过设置的帧率
--- 默认设置为 60，程序会以 60FPS 的帧率运行
---@param FPSlimit number
function M.SetFPS(FPSlimit)
end

--- 获取当前帧率
---@return number
function M.GetFPS()
end

--- 显示、隐藏鼠标指针
---@param show boolean
function M.SetSplash(show)
end

--- 更改窗口名称
---@param windowtitle string
function M.SetTitle(windowtitle)
end

--- lstg.EnumResolutions
--- 此 API 经历了多次变动，但是随着画布模式的推出，很多行为已经成为历史  
--- 
--- [LuaSTG Sub 更改]
--- 枚举支持的显示模式
--- 返回值为以 { width:number, height:number, refresh_rate_numerator:number, refresh_rate_denominator:number } 组成的数组
--- 比如 { { 640, 480, 60, 1 }, { 800, 600, 60, 1 } }
---   * lstg.EnumResolutions():number[][]

--- [LuaSTG Sub v0.19.100 废弃]
--- 注：该 API 的行为已发生巨大的变动，更多内容请查看上方的变更历史  
--- 由于引擎内部实现发生巨大变化，不再能枚举到显示模式  
--- 出于兼容性考虑，目前会固定返回一组分辨率  
---@deprecated
---@return number[][]
function M.EnumResolutions()
end

--- [LuaSTG Sub v0.11.0 新增]
--- 枚举可用的显卡，禁止在launch脚本中调用
---@return string[]
function M.EnumGPUs()
    return { "Intel XXXX", "NVIDIA YYYY", "AMD ZZZZ" }
end

--- [LuaSTG Sub v0.19.5 新增]
--- 运行时切换使用的显卡  
--- 警告：如果在游戏内设置界面提供切换显卡的功能，请务必提醒用户可能会耗费很长时间！  
---@param gpu string
function M.ChangeGPU(gpu)
end

--- [LuaSTG Sub 修改]  
--- 指定画面显示方式：画布分辨率（渲染分辨率）、全屏、垂直同步  
--- 渲染分辨率可以不等于窗口分辨率，窗口大小可以自由调整，引擎会自动缩放显示画面  
--- 全屏模式不再固定采用传统独占全屏，而是自动从多种全屏方式中选择：  
---   * Windows 10/11 优先使用桌面合成引擎，不支持时尝试进入传统独占全屏，若失败则使用全屏无边框窗口  
---   * Windows 7/8/8.1 先尝试进入传统独占全屏，若失败则使用全屏无边框窗口  
---@param width number
---@param height number
---@param windowed boolean
---@param vsync boolean
---@return boolean
function M.ChangeVideoMode(width, height, windowed, vsync)
end

--------------------------------------------------------------------------------
--- 资源管理

--- [LuaSTG Sub v0.20.14 修改]  
--- 加载 lua 源文件，并返回源文件中返回的值  
---@param path string
---@param archivefile string
---@overload fun(scriptfilepath:string)
function M.DoFile(path, archivefile)
end

--- 从指定文件读取所有内容
--- 可读取任意文件，但是内容可能并非是纯文本
---@param path string
---@param archivepath string?
---@return string
function M.LoadTextFile(path, archivepath)
end

--- 加载压缩包
---@param path string
---@param password string
---@overload fun(path:string)
function M.LoadPack(path, password)
end

--- 卸载压缩包，参数为加载该压缩包时填写的路径
---@param path string
function M.UnloadPack(path)
end

---将指定文件（可以是压缩包内的文件）复制到指定路径
---@param src string @源文件路径
---@param dest string @目标文件路径
function M.ExtractRes(src, dest)
end

--- 枚举指定目录下的文件，返回类似以下结构的table，第二项不为空时表示该文件在压缩包内
--- {
---     { "sample.file", nil },          -- 在文件系统中
---     { "dir/sample.file", nil },      -- 在文件系统中
---     { "sample.file", "sample.zip" }, -- 在压缩包内
--- }
---@param searchpath string
---@param extendname string
---@param packname string
---@return table[]
---@overload fun(searchpath:string)
---@overload fun(searchpath:string, extendname:string)
function M.FindFiles(searchpath, extendname, packname)
end

return M
