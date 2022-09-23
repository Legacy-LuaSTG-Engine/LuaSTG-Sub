---@class test.Window
window = {}
window.width = 1920
window.height = 1080
function window:init()
    lstg.ChangeVideoMode(self.width, self.height, true, true)
end
function window:applyCameraV()
    lstg.SetViewport(0, self.width, 0, self.height)
    lstg.SetScissorRect(0, self.width, 0, self.height)
    lstg.SetOrtho(0, self.width, 0, self.height)
    lstg.SetImageScale(1)
    lstg.SetFog()
end

local imgui = require("imgui")
local test = require("test")

local gpu_list = {}
local select_gpu = ""
local function changeGpu()
    if string.len(select_gpu) > 0 then
        lstg.ChangeGPU(select_gpu)
        gpu_list = lstg.EnumGPUs() -- update
        select_gpu = ""
    end
end
local function showSelectGpuWindow()
    local ImGui = imgui.ImGui
    if #gpu_list < 1 then
        gpu_list = lstg.EnumGPUs()
    end
    if ImGui.Begin("Select GPU") then
        for _, v in ipairs(gpu_list) do
            if ImGui.Button(v) then
                select_gpu = v
            end
        end
    end
    ImGui.End()
end

require("test_textrenderer")
require("test_texture")
require("test_sampler")
require("test_mesh")
require("test_stringpack")
require("test_log")
require("test_filesys")
require("test_dwrite")
require("test_colli")
require("test_posteffect")
require("test_monitor")

function GameInit()
    window:init()
    test.onCreate()
end
function GameExit()
    test.onDestroy()
end
function FrameFunc()
    changeGpu()
    imgui.backend.NewFrame()
    imgui.ImGui.NewFrame()
    --imgui.ImGui.ShowDemoWindow()
    --imgui.backend.ShowTestInputWindow()
    --imgui.backend.ShowMemoryUsageWindow()
    imgui.backend.ShowFrameStatistics()
    imgui.backend.ShowResourceManagerDebugWindow()
    showSelectGpuWindow()
    test.onUpdate()
    imgui.ImGui.EndFrame()
    local Key = lstg.Input.Keyboard
    if Key.GetKeyState(Key.Escape) then
        return true
    end
    return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 128, 128, 128))
    test.onRender()
    imgui.ImGui.Render()
    imgui.backend.RenderDrawData()
    lstg.EndScene()
end
