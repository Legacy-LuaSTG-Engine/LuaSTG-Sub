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
---@param width number
---@param height number
function window:setSize(width, height)
    window.width = width
    window.height = height
    lstg.ChangeVideoMode(self.width, self.height, true, true)
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
local function showSelectResolutionWindow()
    local ImGui = imgui.ImGui
    local list = {
        {1280,720,"1280x720"},
        {1600,900,"1600x900"},
        {1920,1080,"1920x1080"},
    }
    if ImGui.Begin("Select Resolution") then
        for _, v in ipairs(list) do
            if ImGui.Button(v[3]) then
                window:setSize(v[1], v[2])
            end
        end
    end
    ImGui.End()
end

local adv_list = {}
local select_adv = ""
local function changeAudioDevice()
    if string.len(select_adv) > 0 then
        lstg.ChangeAudioDevice(select_adv)
        adv_list = lstg.ListAudioDevice(true) -- update
        select_adv = ""
    end
end
local function showSelectAudioDeviceWindow()
    local ImGui = imgui.ImGui
    if #adv_list < 1 then
        adv_list = lstg.ListAudioDevice()
    end
    if ImGui.Begin("Select Audio Device") then
        for _, v in ipairs(adv_list) do
            if ImGui.Button(v) then
                select_adv = v
            end
        end
    end
    ImGui.End()
end

require("test_rendertarget")
require("test_textrenderer")
require("test_texture")
require("test_sampler")
require("test_model")
require("test_mesh")
require("test_stringpack")
require("test_log")
require("test_filesys")
require("test_dwrite")
require("test_colli")
require("test_posteffect")
require("test_blend_color_burn")
require("test_monitor")
require("test_utf8api")
require("test_lfs")
require("test_f2dfont")
require("test_hgefont")
require("test_ttf")

function GameInit()
    window:init()
    test.onCreate()
end
function GameExit()
    test.onDestroy()
end
function FrameFunc()
    changeGpu()
    imgui.backend.NewFrame(true)
    imgui.ImGui.NewFrame()
    --imgui.ImGui.ShowDemoWindow()
    --imgui.backend.ShowTestInputWindow()
    --imgui.backend.ShowMemoryUsageWindow()
    imgui.backend.ShowFrameStatistics()
    imgui.backend.ShowResourceManagerDebugWindow()
    showSelectGpuWindow()
    showSelectResolutionWindow()
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
