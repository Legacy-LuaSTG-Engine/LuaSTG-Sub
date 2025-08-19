local test = require("test")
local imgui = require("imgui")

local TEST_NAME = "File: FileSystemArchive"

---@class test.file.FileSystemArchive : test.Base
local M = {}

function M:onCreate()
    ---@diagnostic disable-next-line: undefined-field
    lstg.LoadPackSub("assets/alpha.zip") -- internal api
    lstg.DoFile("alpha/alpha.lua", "assets/alpha.zip")
end

function M:onDestroy()
end

function M:onUpdate()
end

function M:onRender()
end

test.registerTest("test.file.FileSystemArchive", M, TEST_NAME)
