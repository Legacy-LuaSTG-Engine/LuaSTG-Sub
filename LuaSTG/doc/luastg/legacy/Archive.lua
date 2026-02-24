--------------------------------------------------------------------------------
--- LuaSTG Sub 压缩包
--- 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- 压缩包

---@class lstg.Archive
local Archive = {}

---判断压缩包是否还有效，可能压缩包本身已经被卸载但是引用还在
---@return boolean
function Archive:IsValid()
end

---判断压缩包是否存在指定的文件
---@param filepath string @文件在压缩包内的路径
---@return boolean
function Archive:FileExist(filepath)
end

---列出指定路径内所有的文件和文件夹
---@param searchpath string @搜索路径
---@param extend string|nil @如果该参数存在，则匹配文件拓展名，并且屏蔽文件夹，只保留文件
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function Archive:EnumFiles(searchpath, extend)
    return {{"sample.ext",false}, {"sampledir/",true}}
end

---列出压缩包内所有的文件和文件夹
---@return table<number, table> @返回的表中，每个元素为普通的表，第一项是文件或文件夹的完整路径，第二项为boolean值，为true时代表该项为文件夹
function Archive:ListFiles()
    return {{"sample.ext",false}, {"sampledir/",true}}
end

---获取压缩包名，请参考lstg.FileManager.LoadArchive第一个参数
---@return string
function Archive:GetName()
end

---获取压缩包优先级
---@return number
function Archive:GetPriority()
end

---设置压缩包优先级
---@param priority number
function Archive:SetPriority(priority)
end
