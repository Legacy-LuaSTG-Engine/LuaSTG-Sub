local test = require("test")
local imgui = require("imgui")

local TEST_NAME = "File: FileSystem"

---@class test.file.FileSystem : test.Base
local M = {}

function M:onCreate()
    for k, v in pairs(imgui.ImGuiTreeNodeFlags) do
        print(k, v)
    end
end

function M:onDestroy()
end

function M:onUpdate()
    local ImGui = imgui.ImGui
    local ImGuiTreeNodeFlags = imgui.ImGuiTreeNodeFlags

    local file_flags = ImGuiTreeNodeFlags.Leaf + ImGuiTreeNodeFlags.NoTreePushOnOpen

    ---@param path string
    local function list(path)
        local files = lstg.FileManager.EnumFiles(path)
        for i, f in ipairs(files) do
            local name = f[1]:sub(path:len() + 1)
            if name:sub(name:len()) == "/" then
                name = name:sub(1, name:len() - 1)
            end
            local label = name .. "##" .. f[1]
            if f[2] then
                if ImGui.TreeNodeEx(label) then
                    list(f[1])
                    ImGui.TreePop()
                end
            else
                ImGui.TreeNodeEx(label, file_flags)
            end
        end
    end

    if ImGui.Begin(TEST_NAME) then
        list("")
    end
    ImGui.End()
end

function M:onRender()
end

test.registerTest("test.file.FileSystem", M, TEST_NAME)
