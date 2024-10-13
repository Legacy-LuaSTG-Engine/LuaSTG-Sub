local timer = 0
-- local window = { width = 1280, height = 720, }
local window = { width = 1920, height = 1080, }
local Keyboard = lstg.Input.Keyboard
local task = require("task")
local global_tasks = task.Manager()
local global_color = lstg.Color(255, 0, 0, 0)
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
    lstg.CreateRenderTarget("mainRdrTgt")
    lstg.LoadImage("mainRdrTgt", "mainRdrTgt", 0, 0, window.width, window.height)
     lstg.SetTextureSamplerState("mainRdrTgt", "linear+wrap")
    global_tasks:add(function()
        while true do
            for i = 1, 60 do
                global_color.r = (i / 60) * 128
                global_color.b = (i / 60) * 128
                task.wait(1)
            end
            for i = 59, 0, -1 do
                global_color.r = (i / 60) * 128
                global_color.b = (i / 60) * 128
                task.wait(1)
            end
            global_tasks:add(function()
                for i = 1, 15 do
                    global_color.g = (i / 15) * 192
                    task.wait(1)
                end
                for i = 14, 0, -1 do
                    global_color.g = (i / 15) * 192
                    task.wait(1)
                end
            end)
        end
    end)
end
function GameExit()
end
function FrameFunc()
    if lstg.GetLastKey() == lstg.Input.Keyboard.F1 then
        window.width, window.height = 1920, 1080
        lstg.ChangeVideoMode(window.width, window.height, true, false)
    elseif lstg.GetLastKey() == lstg.Input.Keyboard.F2 then
        window.width, window.height = 1600, 900
        lstg.ChangeVideoMode(window.width, window.height, true, false)
    end
    global_tasks:remove_dead()
    global_tasks:resume_all()
    lstg.AfterFrame(2) -- TODO: remove (2)
    lstg.ObjFrame(2) -- TODO: remove (2)
    lstg.CollisionCheck(1, 2) -- group 1 and group 2
    lstg.BoundCheck(2) -- TODO: remove (2)
    timer = timer + 1
    if Keyboard.GetKeyState(Keyboard.Escape) then
        return true -- exit
    end
    return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.PushRenderTarget("mainRdrTgt")
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    set_camera()
    lstg.ObjRender()
    local cx, cy = window.width / 2, window.height / 2
    lstg.RenderTTF("Sans", "海内存知己\n天涯若比邻\n欢迎来到 LuaSTG Sub", cx, cx, cy, cy, 1 + 4, lstg.Color(255, 0, 0, 0), 2)
    lstg.SetImageState("white", "", global_color)
    local circle_r = 300
    for i = 0, (360 - 1), (360 / 60) do
        local angle = i + timer * 0.17
        local cosv, sinv = math.cos(math.rad(angle)), math.sin(math.rad(angle))
        local hscale = 16 + 15 * math.sin(math.rad(i * 11 + timer * 0.11))
        local cr = circle_r + hscale * 8
        lstg.Render("white", cx + cr * cosv, cy + cr * sinv, angle, hscale, 1)
    end
    lstg.PopRenderTarget()
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    set_camera()
    -- Alternate rendering method
     lstg.RenderTexture(
         "mainRdrTgt", "",
         {0, window.height, 0, timer, timer, lstg.Color(0xFFFFFFFF)},
         {window.width, window.height, 0, window.width + timer, timer, lstg.Color(0xFFFFFFFF)},
         {window.width, 0, 0, window.width + timer, window.height + timer, lstg.Color(0xFFFFFFFF)},
         {0, 0, 0, timer, window.height + timer, lstg.Color(0xFFFFFFFF)}
     )
    --lstg.RenderRect("mainRdrTgt", 0, window.width, 0, window.height)
    lstg.EndScene()
end
