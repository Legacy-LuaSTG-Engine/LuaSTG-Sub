---=====================================
---luastg 文件管理接口
---lstg.FileManager 库
---作者:Xiliusha
---邮箱:Xiliusha@outlook.com
---=====================================

---[不稳定的实现，建议直接使用LoadPack、UnloadPack等稳定的API]
---@class lstg.FileManager
local m = {}
lstg.FileManager = m

----------------------------------------
---压缩包方法

---[不稳定的实现，日后可能会变更] 压缩包对象
---@class lstg.Archive
local c = {}

---判断压缩包是否还有效，可能压缩包本身已经被卸载但是引用还在
---@return boolean
function c:IsValid()
    return true
end

---判断压缩包是否存在指定的文件
---@param filepath string @文件在压缩包内的路径
---@return boolean
function c:FileExist(filepath)
    return true
end

---列出指定路径内所有的文件和文件夹
---@param searchpath string @搜索路径
---@param extend string|nil @如果该参数存在，则匹配文件拓展名，并且屏蔽文件夹，只保留文件
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function c:EnumFiles(searchpath, extend)
    return {{"sample.ext",false}, {"sampledir/",true}}
end

---列出压缩包内所有的文件和文件夹
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function c:ListFiles()
    return {{"sample.ext",false}, {"sampledir/",true}}
end

---获取压缩包名，请参考lstg.FileManager.LoadArchive第一个参数
---@return string
function c:GetName()
    return "sample.zip"
end

---获取压缩包优先级
---@return number
function c:GetPriority()
    return 0
end

---设置压缩包优先级
---@param priority number
function c:SetPriority(priority)
end

----------------------------------------
---文件管理方法

---加载压缩包
---@param archivefilepath string @压缩包文件路径
---@param priority number|nil @可选参数，默认为0，压缩包优先级，必须为整数
---@param password string|nil @可选参数，默认为空(nil)，压缩包密码
---@return lstg.Archive|nil @如果加载成功，返回一个压缩包对象，否则返回空值
function m.LoadArchive(archivefilepath, priority, password)
end

---卸载压缩包
---@param archivefilepath string @压缩包文件路径，请参考lstg.FileManager.LoadArchive第一个参数
---@return boolean @返回的boolean值为true时，表示卸载该压缩包前，该压缩包确实已加载并卸载该压缩包，否则不存在且不进行任何操作
function m.UnloadArchive(archivefilepath)
    return true
end

---卸载所有已加载的压缩包
---@return nil @该方法没有返回值
function m.UnloadAllArchive()
end

---判断指定压缩包是否已经加载
---@param archivename string @压缩包名，请参考lstg.FileManager.LoadArchive第一个参数，另可参考lstg.Archive:GetName的返回值
---@return boolean
function m.ArchiveExist(archivename)
    return true
end

---枚举已经加载的压缩包
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是压缩包名，第二项为压缩包优先级
function m.EnumArchives()
    return {{"sample.zip", 0}, {"foo.zip", 9961}}
end

---获取已加载的压缩包，返回lstg.Archive对象
---@param archivename string @压缩包名，请参考lstg.FileManager.LoadArchive第一个参数，另可参考lstg.Archive:GetName的返回值
---@return lstg.Archive
function m.GetArchive(archivename)
    ---@type lstg.Archive
    local zip
    return zip
end

---判断指定的文件是否存在，只判断文件系统中的文件
---@param filepath string @文件
---@return boolean
function m.FileExist(filepath)
    return true
end

---判断指定的文件是否存在，除了判断文件系统中的文件外，还判断已加载的压缩包中的文件
---@param filepath string @文件
---@return boolean
function m.FileExistEx(filepath)
    return true
end

---列出指定路径内所有的文件和文件夹，仅搜索文件系统中的，不包括压缩包中的
---@param searchpath string @搜索路径
---@param extend string|nil @如果该参数存在，则匹配文件拓展名，并且屏蔽文件夹，只保留文件
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function m.EnumFiles(searchpath, extend)
    return {{"sample.ext",false}, {"sampledir/",true}}
end

---列出指定路径内所有的文件和文件夹，包括加载的压缩包中的
---@param searchpath string @搜索路径
---@param extend string|nil @如果该参数存在，则匹配文件拓展名，并且屏蔽文件夹，只保留文件
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function m.EnumFilesEx(searchpath, extend)
    return {{"sample.ext",false, nil}, {"sampledir/",true, "sample.zip"}}
end

return m
