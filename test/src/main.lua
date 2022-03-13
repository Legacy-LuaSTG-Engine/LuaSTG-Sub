local timer = 0
local window = { width = 1920, height = 1080, }
local imgui = require("imgui")
local function set_camera()
    lstg.SetViewport(0, window.width, 0, window.height)
    lstg.SetScissorRect(0, window.width, 0, window.height)
    lstg.SetOrtho(0, window.width, 0, window.height)
    lstg.SetImageScale(1)
    lstg.SetFog()
end
function GameInit()
    --lstg.ChangeVideoMode(window.width, window.height, true, true)
    set_camera()
    lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 16, 16)
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
	return false
end
function RenderFunc()
    lstg.BeginScene()
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    set_camera()
    lstg.RenderTTF("Sans", "您好，别来无恙啊！", 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)

    RenderTTF3(
        "Sans", "旋转测试 ++++ 测试文本，。……",
        window.width / 2, window.height / 2,
        timer / 4,
        4, 4,
        "", lstg.Color(255, 0, 0, 0),
        "center", "vcenter")

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
