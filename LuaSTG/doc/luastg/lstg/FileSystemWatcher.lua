---@diagnostic disable: missing-return, unused-local

---@class lstg.FileSystemWatcher.FileNotifyInformation
---@field file_name string
---@field action lstg.FileSystemWatcher.FileAction

---@class lstg.FileSystemWatcher
local FileSystemWatcher = {}

---@param info lstg.FileSystemWatcher.FileNotifyInformation
---@return boolean
function FileSystemWatcher:read(info)
end

function FileSystemWatcher:close()
end

---@param path string
---@return lstg.FileSystemWatcher
function FileSystemWatcher.create(path)
end

return FileSystemWatcher
