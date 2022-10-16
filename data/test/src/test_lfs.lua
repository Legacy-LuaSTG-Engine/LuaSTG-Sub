local test = require("test")

---@class test.Module.LuaFileSystem : test.Base
local M = {}

function M:onCreate()
    lstg.Print("========== 检查库导入情况 ==========")
    local Glfs = lfs
    assert(Glfs, "全局的 lfs 呢？")
    local lfs = require("lfs")
    assert(Glfs == lfs)
    for k, v in pairs(lfs) do
        lstg.Print(k, tostring(v))
    end
    lstg.Print("========== 枚举文件测试 1 ==========")
    for it in lfs.dir(".") do
        lstg.Print(it, lfs.attributes("./" .. it, "mode"))
    end
    lstg.Print("========== 枚举文件测试 2 ==========")
    for it in lfs.dir("src") do
        lstg.Print(it, lfs.attributes("src/" .. it, "mode"))
    end
    lstg.Print("========== 中文文件夹测试 ==========")
    lstg.Print("创建文件夹")
    lfs.mkdir("你奶奶的文件夹，还是 带 空 格 的高血压 Directory")
    for it in lfs.dir(".") do
        lstg.Print(it, lfs.attributes("./" .. it, "mode"))
    end
    lstg.Print("删除文件夹")
    lfs.rmdir("你奶奶的文件夹，还是 带 空 格 的高血压 Directory")
    for it in lfs.dir(".") do
        lstg.Print(it, lfs.attributes("./" .. it, "mode"))
    end
    lstg.Print("========== 文件对象属性测试 1 ==========")
    lstg.Print("src", lfs.attributes("src", "mode"))
    lstg.Print("launch", lfs.attributes("launch", "mode"))
    lstg.Print("不存在的文件", lfs.attributes("不存在的文件", "mode"))
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.Module.LuaFileSystem", M)
