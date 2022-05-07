local timer = 0
local window = { width = 2560, height = 1440, }
local Key = require("keycode").Keyboard
local MKey = require("keycode").Mouse
local imgui = require("imgui")
local function set_camera()
    lstg.SetViewport(0, window.width, 0, window.height)
    lstg.SetScissorRect(0, window.width, 0, window.height)
    lstg.SetOrtho(0, window.width, 0, window.height)
    lstg.SetImageScale(1)
    lstg.SetFog()
end
local function Camera3D()
    ---@class kuanlan.Camera3D
    local M = {
        dir = 90,
        upd = 0,
        x = 0,
        y = 0,
        z = -10,
        speed = 0.05,

        rbtn = false,
        mx = 0,
        my = 0,
        last_dir = 90,
        last_upd = 0,
        aspeed = 0.1,
    }
    function M:update()
        local speed = self.speed
        if lstg.GetKeyState(Key.CTRL) then
            speed = speed * 10.0
        end
        if lstg.GetKeyState(Key.W) then
            self.x = self.x + speed * math.cos(math.rad(self.dir))
            self.z = self.z + speed * math.sin(math.rad(self.dir))
        elseif lstg.GetKeyState(Key.S) then
            self.x = self.x - speed * math.cos(math.rad(self.dir))
            self.z = self.z - speed * math.sin(math.rad(self.dir))
        end
        if lstg.GetKeyState(Key.D) then
            self.x = self.x + speed * math.cos(math.rad(self.dir - 90))
            self.z = self.z + speed * math.sin(math.rad(self.dir - 90))
        elseif lstg.GetKeyState(Key.A) then
            self.x = self.x + speed * math.cos(math.rad(self.dir + 90))
            self.z = self.z + speed * math.sin(math.rad(self.dir + 90))
        end
        if lstg.GetKeyState(Key.SPACE) then
            self.y = self.y + speed
        elseif lstg.GetKeyState(Key.SHIFT) then
            self.y = self.y - speed
        end
        if not self.rbtn and lstg.GetMouseState(MKey.RBUTTON) then
            self.rbtn = true
            self.mx, self.my = lstg.GetMousePosition()
            self.last_dir = self.dir
            self.last_upd = self.upd
        elseif self.rbtn and not lstg.GetMouseState(MKey.RBUTTON) then
            self.rbtn = false
        end
        if self.rbtn then
            local mx, my = lstg.GetMousePosition()
            local dx, dy = mx - self.mx, my - self.my
            self.dir = self.last_dir - self.aspeed * dx
            self.upd = self.last_upd + self.aspeed * dy
            self.upd = math.max(-89.0, math.min(self.upd, 89.0))
        end
    end
    function M:apply()
        local tx, ty, tz = 1, 0, 0
        local ux, uy, uz = 0, 1, 0
        local function vec2_rot(x, y, r_deg)
            local sin_v = math.sin(math.rad(r_deg))
            local cos_v = math.cos(math.rad(r_deg))
            return x * cos_v - y * sin_v, x * sin_v + y * cos_v
        end
        tx, ty = vec2_rot(tx, ty, self.upd)
        ux, uy = vec2_rot(ux, uy, self.upd)
        tx, tz = vec2_rot(tx, tz, self.dir)
        ux, uz = vec2_rot(ux, uz, self.dir)
        lstg.SetViewport(0, window.width, 0, window.height)
        lstg.SetScissorRect(0, window.width, 0, window.height)
        lstg.SetPerspective(
            self.x, self.y, self.z,
            self.x + tx, self.y + ty, self.z + tz,
            ux, uy, uz,
            math.rad(80), window.width / window.height,
            0.01, 1000.0
        )
        lstg.SetImageScale(1)
        lstg.SetFog(5, 20, lstg.Color(255, 255, 255, 255))
    end
    return M
end
local ps = nil
function GameInit()
    lstg.ChangeVideoMode(window.width, window.height, true, true)
    lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 16, 16)
    lstg.LoadTexture("tex:particles", "res/particles.png")
    lstg.LoadImage("img:particle1", "tex:particles", 0, 0, 32, 32)
    lstg.LoadPS("ps:1", "res/ghost_fire_1.psi", "img:particle1")
    ps = lstg.ParticleSystemInstance("ps:1")

    local cjson = require("cjson")
    lstg.Print(cjson.encode({hello="world"}))
    lstg.Print(cjson.decode("{\"hello\": \"world\"}").hello)

    lstg.FileManager.CreateDirectory("测试test")
    lstg.FileManager.CreateDirectory("测试test2\\")
    lstg.FileManager.CreateDirectory("测试test3/")
    lstg.FileManager.CreateDirectory("第一层/第二层/第三层/第四层/第五层/第六层/第七层/第八层/第九层/第十层")
    lstg.FileManager.CreateDirectory("第1层/第2层/第3层/第4层/第5层/第6层/第7层/第8层/第9层/第10层")

    lstg.FileManager.RemoveDirectory("测试test")
    lstg.FileManager.RemoveDirectory("测试test2\\")
    lstg.FileManager.RemoveDirectory("测试test3/")
    lstg.FileManager.RemoveDirectory("第一层")
    lstg.FileManager.RemoveDirectory("第1层/")
end
function GameExit()
end
function FrameFunc()
    timer = timer + 1
    imgui.backend.NewFrame()
    imgui.ImGui.NewFrame()
    imgui.ImGui.ShowDemoWindow()
    imgui.backend.ShowMemoryUsageWindow()
    imgui.backend.ShowFrameStatistics()
    imgui.ImGui.EndFrame()
    ps:Update(64, 64, 0)
    return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 64, 64, 64))
    lstg.ClearZBuffer(1.0)

    set_camera()

    local k = lstg.Input.Keyboard
    local m = lstg.Input.Mouse
    
    local kk1 = k.GetKeyState(k.X)
    local mk1, mk2, mk3, mk4, mk5 = m.GetKeyState(m.Left), m.GetKeyState(m.Middle), m.GetKeyState(m.Right), m.GetKeyState(m.X1), m.GetKeyState(m.X2)
    local mx1, my1 = lstg.Input.Mouse.GetPosition()
    local mx2, my2 = lstg.Input.Mouse.GetPosition(true)
    local mz = lstg.Input.Mouse.GetWheelDelta()

    local st = string.format("%s\n%s, %s, %s, %s, %s\n%.2f, %.2f\n%.2f, %.2f\n%.2f",
        kk1,
        mk1, mk2, mk3, mk4, mk5,
        mx1, my1,
        mx2, my2,
        mz
    )

    lstg.RenderTTF("Sans", st, 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)

    local va, vb, vc = lstg.GetVersionNumber()

    lstg.RenderTTF("Sans", string.format("版本号 %d.%d.%d", va, vb, vc), 0, 0, 64, 64, 0 + 8, lstg.Color(255, 0, 0, 0), 2)
    lstg.RenderTTF("Sans", lstg.GetVersionName(), 0, 0, 0, 0, 0 + 8, lstg.Color(255, 0, 0, 0), 2)

    --lstg.RenderTTF("Sans", "您好，别来无恙啊！", 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)

    --RenderTTF3(
    --    "Sans", "旋转测试 ++++ 测试文本，。……",
    --    window.width / 2, window.height / 2,
    --    timer,
    --    4, 4,
    --    "", lstg.Color(255, 0, 0, 0),
    --    "center", "vcenter")

    ps:Render(1)

    imgui.ImGui.Render()
    imgui.backend.RenderDrawData()
    lstg.EndScene()
end

---@param ttfname string
---@param text string
---@param x number
---@param y number
---@param rot number
---@param hscale number
---@param vscale number
---@param blend lstg.BlendMode
---@param color lstg.Color
---@return number, number
function RenderTTF3(ttfname, text, x, y, rot, hscale, vscale, blend, color, ...)
    -- 警告：这里的实现代码以后可能会变化，甚至转为 C++ 实现
    -- 警告：请勿直接使用这些 Native API
    
    -- 翻译对齐
    local args = {...}
    local halign = 0
    local valign = 0
    for _, v in ipairs(args) do
        if v == "center" then
            halign = 1
        elseif v == "right" then
            halign = 2
        elseif v == "vcenter" then
            valign = 1
        elseif v == "bottom" then
            valign = 2
        end
    end

    -- 设置字体
    
    local fr = lstg.FontRenderer
    fr.SetFontProvider(ttfname)
    fr.SetScale(hscale, vscale)

    -- 计算笔触位置

    local x0, y0 = x, y
    local l, r, b, t = fr.MeasureTextBoundary(text)
    local w, h = r - l, t - b
    if halign == 0 then
        x = x - l -- 使左边缘对齐 x
    elseif halign == 1 then
        x = (x - l) - (w / 2) -- 居中
    else -- "right"
        x = x - r -- 使右边缘对齐 x
    end
    if valign == 0 then
        y = y - t -- 使顶边缘对齐 y
    elseif valign == 1 then
        y = (y - b) - (h / 2) -- 居中
    else -- "bottom"
        y = y - b -- 使底边缘对齐 y
    end

    -- 对笔触位置进行旋转

    local cos_v = math.cos(math.rad(rot))
    local sin_v = math.sin(math.rad(rot))
    local dx = x - x0
    local dy = y - y0
    local x1 = x0 + dx * cos_v - dy * sin_v
    local y1 = y0 + dx * sin_v + dy * cos_v

    -- 绘制

    local r, x2, y2 = fr.RenderTextInSpace(
        text,
        x1, y1, 0.5,
        math.cos(math.rad(rot))     , math.sin(math.rad(rot))     , 0,
        math.cos(math.rad(rot - 90)), math.sin(math.rad(rot - 90)), 0,
        blend, color
    )
    assert(r)

    return x2, y2
end

if false then
    for it in lfs.dir(".") do
        lstg.Print(it, lfs.attributes("./" .. it, "mode"))
    end
    do
        local f, e = io.open("你好 io.open，你可终于支持中文辣.txt", "wb")
        assert(f, e)
        f:write("你妈死了")
        f:close()
    end
    do
        local f, e = io.open("中文测试测试.txt", "wb")
        assert(f, e)
        f:write("你妈死了2")
        f:close()
        os.remove("中文测试测试.txt")
    end
    do
        local f, e = io.open("中文测试测试 Test test 还有空格.txt", "wb")
        assert(f, e)
        f:write("你妈死了3")
        f:close()
        local f2 = io.open("Test 测试重命名.txt", "rb")
        if f2 then
            f2:close()
            os.remove("Test 测试重命名.txt")
        end
        os.rename("中文测试测试 Test test 还有空格.txt", "Test 测试重命名.txt")
    end
    do
        for _ = 1, 10 do
            local s = os.tmpname()
            lstg.Print(s)
            local f = io.open(s, "wb")
            assert(f)
            f:close()
            os.remove(s)
        end
    end
end
