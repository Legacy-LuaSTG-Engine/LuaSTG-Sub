local test = require("test")
local imgui = require("imgui")
local FileSystemWatcher = require("lstg.FileSystemWatcher")

local TEST_NAME = "File: FileSystemWatcher"

local function refreshGlyphCache(str)
    ---@diagnostic disable-next-line: undefined-field
    imgui.backend.CacheGlyphFromString(str)
end

---@class test.file.FileSystemWatcher : test.Base
local M = {}

function M:onCreate()
    self.watcher = FileSystemWatcher.create("C:\\Users\\Admin\\Downloads");
    self.changes = {}
end

function M:onDestroy()
    self.watcher:close()
end

function M:onUpdate()
    local change = { file_name = "", action = 0 }
    while self.watcher:read(change) do
        table.insert(self.changes, { file_name = change.file_name, action = change.action })
        refreshGlyphCache(change.file_name)
    end

    ---@diagnostic disable-next-line: undefined-field
    local ImGui = imgui.ImGui
    if ImGui.Begin(TEST_NAME) then
        for i, v in ipairs(self.changes) do
            ImGui.Text(string.format("%d. (%d) %s", i, v.action, v.file_name))
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.file.FileSystemWatcher", M, TEST_NAME)
