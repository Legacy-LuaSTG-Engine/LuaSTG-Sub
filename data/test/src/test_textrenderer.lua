local test = require("test")

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

---@class test.Module.TextRenderer : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 16, 16)
    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 8, "Sans")
end

function M:onUpdate()
end

function M:onRender()
    --set_camera()

    --local k = lstg.Input.Keyboard
    --local m = lstg.Input.Mouse
    
    --local kk1 = k.GetKeyState(k.X)
    --local mk1, mk2, mk3, mk4, mk5 = m.GetKeyState(m.Left), m.GetKeyState(m.Middle), m.GetKeyState(m.Right), m.GetKeyState(m.X1), m.GetKeyState(m.X2)
    --local mx1, my1 = lstg.Input.Mouse.GetPosition()
    --local mx2, my2 = lstg.Input.Mouse.GetPosition(true)
    --local mz = lstg.Input.Mouse.GetWheelDelta()
    --
    --local st = string.format("%s\n%s, %s, %s, %s, %s\n%.2f, %.2f\n%.2f, %.2f\n%.2f",
    --    kk1,
    --    mk1, mk2, mk3, mk4, mk5,
    --    mx1, my1,
    --    mx2, my2,
    --    mz
    --)

    --local c_white = lstg.Color(255, 255, 255, 255)

    --lstg.RenderTTF("Sans", st, 0, 0, 720, 720, 0 + 0, c_white, 2)

    --local va, vb, vc = lstg.GetVersionNumber()

    --lstg.RenderTTF("Sans", string.format("版本号 %d.%d.%d", va, vb, vc), 0, 0, 64, 64, 0 + 8, c_white, 2)
    --lstg.RenderTTF("Sans", lstg.GetVersionName(), 0, 0, 0, 0, 0 + 8, c_white, 2)
    --lstg.RenderTTF("Sans", string.format("timer: %d", timer), 0, 0, 32, 32, 0 + 8, c_white, 2)

    --lstg.RenderTTF("Sans", "您好，别来无恙啊！", 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)

    --RenderTTF3(
    --    "Sans", "旋转测试 ++++ 测试文本，。……",
    --    window.width / 2, window.height / 2,
    --    timer,
    --    4, 4,
    --    "", lstg.Color(255, 0, 0, 0),
    --    "center", "vcenter")
end

test.registerTest("test.Module.TextRenderer", M)
