--------------------------------------------------------------------------------
--- Random number generator based on xoshiro / xoroshiro
--- Original C code written in 2019 by David Blackman and Sebastiano Vigna
--- (vigna@acm.org)
--- Reference https://prng.di.unimi.it/
--- Also reference https://www.pcg-random.org/
--- Lua binding write by 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- sample code

--[[

local random = require("random")
local rng = random.xoshiro256p()

rng:seed(0xFF14)
print(rng:seed())

for _ = 1, 10 do
    print(rng:integer())
end
for _ = 1, 10 do
    print(rng:integer(100))
end
for _ = 1, 10 do
    print(rng:integer(10, 100))
end

for _ = 1, 10 do
    print(rng:number())
end
for _ = 1, 10 do
    print(rng:number(1000.0))
end
for _ = 1, 10 do
    print(rng:integer(100.0, 1000.0))
end

for _ = 1, 10 do
    print(rng:sign)
end

--]]

--------------------------------------------------------------------------------
--- generator concept

---@class random.generator
local C = {}

--- set or get seed  
---@param value number @integer value
---@overload fun(self:random.generator):number
function C:seed(value) end

--- generate integer value  
--- rng:integer() will generate integer value in range [0, max lua_Integer]  
--- rng:integer(b) will generate integer value in range [0, b]  
--- rng:integer(a, b) will generate integer value in range [a, b]  
---@param min number @integer value
---@param max number @integer value
---@return number @integer value
---@overload fun(self:random.generator):number
---@overload fun(self:random.generator, max:number):number
function C:integer(min, max) end

--- generate float value  
--- rng:number() will generate float value in range [0.0, 1.0]  
--- rng:number(b) will generate float value in range [0.0, b]  
--- rng:number(a, b) will generate float value in range [a, b]  
---@param min number
---@param max number
---@return number
---@overload fun(self:random.generator):number
---@overload fun(self:random.generator, max:number):number
function C:number(min, max) end

--- generate integer value -1 or 1  
---@return number @integer value
function C:sign() end

--------------------------------------------------------------------------------
--- random library

---@class random
local M = {}

---------- seeding ----------

---@class random.splitmix64 : random.generator

--- splitmix64  
--- it is used for seeding by other random number generators  
--- not recommended for direct use  
---@return random.splitmix64
function M.splitmix64() end

---------- xoroshiro128 family ----------

---@class random.xoroshiro128p : random.generator

--- xoroshiro128 family: xoroshiro128+  
--- not recommended  
---@return random.xoroshiro128p
function M.xoroshiro128p() end

---@class random.xoroshiro128pp : random.generator

--- xoroshiro128 family: xoroshiro128++  
---@return random.xoroshiro128pp
function M.xoroshiro128pp() end

---@class random.xoroshiro128ss : random.generator

--- xoroshiro128 family: xoroshiro128**  
---@return random.xoroshiro128ss
function M.xoroshiro128ss() end

---------- xoshiro256 family ----------

---@class random.xoshiro256p : random.generator

--- xoshiro256 family: xoshiro256+  
--- recommended for generating double value 
---@return random.xoshiro256p
function M.xoshiro256p() end

---@class random.xoshiro256pp : random.generator

--- xoshiro256 family: xoshiro256++  
---@return random.xoshiro256pp
function M.xoshiro256pp() end

---@class random.xoshiro256ss : random.generator

--- xoshiro256 family: xoshiro256**  
--- recommended for Lua 
---@return random.xoshiro256ss
function M.xoshiro256ss() end

---------- xoshiro512 family ----------

---@class random.xoshiro512p : random.generator

--- xoshiro512 family: xoshiro512+  
---@return random.xoshiro512p
function M.xoshiro512p() end

---@class random.xoshiro512pp : random.generator

--- xoshiro512 family: xoshiro512++  
---@return random.xoshiro512pp
function M.xoshiro512pp() end

---@class random.xoshiro512ss : random.generator

--- xoshiro512 family: xoshiro512**  
---@return random.xoshiro512ss
function M.xoshiro512ss() end

---------- xoroshiro1024 family ----------

---@class random.xoroshiro1024s : random.generator

--- xoroshiro1024 family: xoroshiro1024*  
---@return random.xoroshiro1024s
function M.xoroshiro1024s() end

---@class random.xoroshiro1024pp : random.generator

--- xoroshiro1024 family: xoroshiro1024++  
---@return random.xoroshiro1024pp
function M.xoroshiro1024pp() end

---@class random.xoroshiro1024ss : random.generator

--- xoroshiro1024 family: xoroshiro1024**  
---@return random.xoroshiro1024ss
function M.xoroshiro1024ss() end

return M
