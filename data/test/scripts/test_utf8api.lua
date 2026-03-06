local test = require("test")

---@class test.Module.UTF8API : test.Base
local M = {}

function M:onCreate()
    local cjson = require("cjson")
    lstg.Print(cjson.encode({hello="world"}))
    lstg.Print(cjson.decode("{\"hello\": \"world\"}").hello)

    lstg.FileManager.CreateDirectory("测试test")
    lstg.FileManager.CreateDirectory("测试test2\\")
    lstg.FileManager.CreateDirectory("测试test3/")
    lstg.FileManager.CreateDirectory("第一层/第二层/第三层/第四层/第五层/第六层/第七层/第八层/第九层/第十层")
    lstg.FileManager.CreateDirectory("第1层/第2层/第3层/第4层/第5层/第6层/第7层/第8层/第9层/第10层")

    lstg.FileManager.RemoveDirectory("测试test")
    lstg.FileManager.RemoveDirectory("测试test2\\")
    lstg.FileManager.RemoveDirectory("测试test3/")
    lstg.FileManager.RemoveDirectory("第一层")
    lstg.FileManager.RemoveDirectory("第1层/")
    
    for it in lfs.dir(".") do
        lstg.Print(it, lfs.attributes("./" .. it, "mode"))
    end
    do
        local f, e = io.open("你好 io.open，你可终于支持中文辣.txt", "wb")
        assert(f, e)
        f:write("中国智造，惠及全球")
        f:close()
    end
    do
        local f, e = io.open("中文测试测试.txt", "wb")
        assert(f, e)
        f:write("文明和谐")
        f:close()
        os.remove("中文测试测试.txt")
    end
    do
        local f, e = io.open("中文测试测试 Test test 还有空格.txt", "wb")
        assert(f, e)
        f:write("你好世界")
        f:close()
        local f2 = io.open("Test 测试重命名.txt", "rb")
        if f2 then
            f2:close()
            os.remove("Test 测试重命名.txt")
        end
        os.rename("中文测试测试 Test test 还有空格.txt", "Test 测试重命名.txt")
    end
    do
        for _ = 1, 10 do
            local s = os.tmpname()
            lstg.Print(s)
            local f = io.open(s, "wb")
            assert(f)
            f:close()
            os.remove(s)
        end
    end
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.UTF8API", M)
