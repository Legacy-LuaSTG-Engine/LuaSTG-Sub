--------------------------------------------------------------------------------
--- Random number generator
--- xoshiro / xoroshiro https://prng.di.unimi.it/
--- PCG / jsf / sfc https://www.pcg-random.org/
--- Lua binding write by 璀境石
--------------------------------------------------------------------------------

---@diagnostic disable: missing-return

--------------------------------------------------------------------------------
--- recommended generator

--- 1. pcg64_oneseq
--- 2. pcg64_fast
--- 3. xoshiro256ss

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

--- serialize internal state
---@return string
function C:serialize() end

--- deserialize internal state
---@param state string
---@return boolean
function C:deserialize(state) end

--------------------------------------------------------------------------------
--- random library

---@class random
local M = {}

---------- splitmix family ----------

---@class random.splitmix64 : random.generator
local splitmix64 = {}
function splitmix64:clone() return self end

--- splitmix64  
--- internal state: 64bits  
--- output result: 64bits  
--- it is used for seeding by other random number generators  
--- not recommended for direct use  
---@return random.splitmix64
function M.splitmix64() end

---------- xoshiro128 family ----------

---@class random.xoshiro128p : random.generator
local xoshiro128p = {}
function xoshiro128p:clone() return self end

--- xoshiro128 family: xoshiro128+  
--- internal state: 128bits  
--- output result: 32bits  
--- not recommended  
--- only recommended for generating float value  
---@return random.xoshiro128p
function M.xoshiro128p() end

---@class random.xoshiro128pp : random.generator
local xoshiro128pp = {}
function xoshiro128pp:clone() return self end

--- xoshiro128 family: xoshiro128++  
--- internal state: 128bits  
--- output result: 32bits  
--- not recommended  
---@return random.xoshiro128pp
function M.xoshiro128pp() end

---@class random.xoshiro128ss : random.generator
local xoshiro128ss = {}
function xoshiro128ss:clone() return self end

--- xoshiro128 family: xoshiro128**  
--- internal state: 128bits  
--- output result: 32bits  
--- not recommended  
---@return random.xoshiro128ss
function M.xoshiro128ss() end

---------- xoroshiro128 family ----------

---@class random.xoroshiro128p : random.generator
local xoroshiro128p = {}
function xoroshiro128p:clone() return self end

--- xoroshiro128 family: xoroshiro128+  
--- internal state: 128bits  
--- output result: 64bits  
--- not recommended  
---@return random.xoroshiro128p
function M.xoroshiro128p() end

---@class random.xoroshiro128pp : random.generator
local xoroshiro128pp = {}
function xoroshiro128pp:clone() return self end

--- xoroshiro128 family: xoroshiro128++  
--- internal state: 128bits  
--- output result: 64bits  
---@return random.xoroshiro128pp
function M.xoroshiro128pp() end

---@class random.xoroshiro128ss : random.generator
local xoroshiro128ss = {}
function xoroshiro128ss:clone() return self end

--- xoroshiro128 family: xoroshiro128**  
--- internal state: 128bits  
--- output result: 64bits  
---@return random.xoroshiro128ss
function M.xoroshiro128ss() end

---------- xoshiro256 family ----------

---@class random.xoshiro256p : random.generator
local xoshiro256p = {}
function xoshiro256p:clone() return self end

--- xoshiro256 family: xoshiro256+  
--- internal state: 256bits  
--- output result: 64bits  
--- recommended for generating double value  
---@return random.xoshiro256p
function M.xoshiro256p() end

---@class random.xoshiro256pp : random.generator
local xoshiro256pp = {}
function xoshiro256pp:clone() return self end

--- xoshiro256 family: xoshiro256++  
--- internal state: 256bits  
--- output result: 64bits  
---@return random.xoshiro256pp
function M.xoshiro256pp() end

---@class random.xoshiro256ss : random.generator
local xoshiro256ss = {}
function xoshiro256ss:clone() return self end

--- xoshiro256 family: xoshiro256**  
--- internal state: 256bits  
--- output result: 64bits  
--- recommended for Lua  
---@return random.xoshiro256ss
function M.xoshiro256ss() end

---------- xoshiro512 family ----------

---@class random.xoshiro512p : random.generator
local xoshiro512p = {}
function xoshiro512p:clone() return self end

--- xoshiro512 family: xoshiro512+  
--- internal state: 512bits  
--- output result: 64bits  
---@return random.xoshiro512p
function M.xoshiro512p() end

---@class random.xoshiro512pp : random.generator
local xoshiro512pp = {}
function xoshiro512pp:clone() return self end

--- xoshiro512 family: xoshiro512++  
--- internal state: 512bits  
--- output result: 64bits  
---@return random.xoshiro512pp
function M.xoshiro512pp() end

---@class random.xoshiro512ss : random.generator
local xoshiro512ss = {}
function xoshiro512ss:clone() return self end

--- xoshiro512 family: xoshiro512**  
--- internal state: 512bits  
--- output result: 64bits  
---@return random.xoshiro512ss
function M.xoshiro512ss() end

---------- xoroshiro1024 family ----------

---@class random.xoroshiro1024s : random.generator
local xoroshiro1024s = {}
function xoroshiro1024s:clone() return self end

--- xoroshiro1024 family: xoroshiro1024*  
--- internal state: 1024bits  
--- output result: 64bits  
---@return random.xoroshiro1024s
function M.xoroshiro1024s() end

---@class random.xoroshiro1024pp : random.generator
local xoroshiro1024pp = {}
function xoroshiro1024pp:clone() return self end

--- xoroshiro1024 family: xoroshiro1024++  
--- internal state: 1024bits  
--- output result: 64bits  
---@return random.xoroshiro1024pp
function M.xoroshiro1024pp() end

---@class random.xoroshiro1024ss : random.generator
local xoroshiro1024ss = {}
function xoroshiro1024ss:clone() return self end

--- xoroshiro1024 family: xoroshiro1024**  
--- internal state: 1024bits  
--- output result: 64bits  
---@return random.xoroshiro1024ss
function M.xoroshiro1024ss() end

---------- pcg family ----------

---@class random.pcg32_oneseq : random.generator
local pcg32_oneseq = {}
function pcg32_oneseq:clone() return self end

--- pcg family: pcg32_oneseq  
--- internal state: 64bits  
--- output result: 32bits  
--- period: 2^64  
---@return random.pcg32_oneseq
function M.pcg32_oneseq() end

---@class random.pcg32_fast : random.generator
local pcg32_fast = {}
function pcg32_fast:clone() return self end

--- pcg family: pcg32_fast  
--- internal state: 64bits  
--- output result: 32bits  
--- period: 2^62  
---@return random.pcg32_fast
function M.pcg32_fast() end

---@class random.pcg64_oneseq : random.generator
local pcg64_oneseq = {}
function pcg64_oneseq:clone() return self end

--- pcg family: pcg64_oneseq  
--- internal state: 128bits  
--- output result: 64bits  
--- period: 2^128  
---@return random.pcg64_oneseq
function M.pcg64_oneseq() end

---@class random.pcg64_fast : random.generator
local pcg64_fast = {}
function pcg64_fast:clone() return self end

--- pcg family: pcg64_fast  
--- internal state: 128bits  
--- output result: 64bits  
--- period: 2^126  
---@return random.pcg64_fast
function M.pcg64_fast() end

---------- sfc family ----------

-----@class random.sfc32 : random.generator

--- sfc family: sfc32  
--- internal state: 128bits  
--- output result: 32bits  
-----@return random.sfc32
--function M.sfc32() end

-----@class random.sfc64 : random.generator

--- sfc family: sfc64  
--- internal state: 256bits  
--- output result: 64bits  
-----@return random.sfc64
--function M.sfc64() end

---------- jsf family ----------

-----@class random.jsf32 : random.generator

--- jsf family: jsf32  
--- internal state: 128bits  
--- output result: 32bits  
-----@return random.jsf32
--function M.jsf32() end

-----@class random.jsf64 : random.generator

--- jsf family: jsf64  
--- internal state: 256bits  
--- output result: 64bits  
-----@return random.jsf64
--function M.jsf64() end

return M
