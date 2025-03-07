local test = require("test")
local Vector2 = require("lstg.Vector2")

---@class test.math.Vector : test.Base
local M = {}

local function abs(x)
    if x >= 0 then
        return x
    else
        return -x
    end
end

local function testCreate()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x = math.random(1000000000)
        local y = math.random(1000000000)
        local v = Vector2.create(x, y);
        assert(v.x == x)
        assert(v.y == y)
    end
end

local function testGetterSetter()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local v = Vector2.create();
        assert(v.x == 0)
        assert(v.y == 0)
        local x = math.random(1000000000)
        local y = math.random(1000000000)
        v.x = x
        v.y = y
        assert(v.x == x)
        assert(v.y == y)
    end
end

local function testEquals()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local v1c = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        assert(v1 == v1c)
        assert(v1 ~= v2)
        assert(v1.x == v1c.x)
        assert(v1.y == v1c.y)
        assert(v1.x ~= v2.x)
        assert(v1.y ~= v2.y)
    end
end

local function testOperatorAdd()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        local v3 = v1 + v2
        assert(v3.x == x1 + x2)
        assert(v3.y == y1 + y2)
    end
end

local function testOperatorSub()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        local v3 = v1 - v2
        assert(v3.x == x1 - x2)
        assert(v3.y == y1 - y2)
    end
end

local function testOperatorMul()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        local v3 = v1 * v2
        assert(v3.x == x1 * x2)
        assert(v3.y == y1 * y2)
    end
end

local function testOperatorDiv()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        local v3 = v1 / v2
        assert(v3.x == x1 / x2)
        assert(v3.y == y1 / y2)
    end
end

local function testOperatorUnm()
    math.randomseed(os.time())
    for _ = 0, 1000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local v2 = -v1;
        assert(v2.x == -x1)
        assert(v2.y == -y1)
    end
end

local function testLengthAndAngle()
    local MAX_ERROR = 0.00001
    math.randomseed(os.time())
    for _ = 0, 100000 do
        local a = math.random() * 1000000000.0
        local r = math.random() * 1000000000.0
        local x = r * math.cos(a)
        local y = r * math.sin(a)
        local la = math.atan2(y, x)
        local v1 = Vector2.create(x, y);
        local a1 = v1:angle()
        local a1d = v1:degreeAngle()
        local r1 = v1:length()
        assert(abs(a1 - la) < MAX_ERROR)
        assert(abs(a1d - math.deg(la)) < MAX_ERROR)
        assert(abs(r - r1) < MAX_ERROR)
    end
end

local function testNormalized()
    local MAX_ERROR = 0.00000001
    math.randomseed(os.time())
    for _ = 0, 100000 do
        local a = math.random(1000000000)
        local r = math.max(MAX_ERROR, math.random(1000000000))
        local x = r * math.cos(a)
        local y = r * math.sin(a)
        local v1 = Vector2.create(x, y);
        local v2 = v1:normalized()
        assert(abs(1.0 - math.sqrt(v2.x * v2.x + v2.y * v2.y)) < MAX_ERROR)
    end
    -- TODO: THIS IS EXCEPTION, throw a lua error
    for _ = 0, 100 do
        local v1 = Vector2.create();
        local v2 = v1:normalized()
        assert(v2.x == 0 and v2.y == 0)
    end
end

local function testDot()
    local MAX_ERROR = 0.00000001
    math.randomseed(os.time())
    for _ = 0, 100000 do
        local x1 = math.random(1000000000)
        local y1 = math.random(1000000000)
        local v1 = Vector2.create(x1, y1);
        local x2 = math.random(1000000000)
        local y2 = math.random(1000000000)
        local v2 = Vector2.create(x2, y2);
        local dot = v1:dot(v2)
        assert(abs(dot - (x1 * x2 + y1 * y2)) < MAX_ERROR)
    end
end

function M:onCreate()
    print("testCreate")
    testCreate()

    print("testGetterSetter")
    testGetterSetter()

    print("testEquals")
    testEquals()
    print("testOperatorAdd")
    testOperatorAdd()
    print("testOperatorSub")
    testOperatorSub()
    print("testOperatorMul")
    testOperatorMul()
    print("testOperatorDiv")
    testOperatorDiv()
    print("testOperatorUnm")
    testOperatorUnm()

    print("testLengthAndAngle")
    testLengthAndAngle()
    print("testNormalized")
    testNormalized()
    print("testDot")
    testDot()
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.graphics.Vector", M, "Math: Vector")
