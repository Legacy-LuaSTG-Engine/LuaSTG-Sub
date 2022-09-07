--------------------------------------------------------------------------------
--- LuaSTG basic example
--- by Kuanlan@outlook.com
--------------------------------------------------------------------------------

local Keyboard = lstg.Input.Keyboard
local task = require("task")
local global_tasks = task.Manager()

--------------------------------------------------------------------------------
--- camera

local window = { width = 1280, height = 720, }
local function applyCamera()
    lstg.SetViewport(0, window.width, 0, window.height)
    lstg.SetScissorRect(0, window.width, 0, window.height)
    lstg.SetOrtho(0, window.width, 0, window.height)
    lstg.SetImageScale(1)
    lstg.SetFog()
end

--------------------------------------------------------------------------------
--- game object

local LAYER_PLAYER = 0
local LAYER_ENEMY_BULLET = 100
local GROUP_PLAYER = 1
local GROUP_ENEMY_BULLET = 2
local function clamp(v, a, b)
    return math.max(a, math.min(v, b))
end
local player_class = {
    function(self)
        self.group = GROUP_PLAYER
        self.layer = LAYER_PLAYER
        self.img = "player-rect"
        self.rect = true
        self.a = 8 -- 16x16
        self.b = 8
        self.bound = false
        self.x = window.width / 2
        self.y = window.height / 4
    end,
    function(self) end,
    function(self)
        local dx = 0
        local dy = 0
        if Keyboard.GetKeyState(Keyboard.Left) and Keyboard.GetKeyState(Keyboard.Right) then
            dx = 0
        elseif Keyboard.GetKeyState(Keyboard.Left) then
            dx = -1
        elseif Keyboard.GetKeyState(Keyboard.Right) then
            dx = 1
        end
        if Keyboard.GetKeyState(Keyboard.Down) and Keyboard.GetKeyState(Keyboard.Up) then
            dy = 0
        elseif Keyboard.GetKeyState(Keyboard.Down) then
            dy = -1
        elseif Keyboard.GetKeyState(Keyboard.Up) then
            dy = 1
        end
        if dx ~= 0 and dy ~= 0 then
            local k = math.sqrt(2.0) * 0.5
            dx = dx * k
            dy = dy * k
        end
        local speed = 4
        if Keyboard.GetKeyState(Keyboard.LeftShift) then
            speed = 2
        end
        self.x = clamp(self.x + speed * dx, 8, window.width - 8)
        self.y = clamp(self.y + speed * dy, 8, window.height - 8)
    end,
    lstg.DefaultRenderFunc,
    function(self, other)
        if other.group == GROUP_ENEMY_BULLET then
            lstg.Del(other)
        end
    end,
    function(self) end;
    is_class = true,
}
local bullet_class = {
    function(self)
        self.group = GROUP_ENEMY_BULLET
        self.layer = LAYER_ENEMY_BULLET
        self.img = "bullet-rect"
        self.rect = true
        self.a = 4 -- 16x16 / 2
        self.b = 4
        self.hscale = 0.5
        self.vscale = 0.5
    end,
    function(self) end,
    function(self) end,
    lstg.DefaultRenderFunc,
    function(self, other) end,
    function(self) end;
    is_class = true,
}
local function createBullet(x, y, a, v)
    local obj = lstg.New(bullet_class)
    obj.x = x
    obj.y = y
    lstg.SetV(obj, v, a, true)
    return obj
end
local function buildGameObjectScene()
    lstg.ResetPool()
    lstg.New(player_class)
    global_tasks:add(function()
        while true do
            local x, y = window.width / 2, window.height / 2
            for i = 0, 239 do
                createBullet(x, y, (i / 240) * 360, 3)
            end
            task.wait(4)
        end
    end)
end
local function updateGameObject()
    lstg.ObjFrame()
    lstg.SetBound(0, window.width, 0, window.height)
    lstg.BoundCheck()
    lstg.CollisionCheck(GROUP_PLAYER, GROUP_ENEMY_BULLET)
    lstg.UpdateXY()
    lstg.AfterFrame()
end
local function renderGameObject()
    lstg.ObjRender()
end

--------------------------------------------------------------------------------
--- background

local timer = 0
local function updateBackground()
    timer = timer + 1
end
local function renderText(font, text, x, y, scale, color, oscale, ocolor, align)
    local d = oscale
    local k = oscale * math.sqrt(2.0) * 0.5
    local s = 2 * scale
    lstg.RenderTTF(font, text, x - d, x - d, y, y, align, ocolor, s)
    lstg.RenderTTF(font, text, x + d, x + d, y, y, align, ocolor, s)
    lstg.RenderTTF(font, text, x, x, y - d, y - d, align, ocolor, s)
    lstg.RenderTTF(font, text, x, x, y + d, y + d, align, ocolor, s)
    lstg.RenderTTF(font, text, x - k, x - k, y + k, y + k, align, ocolor, s)
    lstg.RenderTTF(font, text, x + k, x + k, y + k, y + k, align, ocolor, s)
    lstg.RenderTTF(font, text, x + k, x + k, y - k, y - k, align, ocolor, s)
    lstg.RenderTTF(font, text, x - k, x - k, y - k, y - k, align, ocolor, s)
    lstg.RenderTTF(font, text, x, x, y, y, align, color, s)
end
local function renderBackground()
    local cx, cy = window.width / 2, window.height / 2
    renderText("Sans", "海内存知己\n天涯若比邻\n欢迎来到 LuaSTG Sub",
        cx, cy,
        1, lstg.Color(255, 2255, 255, 255),
        4, lstg.Color(255, 0, 0, 0),
        1 + 4)
    local circle_r = 300
    for i = 0, (360 - 1), (360 / 60) do
        local angle = i + timer * 0.17
        local cosv, sinv = math.cos(math.rad(angle)), math.sin(math.rad(angle))
        local hscale = 16 + 15 * math.sin(math.rad(i * 11 + timer * 0.11))
        local cr = circle_r + hscale * 8
        lstg.Render("black-rect", cx + cr * cosv, cy + cr * sinv, angle, hscale, 1)
    end
end
local function drawDebugInfo()
    renderText("Sans", "移动/Move: ←↑↓→  低速移动/Slower Move: LeftShift",
        4, 4,
        0.5, lstg.Color(255, 2255, 255, 255),
        2, lstg.Color(255, 0, 0, 0),
        0 + 8)
    local text = string.format("OBJ %d FPS %.2f", lstg.GetnObj(), lstg.GetFPS())
    renderText("Sans", text,
        window.width - 4, 4,
        0.5, lstg.Color(255, 2255, 255, 255),
        2, lstg.Color(255, 0, 0, 0),
        2 + 8)
end

--------------------------------------------------------------------------------
--- framework

function GameInit()
    lstg.ChangeVideoMode(window.width, window.height, true, false)
    applyCamera()
    lstg.LoadTexture("white", "res/white.png", false)
    lstg.LoadImage("white", "white", 0, 0, 16, 16)
    lstg.LoadImage("black-rect", "white", 0, 0, 16, 16)
    lstg.SetImageState("black-rect", "", lstg.Color(255, 0, 0, 0))
    lstg.LoadImage("player-rect", "white", 0, 0, 16, 16)
    lstg.SetImageState("player-rect", "", lstg.Color(255, 64, 64, 255))
    lstg.LoadImage("bullet-rect", "white", 0, 0, 16, 16)
    lstg.SetImageState("bullet-rect", "", lstg.Color(96, 0, 0, 0))
    if not lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 48, 48) then
        lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttf", 48, 48) -- Windows 7
    end
    buildGameObjectScene()
end
function GameExit()
end
function FrameFunc()
    global_tasks:remove_dead()
    global_tasks:resume_all()
    updateBackground()
    updateGameObject()
    if Keyboard.GetKeyState(Keyboard.Escape) then
        return true -- exit
    end
    return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    applyCamera()
    renderGameObject()
    renderBackground()
    drawDebugInfo()
    lstg.EndScene()
end
