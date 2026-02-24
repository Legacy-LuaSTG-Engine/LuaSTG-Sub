---@diagnostic disable: missing-return

---@class lfs
local M = {}
lfs = M

---@class lfs.attribute
local attributes = {
	--- on Unix systems, this represents the device that the inode resides on. On Windows systems, represents the drive number of the disk containing the file
	dev = 0,
	--- on Unix systems, this represents the inode number. On Windows systems this has no meaning
	ino = 0,
	--- string representing the associated protection mode (the values could be file, directory, link, socket, named pipe, char device, block device or other)
	mode = "",
	--- number of hard links to the file
	nlink = 0,
	--- user-id of owner (Unix only, always 0 on Windows)
	uid = 0,
	--- group-id of owner (Unix only, always 0 on Windows)
	gid = 0,
	--- on Unix systems, represents the device type, for special file inodes. On Windows systems represents the same as dev
	rdev = 0,
	--- time of last access
	access = 0,
	--- time of last data modification
	modification = 0,
	--- time of last file status change
	change = 0,
	--- file size, in bytes
	size = 0,
	--- file permissions string
	permissions = "",
	--- block allocated for file; (Unix only)
	blocks = 0,
	--- optimal file system I/O blocksize; (Unix only)
	blksize = 0,
}

---@alias lfs.attribute.key '"dev"' | '"ino"' | '"mode"' | '"nlink"' | '"uid"' | '"gid"' | '"rdev"' | '"access"' | '"modification"' | '"change"' | '"size"' | '"permissions"' | '"blocks"' | '"blksize"'

--- Returns a table with the file attributes corresponding to filepath
--- (or nil followed by an error message and a system-dependent error code in case of error).
--- If the second optional argument is given and is a string,
--- then only the value of the named attribute is returned
--- (this use is equivalent to lfs.attributes(filepath)[aname],
--- but the table is not created and only one attribute is retrieved from the O.S.).
--- if a table is passed as the second argument, it is filled with attributes and returned instead of a new table.
--- The attributes are described as follows; attribute mode is a string, all the others are numbers,
--- and the time related attributes use the same time reference of os.time.
---@param path string
---@param attr lfs.attribute.key
---@return nil, string, number
---@overload fun(path:string, attr:lfs.attribute.key):number
---@overload fun(path:string, attr:lfs.attribute.key):string
---@overload fun(path:string, attr:lfs.attribute):nil, string, number
---@overload fun(path:string, attr:lfs.attribute):lfs.attribute
---@overload fun(path:string):nil, string, number
---@overload fun(path:string):lfs.attribute
function M.attributes(path, attr)
end

--- Changes the current working directory to the given path.
---@param path string
---@return boolean, string @succesful, error message (if failed)
---@overload fun(path:string):boolean
function M.chdir(path)
end

--- Returns a string with the current working directory or nil plus an error string.
---@return nil, string @nil, error message
---@overload fun():string
function M.currentdir()
end

--- Lua iterator over the entries of a given directory.
--- Each time the iterator is called with dir_obj it returns a directory entry's name as a string,
--- or nil if there are no more entries.
--- You can also iterate by calling dir_obj:next(),
--- and explicitly close the directory before the iteration finished with dir_obj:close().
--- Raises an error if path is not a directory.
---@param path string
---@return fun(t:userdata):string, userdata
function M.dir(path)
end

--- Creates a new directory. The argument is the name of the new directory.
--- Returns true in case of success or nil, an error message and a system-dependent error code in case of error.
---@param path string
---@return boolean, string, number @succesful, error message (if failed), error code (if failed)
---@overload fun():boolean
function M.mkdir(path)
end

--- Removes an existing directory. The argument is the name of the directory.
--- Returns true in case of success or nil, an error message and a system-dependent error code in case of error.
---@param path string
---@return boolean, string, number @succesful, error message (if failed), error code (if failed)
---@overload fun():boolean
function M.rmdir(path)
end

return M
