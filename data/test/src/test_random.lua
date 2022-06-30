local test = require("test")

---@class test.Module.Random : test.Base
local M = {}

function M:onCreate()
    local function test_random()
        local random = require("random")
        local rng = random.xoshiro256ss()
        rng:seed(114514)
        lstg.Print("======", rng:seed())
        for i = 1, 100 do
            lstg.Print(i, rng:integer(1000, 1000000))
        end
        for i = 2, 200 do
            lstg.Print(i, rng:integer(1000000))
        end
        for i = 3, 300 do
            lstg.Print(i, rng:integer())
        end
    
        for i = 1, 100 do
            lstg.Print(i, rng:number(1000, 1000000))
        end
        for i = 2, 200 do
            lstg.Print(i, rng:number(1000000))
        end
        for i = 3, 300 do
            lstg.Print(i, rng:number())
        end
    
        for i = 1, 100 do
            lstg.Print(i, rng:sign())
        end
    end
    test_random()
    test_random()
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.Random", M)
