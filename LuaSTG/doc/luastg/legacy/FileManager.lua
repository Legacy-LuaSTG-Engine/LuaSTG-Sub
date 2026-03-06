--------------------------------------------------------------------------------
--- LuaSTG Sub 文件管理
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 文件管理

---@class lstg.FileManager
local M = {}

---加载压缩包
---@param archivefilepath string @压缩包文件路径
---@param priority number|nil @可选参数，默认为0，压缩包优先级，必须为整数
---@param password string|nil @可选参数，默认为空(nil)，压缩包密码
---@return lstg.Archive|nil @如果加载成功，返回一个压缩包对象，否则返回空值
function M.LoadArchive(archivefilepath, priority, password)
end

---卸载压缩包
---@param archivefilepath string @压缩包文件路径，请参考lstg.FileManager.LoadArchive第一个参数
---@return boolean @返回的boolean值为true时，表示卸载该压缩包前，该压缩包确实已加载并卸载该压缩包，否则不存在且不进行任何操作
function M.UnloadArchive(archivefilepath)
end

---卸载所有已加载的压缩包
---@return nil @该方法没有返回值
function M.UnloadAllArchive()
end

---判断指定压缩包是否已经加载
---@param archivename string @压缩包名，请参考lstg.FileManager.LoadArchive第一个参数，另可参考lstg.Archive:GetName的返回值
---@return boolean
function M.ArchiveExist(archivename)
end

---枚举已经加载的压缩包
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是压缩包名，第二项为压缩包优先级
function M.EnumArchives()
    return {{"sample.zip", 0}, {"foo.zip", 9961}}
end

---获取已加载的压缩包，返回lstg.Archive对象
---@param archivename string @压缩包名，请参考lstg.FileManager.LoadArchive第一个参数，另可参考lstg.Archive:GetName的返回值
---@return lstg.Archive
function M.GetArchive(archivename)
end

--------------------------------------------------------------------------------
--- 文件判断和文件枚举
--- Check file exists; list files and directories

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.16.0 修改]  
--- 检查文件是否存在  
--- 如果 `also_check_archive` 为 true，该方法还会检查是否有任意一个压缩包存在指定的文件  
--- [LuaSTG Ex Plus Add]  
--- [LuaSTG Sub v0.16.0 Change]  
--- Check if the file exists  
--- If `also_check_archive` is true, the method also checks if any Archive contain the specified file  
---@param filepath string
---@param also_check_archive boolean
---@return boolean
---@overload fun(filepath:string):boolean
function M.FileExist(filepath, also_check_archive)
end

---@deprecated
function M.FileExistEx(filepath)
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.16.0 修改]  
--- 列出指定路径下的文件和文件夹  
--- 如果指定了 `extend` 参数，则忽略文件夹，并匹配检查文件拓展名  
--- 如果 `also_enum_archives` 为 true，该方法还会列出压缩包内的文件  
--- [LuaSTG Ex Plus Add]  
--- [LuaSTG Sub v0.16.0 Change]  
--- List files and directories under search path   
--- If `extend` is true, the method ignore directories and match file extension name  
--- If `also_enum_archives` is true, the method also list files in Archive  
---@param searchpath string
---@param extend string
---@param also_enum_archives boolean
---@overload fun(searchpath:string)
---@overload fun(searchpath:string, extend:string)
---@overload fun(searchpath:string, extend:nil, also_enum_archives:boolean)
function M.EnumFiles(searchpath, extend, also_enum_archives)
    return {
        { "sample.ext"          , false },
        { "sampledir/"          , true  },
        { "sampledir/sample.txt", false },
    }
end

---@deprecated
function M.EnumFilesEx(searchpath, extend)
end

--------------------------------------------------------------------------------
--- 搜索路径管理
--- 引擎在加载 Lua 脚本、图片文件、音频文件等资源的时候会从搜索路径中查找
--- Search path management
--- The engine will find from the search path when loading Lua scripts, texture files, audio files...

--- [LuaSTG Sub v0.10.0 新增]  
--- 添加搜索路径条目  
--- [LuaSTG Sub v0.10.0 Add]  
--- Add a search path  
---@param search_path string
function M.AddSearchPath(search_path)
end

--- [LuaSTG Sub v0.10.0 新增]  
--- 移除搜索路径条目  
--- [LuaSTG Sub v0.10.0 Add]  
--- Remove a search path  
---@param search_path string
function M.RemoveSearchPath(search_path)
end

--- [LuaSTG Sub v0.10.0 新增]  
--- 移除所有搜索路径条目  
--- [LuaSTG Sub v0.10.0 Add]  
--- Remove all search path  
function M.ClearSearchPath()
end

--------------------------------------------------------------------------------
--- 文件夹管理（不影响资源包内的文件夹）
--- Directory management (not affecting directories in archive)

--- [LuaSTG Sub v0.15.5 新增]  
--- 创建文件夹，可以是多级文件夹  
--- [LuaSTG Sub v0.15.5 Add]  
--- Create directories for every element of dirs that does not already exist  
---@param dirs string
---@return boolean
function M.CreateDirectory(dirs)
end

--- [LuaSTG Sub v0.15.5 新增]  
--- 递归删除文件夹，会删除所有子文件夹和包含的所有文件  
--- [LuaSTG Sub v0.15.5 Add]  
--- Deleta a directory recursively, delete all subdirectory and all files contained in  
---@param path string
---@return boolean
function M.RemoveDirectory(path)
end

--- [LuaSTG Sub v0.16.0 新增]  
--- 检查文件夹是否存在  
--- 如果 `also_check_archive` 为 true，该方法还会检查是否有任意一个压缩包存在指定的文件夹  
--- [LuaSTG Sub v0.16.0 Add]  
--- Check if the directory exists  
--- If `also_check_archive` is true, the method also checks if any Archive contain the specified directory  
---@param path string
---@param also_check_archive boolean
---@return boolean
---@overload fun(path:string):boolean
function M.DirectoryExist(path, also_check_archive)
end

return M
