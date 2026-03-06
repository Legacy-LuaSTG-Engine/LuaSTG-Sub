---@enum lstg.FileSystemWatcher.FileAction
local FileAction = {
    added = 1,
    removed = 2,
    modified = 3,
    renamed_old_name = 4,
    renamed_new_name = 5,
}
return FileAction
