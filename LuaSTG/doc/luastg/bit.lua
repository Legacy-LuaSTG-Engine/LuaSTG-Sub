--- LuaJIT built-in library: bit

---@diagnostic disable: missing-return

---@class bit
local M = {}
bit = M

--- Normalizes a number to the numeric range (32 bit interger) for bit operations and returns it.
---@param x number
---@return number
function M.tobit(x)
end

--- Converts its first argument to a hex string.
--- The number of hex digits is given by the absolute value of the optional second argument.
--- Positive numbers between 1 and 8 generate lowercase hex digits.
--- Negative numbers generate uppercase hex digits.
---@param x number
---@param n number @default to 8
---@return string
---@overload fun(x:number):string
function M.tohex(x, n)
end

--- bitwise not
---@param x number
---@return number
function M.bnot(x)
end

--- bitwise and
---@param x1 number
---@param x2 number
---@vararg number
---@return number
function M.band(x1, x2, ...)
end

--- bitwise or
---@param x1 number
---@param x2 number
---@vararg number
---@return number
function M.bor(x1, x2, ...)
end

--- bitwise xor
---@param x1 number
---@param x2 number
---@vararg number
---@return number
function M.bxor(x1, x2, ...)
end

--- bitwise logical left-shift
---@param x number
---@param n number
---@return number
function M.lshift(x, n)
end

--- bitwise logical right-shift
---@param x number
---@param n number
---@return number
function M.rshift(x, n)
end

--- bitwise arithmetic right-shift
---@param x number
---@param n number
---@return number
function M.arshift(x, n)
end

--- bitwise left rotation
---@param x number
---@param n number
---@return number
function M.rol(x, n)
end

--- bitwise right rotation
---@param x number
---@param n number
---@return number
function M.ror(x, n)
end

--- Swaps the bytes of its argument and returns it.
--- This can be used to convert little-endian 32 bit numbers to big-endian 32 bit numbers or vice versa.
---@param x number
---@return number
function M.bswap(x)
end

return M
