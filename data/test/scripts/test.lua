local imgui_exist, imgui = pcall(require, "imgui")
local Blank = require("test.Blank")

---@class test
local M = {}

---@class test.TestRecord
local TestRecord = {
    id = "test.Blank",
    type = Blank,
    name = "Blank",
    path = "test/Blank.lua",
}

---@type test.TestRecord[]
local tests = {
    TestRecord,
}

---@type test.Base
local current_test = Blank

---@param id string
---@param type test.Base
---@param name string?
function M.registerTest(id, type, name)
    for _, v in ipairs(tests) do
        if v.id == id then
            lstg.Log(2, ("test '%s' replace with newer one"):format(id))
            v.type = type
            v.name = name or id
            return
        end
    end
    local record = {
        id = id,
        type = type,
        name = name or id,
        path = "",
    }
    table.insert(tests, record)
end

---@param id string
---@param path string
---@param name string?
function M.registerDynamicTest(id, path, name)
    for _, v in ipairs(tests) do
        if v.id == id then
            lstg.Log(2, ("test '%s' replace with newer one"):format(id))
            v.type = Blank
            v.name = name or id
            v.path = path
            return
        end
    end
    local record = {
        id = id,
        type = Blank,
        name = name or id,
        path = path,
    }
    table.insert(tests, record)
end

function M.onCreate()
    current_test = Blank
    current_test:onCreate()
end

function M.onDestroy()
    current_test:onDestroy()
    current_test = Blank
end

function M.onUpdate()
    if imgui_exist then
        ---@diagnostic disable-next-line: undefined-field
        local ImGui = imgui.ImGui
        if ImGui.Begin("Select Test") then
            for _, v in ipairs(tests) do
                if ImGui.Button(v.name) then
                    M.setTest(v.id)
                end
            end
        end
        ImGui.End()
    end
    current_test:onUpdate()
end

function M.onRender()
    current_test:onRender()
end

---@param id string
function M.setTest(id)
    for _, v in ipairs(tests) do
        if v.id == id then
            current_test:onDestroy()

            if v.path:len() > 0 then
                local success, result = pcall(function()
                    v.type = lstg.DoFile(v.path)
                end)
                if not success then
                    lstg.Log(4, ("load test script '%s' failed: %s"):format(v.path, tostring(result)))
                end
            end
            current_test = setmetatable({}, { __index = v.type })
            current_test:onCreate()
            return
        end
    end
end

return M
