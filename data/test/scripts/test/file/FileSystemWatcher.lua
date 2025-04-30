local test = require("test")
local imgui = require("imgui")
local FileSystemWatcher = require("lstg.FileSystemWatcher")
local FileAction = require("lstg.FileSystemWatcher.FileAction")

local TEST_NAME = "File: FileSystemWatcher"

local function refreshGlyphCache(str)
    ---@diagnostic disable-next-line: undefined-field
    imgui.backend.CacheGlyphFromString(str)
end

---@class test.file.FileSystemWatcher : test.Base
local M = {}

function M:onCreate()
    refreshGlyphCache("添加")
    refreshGlyphCache("移除")
    refreshGlyphCache("修改")
    refreshGlyphCache("重命名（旧）")
    refreshGlyphCache("重命名（新）")
    refreshGlyphCache("未知")
    self.watcher = FileSystemWatcher.create("C:\\Users\\Admin\\Downloads");
    self.changes = {}
end

function M:onDestroy()
    self.watcher:close()
end

local function translateFileAction(action)
    if FileAction.added == action then return "添加" end
    if FileAction.removed == action then return "移除" end
    if FileAction.modified == action then return "修改" end
    if FileAction.renamed_old_name == action then return "重命名（旧）" end
    if FileAction.renamed_new_name == action then return "重命名（新）" end
    return "未知"
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
            ImGui.Text(string.format("%d. %s %s", i, translateFileAction(v.action), v.file_name))
        end
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.file.FileSystemWatcher", M, TEST_NAME)
