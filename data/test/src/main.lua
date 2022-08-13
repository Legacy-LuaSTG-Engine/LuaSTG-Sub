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

require("test_textrenderer")
require("test_texture")
require("test_sampler")
require("test_mesh")
require("test_stringpack")
require("test_log")
require("test_filesys")

function GameInit()
    window:init()
    test.onCreate()
end
function GameExit()
    test.onDestroy()
end
function FrameFunc()
    imgui.backend.NewFrame()
    imgui.ImGui.NewFrame()
    --imgui.ImGui.ShowDemoWindow()
    --imgui.backend.ShowTestInputWindow()
    --imgui.backend.ShowMemoryUsageWindow()
    imgui.backend.ShowFrameStatistics()
    imgui.backend.ShowResourceManagerDebugWindow()
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
