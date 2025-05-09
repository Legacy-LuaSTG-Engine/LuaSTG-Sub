local test = require("test")

---@class test.Module.PostEffect : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")

    lstg.LoadTexture("tex:block", "res/block.png")
    local w, h = lstg.GetTextureSize("tex:block")
    lstg.LoadImage("img:block", "tex:block", 0, 0, w, h)
    lstg.CreateRenderTarget("rt:target")
    lstg.LoadFX("fx:rgb_select", "res/rgb_select.hlsl")
    lstg.LoadFX("fx:rgb_select_new", "res/rgb_select_new.hlsl")
    self.shader = lstg.CreatePostEffectShader("res/shader_mask.hlsl")

    lstg.CreateRenderTarget("rt:background_1")
    lstg.CreateRenderTarget("rt:mask_1")
    do
        lstg.LoadTexture("tex:mask_1", "res/mask_1.png")
        local ww, hh = lstg.GetTextureSize("tex:mask_1")
        lstg.LoadImage("img:mask_1", "tex:mask_1", 0, 0, ww, hh)
    end
    do
        lstg.LoadTexture("tex:image_1", "res/image_1.png")
        local ww, hh = lstg.GetTextureSize("tex:image_1")
        lstg.LoadImage("img:image_1", "tex:image_1", 0, 0, ww, hh)
    end

    lstg.SetResourceStatus(old_pool)

    self.timer = -1
    self.r = 0.0
    self.g = 0.0
    self.b = 0.0
    local function wait(f)
        for _ = 1, f do
            coroutine.yield()
        end
    end
    self.task = coroutine.create(function()
        while true do
            -- r
            for i = 1, 60 do
                self.r = i / 60
                wait(1)
            end
            wait(60)
            for i = 59, 0, -1 do
                self.r = i / 60
                wait(1)
            end
            wait(60)
            -- g
            for i = 1, 60 do
                self.g = i / 60
                wait(1)
            end
            wait(60)
            for i = 59, 0, -1 do
                self.g = i / 60
                wait(1)
            end
            wait(60)
            -- b
            for i = 1, 60 do
                self.b = i / 60
                wait(1)
            end
            wait(60)
            for i = 59, 0, -1 do
                self.b = i / 60
                wait(1)
            end
            wait(60)
        end
    end)
end

function M:onDestroy()
    lstg.RemoveResource("global", 2, "img:block")
    lstg.RemoveResource("global", 1, "tex:block")
    lstg.RemoveResource("global", 1, "rt:target")
    lstg.RemoveResource("global", 9, "fx:rgb_select")
    lstg.RemoveResource("global", 9, "fx:rgb_select_new")

    lstg.RemoveResource("global", 2, "img:mask_1")
    lstg.RemoveResource("global", 1, "tex:mask_1")
    lstg.RemoveResource("global", 2, "img:image_1")
    lstg.RemoveResource("global", 1, "tex:image_1")
    lstg.RemoveResource("global", 1, "rt:background_1")
    lstg.RemoveResource("global", 1, "rt:mask_1")
end

function M:onUpdate()
    self.timer = self.timer + 1
    if coroutine.status(self.task) ~= "dead" then
        assert(coroutine.resume(self.task))
    end
end

function M:onRender()
    lstg.PushRenderTarget("rt:target")
    window:applyCameraV()
    lstg.Render("img:block", window.width * 1 / 4, window.height / 2, 0, 1)
    lstg.PopRenderTarget() -- "rt:target"
    lstg.PostEffect("fx:rgb_select", "rt:target", 6, "mul+alpha", -- 着色器名称，屏幕渲染目标，采样器类型，（最终绘制出来的）混合模式
        -- 浮点参数
        {
            -- self.r, self.g, self.b, 0.0 -- test error report
            { self.r, self.g, self.b, 0.0 }, -- channel_factor(r, g, b, X)
        },
        -- 纹理与采样器类型参数
        {}
    )

    lstg.PushRenderTarget("rt:target")
    window:applyCameraV()
    lstg.Render("img:block", window.width * 3 / 4, window.height / 2, 0, 1)
    lstg.PopRenderTarget() -- "rt:target"
    lstg.PostEffect("rt:target", "fx:rgb_select_new", "mul+alpha", -- 屏幕渲染目标，着色器名称，（最终绘制出来的）混合模式
        -- 其他参数
        {
            channel_factor = lstg.Color(255, 255 * self.r, 255 * self.g, 255 * self.b),
        }
    )

    lstg.PushRenderTarget("rt:background_1")
        lstg.RenderClear(lstg.Color(0))
        lstg.Render("img:image_1", 128, 128, 0, 1)
    lstg.PopRenderTarget()
    lstg.PushRenderTarget("rt:mask_1")
        lstg.RenderClear(lstg.Color(0))
        lstg.Render("img:mask_1", 128, 128, 0, 1)
    lstg.PopRenderTarget()
    self.shader:setTexture("g_texture", "rt:mask_1")
    self.shader:setTexture("g_render_target", "rt:background_1")
    self.shader:setFloat2("g_render_target_size", window.width, window.height)
    self.shader:setFloat4("g_viewport", 128 + 128 * lstg.sin(self.timer), 0, window.width, window.height)
    lstg.PostEffect(self.shader, "")
end

test.registerTest("test.Module.PostEffect", M)
