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
    --test_random()
    --test_random()

    local function test_clone1()
        local rnd = lstg.Rand()
        rnd:Seed(114514)

        local rnd2 = rnd:clone()

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

    local function test_serialize()
        local rnd = lstg.Rand()
        rnd:Seed(math.random(0, 65535))

        for _ = 1, 1000 do
            rnd:Int(11, 20)
            rnd:Float(114.514, 1919.810)
        end

        local state = rnd:serialize()
        local rnd2 = lstg.Rand()
        assert(rnd2:deserialize(state))

        for _ = 1, 1000 do
            local v1 = rnd:Int(11, 20)
            local v2 = rnd2:Int(11, 20)
            local f1 = rnd:Float(114.514, 1919.810)
            local f2 = rnd2:Float(114.514, 1919.810)
            assert(v1 == v2)
            assert(f1 == f2)
        end
    end
    test_serialize()

    local function test_random_case(name)
        local random = require("random")
        ---@class random.generator
        local rng = random[name]()
        local seed = math.random(0, 65535)
        rng:seed(seed)

        for _ = 1, 1000 do
            rng:integer(11, 20)
            rng:number(114.514, 1919.810)
        end

        local state = rng:serialize()
        lstg.Print(seed .. " | " .. state)

        ---@class random.generator
        local rng2 = random[name]()
        assert(rng2:deserialize(state))

        for _ = 1, 1000 do
            local v1 = rng:integer(11, 20)
            local v2 = rng2:integer(11, 20)
            local f1 = rng:number(114.514, 1919.810)
            local f2 = rng2:number(114.514, 1919.810)
            assert(v1 == v2)
            assert(f1 == f2)
        end
    end
    test_random_case("splitmix64")
    test_random_case("xoshiro128p")
    test_random_case("xoshiro128pp")
    test_random_case("xoshiro128ss")
    test_random_case("xoroshiro128p")
    test_random_case("xoroshiro128pp")
    test_random_case("xoroshiro128ss")
    test_random_case("xoshiro256p")
    test_random_case("xoshiro256pp")
    test_random_case("xoshiro256ss")
    test_random_case("xoshiro512p")
    test_random_case("xoshiro512pp")
    test_random_case("xoshiro512ss")
    test_random_case("xoroshiro1024s")
    test_random_case("xoroshiro1024pp")
    test_random_case("xoroshiro1024ss")
    test_random_case("pcg32_oneseq")
    test_random_case("pcg32_fast")
    test_random_case("pcg64_oneseq")
    test_random_case("pcg64_fast")
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.Random", M)
