local timer = 0
local window = { width = 1280, height = 720, }
local function set_camera()
    lstg.SetViewport(0, window.width, 0, window.height)
    lstg.SetScissorRect(0, window.width, 0, window.height)
    lstg.SetOrtho(0, window.width, 0, window.height)
    lstg.SetImageScale(1)
    lstg.SetFog()
end
function GameInit()
    lstg.ChangeVideoMode(window.width, window.height, true, false)
    set_camera()
    lstg.LoadTexture("white", "res/white.png", false)
    lstg.LoadImage("white", "white", 0, 0, 16, 16)
    if not lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 48, 48) then
        lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttf", 48, 48) -- Windows 7
    end
end
function GameExit()
end
function FrameFunc()
    timer = timer + 1
	return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    set_camera()
    local cx, cy = window.width / 2, window.height / 2
    lstg.RenderTTF("Sans", "海内存知己\n天涯若比邻\n欢迎来到 LuaSTG Sub", cx, cx, cy, cy, 1 + 4, lstg.Color(255, 0, 0, 0), 2)
    lstg.SetImageState("white", "", lstg.Color(255, 0, 0, 0))
    local circle_r = 300
    for i = 0, (360 - 1), (360 / 60) do
        local angle = i + timer * 0.17
        local cosv, sinv = math.cos(math.rad(angle)), math.sin(math.rad(angle))
        local hscale = 16 + 15 * math.sin(math.rad(i * 11 + timer * 0.11))
        local cr = circle_r + hscale * 8
        lstg.Render("white", cx + cr * cosv, cy + cr * sinv, angle, hscale, 1)
    end
    lstg.EndScene()
end
