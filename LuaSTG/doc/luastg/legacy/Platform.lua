--------------------------------------------------------------------------------
--- LuaSTG Sub 平台拓展
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

local M = {}

--------------------------------------------------------------------------------
--- Win32 API

--- 警告：该功能慎用，可能会被杀毒软件误杀
--- 注意：该 API 未来可能会被移除，并作为 Windows 平台拓展提供
--- wait 和 show 如果不填写，默认为 true
---@param path string
---@param arg string
---@param workingdir string
---@param wait boolean
---@param show boolean
---@return boolean
---@overload fun(path:string):boolean
---@overload fun(path:string, arg :string):boolean
---@overload fun(path:string, arg :string, workingdir:string):boolean
---@overload fun(path:string, arg :string, workingdir:string, wait:boolean):boolean
function M.Execute(path, arg, workingdir, wait, show)
end

--- 注意：该 API 未来可能会被移除，并作为 Windows 平台拓展提供
---@param title string
---@param text string
---@param flags number
---@return number
function M.MessageBox(title, text, flags)
end

--- [LuaSTG Sub v0.21.114 新增 | Add]  
--- 
--- 重启引擎，当前的引擎实例会退出，并启动一个新的实例  
--- Restart the engine, will terminate the current engine instance and start a new one.  
--- 
--- 示例代码 | Example  
--- ```lua
--- -- 无额外参数 | no args
--- lstg.RestartWithCommandLineArguments()
--- -- 采用相同的参数 | same args
--- lstg.RestartWithCommandLineArguments(lstg.args)
--- -- 额外的参数 | additional args
--- local args = {}
--- for _, v in ipairs(lstg.args) do
---     table.insert(args, v)
--- end
--- table.insert(args, "--hello-world")
--- lstg.RestartWithCommandLineArguments(args)
--- ```
--- 
--- 来自引擎维护者的提醒：  
--- 
--- 该 API 主要用于某些遗留的项目，这些项目在应用程序内没有提供设置菜单，
--- 显示模式、音量等设置只会通过 `launch` 脚本初始化一次。
--- 因此这些项目会先进入“启动器”，等待用户修改好设置并确认开始后，
--- 再用命令行参数 `start_game=true`（用于跳过“启动器”）启动一个新的应用程序实例。  
--- 
--- 如果您的项目正是这么做的，应该修改代码，在用户确认开始后，
--- 通过 `lstg.ChangeVideoMode` 等 API 更新显示模式、音量等设置。  
--- 
--- Reminder from the engine maintainers:  
--- 
--- This API is primarily intended for certain legacy projects that
--- do not provide a settings menu within the application.
--- Display mode, volume, and similar settings are only initialized once
--- through the launch script.
--- Therefore, these projects first enter the "launcher",
--- wait for the user to adjust settings and confirm to start,
--- then launch a new application instance with the command-line argument
--- `start_game=true` (used to skip the "launcher").  
--- 
--- If your project is doing exactly this,
--- you should modify the code to update settings such as display mode and volume
--- using APIs like `lstg.ChangeVideoMode` after the user confirms to start.
---@param args string[]
---@overload fun()
function M.RestartWithCommandLineArguments(args)
end

return M
