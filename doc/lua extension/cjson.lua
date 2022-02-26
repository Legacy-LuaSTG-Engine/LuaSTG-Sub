---@class cjson
local M = {}
cjson = M

---@class cjson.null
local V = nil
M.null = V

---@return cjson
function M.new()
end

---@param v any
---@return string
function M.encode(v)
end

---@param v string
---@return any
function M.decode(v)
end

---@param b boolean @default to true
---@return boolean
function M.decode_invalid_numbers(b)
end

---@param b boolean @default to false
---@return boolean
function M.encode_invalid_numbers(b)
end

---@param keep boolean @default to true
---@return boolean
function M.encode_keep_buffer(keep)
end

---@param depth number @default to 1000
---@return number
function M.encode_max_depth(depth)
end

---@param depth number @default to 1000
---@return number
function M.decode_max_depth(depth)
end

---@param convert boolean @default to false
---@param ratio number @default to 2
---@param safe number @default to 10
---@return boolean, number, number
function M.encode_sparse_array(convert, ratio, safe)
end

---@param precision number @from 1 to 14, default to 14
---@return number
function M.encode_number_precision(precision)
end

---@class cjson.safe
local S = {}
M.safe = S

---@return cjson.safe
function S.new()
end

---@param v any
---@return string, string
function S.encode(v)
end

---@param v string
---@return any, string
function S.decode(v)
end

return M
