local imgui = require("imgui")
local function set_camera()
    lstg.SetViewport(0, 1280, 0, 720)
    lstg.SetScissorRect(0, 1280, 0, 720)
    lstg.SetOrtho(0, 1280, 0, 720)
    lstg.SetImageScale(1)
    lstg.SetFog()
end
function GameInit()
    --lstg.ChangeVideoMode(1280, 720, true, true)
    set_camera()
    lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 16, 16)
end
function GameExit()
end
function FrameFunc()
    imgui.backend.NewFrame()
    imgui.ImGui.NewFrame()
    imgui.ImGui.ShowDemoWindow()
    imgui.backend.ShowMemoryUsageWindow()
    imgui.backend.ShowFrameStatistics()
    imgui.ImGui.EndFrame()
	return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    set_camera()
    lstg.RenderTTF("Sans", "您好，别来无恙啊！", 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)
    imgui.ImGui.Render()
    imgui.backend.RenderDrawData()
    lstg.EndScene()
end
