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

    lstg.SetResourceStatus(old_pool)

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
end

function M:onUpdate()
    if coroutine.status(self.task) ~= "dead" then
        assert(coroutine.resume(self.task))
    end
end

function M:onRender()
    lstg.PushRenderTarget("rt:target")
    window:applyCameraV()
    lstg.Render("img:block", window.width / 2, window.height / 2, 0, 1)
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
end

test.registerTest("test.Module.PostEffect", M)
