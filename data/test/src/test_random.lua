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

    local function test_clone1()
        local rnd = lstg.Rand()
        rnd:Seed(114514)

        local rnd2 = rnd:Clone()

        for _ = 1, 1000 do
            local v1 = rnd:Int(11, 20)
            local v2 = rnd2:Int(11, 20)
            assert(v1 == v2)
        end
    end
    local function test_clone2()
        local random = require("random")
        local rnd = random.pcg32_fast()
        rnd:seed(114514)

        for _ = 1, 1000 do
            rnd:integer(11, 20)
            rnd:number(114.514, 1919.810)
        end

        local rnd2 = rnd:clone()

        for _ = 1, 1000 do
            local v1 = rnd:integer(11, 20)
            local v2 = rnd2:integer(11, 20)
            assert(v1 == v2)
        end
    end
    test_clone1()
    test_clone2()
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.Random", M)
