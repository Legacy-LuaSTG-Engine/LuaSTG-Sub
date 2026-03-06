---@diagnostic disable: missing-return

--- Lua CJSON safe (fail silently)

---@class cjson.safe : cjson
local cjson_safe = require("cjson")

--- The cjson.safe module behaves identically to the cjson module, except when errors are encountered during JSON conversion. On error, the cjson_safe.encode and cjson_safe.decode functions will return nil followed by the error message.  
---@see cjson.encode
---@generic T
---@param v T
---@return string
---@overload fun(v:any): nil, string
function cjson_safe.encode(v)
end

--- The cjson.safe module behaves identically to the cjson module, except when errors are encountered during JSON conversion. On error, the cjson_safe.encode and cjson_safe.decode functions will return nil followed by the error message.  
---@see cjson.decode
---@generic T
---@param v string
---@return T
---@overload fun(v:string): nil, string
function cjson_safe.decode(v)
end

return cjson_safe
