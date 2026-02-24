---@diagnostic disable: missing-return, duplicate-set-field

--------------------------------------------------------------------------------
--- LuaSTG Sub 渲染器
--- 璀境石
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- 迁移指南

-- 关于新增的 lstg.SetScissorRect：
-- LuaSTG Sub 默认开启裁剪功能，因此建议和 lstg.SetViewport 一起调用，参数可相同
-- 否则可能会出现渲染不出画面的问题

-- 关于 lstg.RenderClear：
-- 在 Direct3D 9 中，微软做了一些魔法，只会清空视口范围
-- 这并不符合现在的图形 API 的风格
-- 请通过 lstg.RenderRect 绘制单色矩形来模拟旧行为

-- 关于 lstg.PostEffectCapture 和 lstg.PostEffectApply：
-- 已经移除
-- 建议通过 lstg.CreateRenderTarget、lstg.PushRenderTarget、
-- lstg.PopRenderTarget、lstg.PostEffect 代替

-- 关于 LuaSTG Ex Plus 曾经加入过的 lstg.SetTextureSamplerState：
-- 已经移除
-- 以后可能会将采样器状态和纹理绑定，或者在 RenderTexture 等 API 添加采样器状态参数

-- 关于 LuaSTG Ex Plus 曾经加入过的 lstg.RenderModel：
-- 已经替换为 glTF 模型，其实现也有变动

--------------------------------------------------------------------------------
--- 混合模式

-- 关于 LuaSTG 的混合模式：
-- LuaSTG 的混合模式由两个部分组成，顶点色混合与渲染管线的混合模式
-- 顶点色混合有
--     mul 顶点色与采样的纹理色的 RGBA 通道相乘
--     add 顶点色与采样的纹理色的 RGB 通道相加，A 通道相乘
-- 渲染管线混合模式有
--     alpha  根据 A 通道进行透明度混合
--     add    颜色相加
--     rev    渲染目标颜色减去像素着色器输出的颜色
--     sub    像素着色器输出的颜色减去渲染目标颜色
--     min    使用颜色值最小的颜色
--     max    使用颜色值最大的颜色
--     mul    颜色相乘（PS 的正片叠底）
--     screen 特殊混合模式，PS 的滤色混合模式
-- 单独的 LuaSTG 混合模式（非顶点色 + 渲染管线混合模式的组合）
--     "one"       输出的颜色覆盖渲染目标上的颜色（相当于禁用混合）
--     "alpha+bal" 通过图片的颜色对渲染目标进行反色，图片黑色区域代表不反色，白色区域则代表反色，中间值则用于过渡
-- 需要注意的是 lstg.PostEffect 方法中顶点色混合会失效
-- 这是因为顶点色混合在内置像素着色器中完成，但是屏幕后处理也通过开发者自定义的像素着色器完成

local M = {}

---@alias lstg.BlendMode '""' | '"mul+alpha"' | '"mul+add"' | '"mul+rev"' | '"mul+sub"' | '"add+alpha"' | '"add+add"' | '"add+rev"' | '"add+sub"' | '"alpha+bal"' | '"mul+min"' | '"mul+max"' | '"mul+mul"' | '"mul+screen"' | '"add+min"' | '"add+max"' | '"add+mul"' | '"add+screen"' | '"one"'

--------------------------------------------------------------------------------
--- 图形功能

--- 启动渲染器
function M.BeginScene()
end

--- 结束渲染并提交
function M.EndScene()
end

--------------------------------------------------------------------------------
--- 渲染管线

--- 设置雾 
--- 不传递参数时关闭雾功能  
--- 当 near 和 far 不小于 0 且 near 小于 far 时，为线性雾  
--- 当 near 为 -1.0 时，为指数雾，far 表示雾密度  
--- 当 near 为 -2.0 时，为二次指数雾，far 表示雾密度  
---@param near number
---@param far number
---@param color lstg.Color
---@overload fun()
function M.SetFog(near, far, color)
end

--- 设置视口，原点位于窗口左下角，x 轴朝右 y 轴朝上
---@param left number
---@param right number
---@param bottom number
---@param top number
function M.SetViewport(left, right, bottom, top)
end

--- [LuaSTG Sub 新增]
--- 设置裁剪矩形，原点位于窗口左下角，x 轴朝右 y 轴朝上
---@param left number
---@param right number
---@param bottom number
---@param top number
function M.SetScissorRect(left, right, bottom, top)
end

--- 设置正交摄像机
---@param left number
---@param right number
---@param bottom number
---@param top number
function M.SetOrtho(left, right, bottom, top)
end

--- 设置透视摄像机，视角为弧度制（非常需要注意）
---@param x number
---@param y number
---@param z number
---@param atx number
---@param aty number
---@param atz number
---@param upx number
---@param upy number
---@param upz number
---@param fovy number
---@param aspect number
---@param zn number
---@param zf number
function M.SetPerspective(x, y, z, atx, aty, atz, upx, upy, upz, fovy, aspect, zn, zf)
end

--- 控制是否使用深度缓冲区，0 关闭，1 开启
---@param state number
function M.SetZBufferEnable(state)
end

--------------------------------------------------------------------------------
--- 渲染目标

--- [LuaSTG Sub 更改]
--- 使用指定颜色清空渲染目标
---@param color lstg.Color
function M.RenderClear(color)
end

--- 以指定深度值清空深度缓冲区，一般填 1.0
---@param depth number
function M.ClearZBuffer(depth)
end

-- 关于渲染目标栈：
-- LuaSTG Plus、LuaSTG Ex Plus、LuaSTG Sub 通过栈来管理渲染目标的设置，以简化使用

--- 将一个渲染目标加入栈顶
---@param rtname string
---@overload fun(render_target: lstg.RenderTarget)
---@overload fun(render_target: lstg.RenderTarget, depth_stencil_buffer: lstg.DepthStencilBuffer)
function M.PushRenderTarget(rtname)
end

--- 弹出一个渲染目标
function M.PopRenderTarget()
end

--------------------------------------------------------------------------------
--- 画面绘制

--- 在矩形区域内绘制图片
---@param imgname string
---@param left number
---@param right number
---@param bottom number
---@param top number
function M.RenderRect(imgname, left, right, bottom, top)
end

--- [受到 lstg.SetImageScale 影响]  
--- 绘制图片
---@param imgname string
---@param x number
---@param y number
---@param rot number
---@param hscale number
---@param vscale number
---@param z number
---@overload fun(imgname:string, x:number, y:number)
---@overload fun(imgname:string, x:number, y:number, rot:number)
---@overload fun(imgname:string, x:number, y:number, rot:number, scale:number)
---@overload fun(imgname:string, x:number, y:number, rot:number, hscale:number, vscale:number)
function M.Render(imgname, x, y, rot, hscale, vscale, z)
end

--- [LuaSTG Ex Plus 新增]
--- [受到 lstg.SetImageScale 影响]  
--- 绘制图片序列
---@param aniname string
---@param anitimer number
---@param x number
---@param y number
---@param rot number
---@param hscale number
---@param vscale number
---@param z number
---@overload fun(aniname:string, anitimer:number, x:number, y:number)
---@overload fun(aniname:string, anitimer:number, x:number, y:number, rot:number)
---@overload fun(aniname:string, anitimer:number, x:number, y:number, rot:number, scale:number)
---@overload fun(aniname:string, anitimer:number, x:number, y:number, rot:number, hscale:number, vscale:number)
function M.RenderAnimation(aniname, anitimer, x, y, rot, hscale, vscale, z)
end

--- 指定 4 个顶点位置绘制图片
---@param imgname string
---@param x1 number
---@param y1 number
---@param z1 number
---@param x2 number
---@param y2 number
---@param z2 number
---@param x3 number
---@param y3 number
---@param z3 number
---@param x4 number
---@param y4 number
---@param z4 number
function M.Render4V(imgname, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)
end

--- 指定 4 个顶点绘制纹理  
--- 每个顶点的结构为 { x:number, y:number, z:number, u:number, v:number, color:lstg.Color }  
--- uv 坐标以图片左上角为原点，u 轴向右，v 轴向下，单位为像素（不是 0.0 到 1.0）  
---@param texname string
---@param blendmode lstg.BlendMode
---@param v1 { [1]: number, [2]: number, [3]: number, [4]: number, [5]: number, [6]: number | lstg.Color }
---@param v2 { [1]: number, [2]: number, [3]: number, [4]: number, [5]: number, [6]: number | lstg.Color }
---@param v3 { [1]: number, [2]: number, [3]: number, [4]: number, [5]: number, [6]: number | lstg.Color }
---@param v4 { [1]: number, [2]: number, [3]: number, [4]: number, [5]: number, [6]: number | lstg.Color }
function M.RenderTexture(texname, blendmode, v1, v2, v3, v4)
end

--------------------------------------------------------------------------------
--- 文本绘制

-- 关于对齐：
-- 0x00 左对齐 & 上对齐
-- 0x01 水平居中
-- 0x02 右对齐
-- 0x04 垂直居中
-- 0x08 下对齐

-- 关于其他 flag：
-- 0x10 残废的自动换行，建议只用于中文文本，否则英文单词直接寄

--- [受到 lstg.SetImageScale 影响]  
--- 绘制图片字体  
---@param fntname string
---@param text string
---@param x number
---@param y number
---@param scale number
---@param align number
function M.RenderText(fntname, text, x, y, scale, align)
end

--- [受到 lstg.SetImageScale 影响]  
--- 注意，LuaSTG Plus、LuaSTG Ex Plus、LuaSTG Sub 的 RenderTTF 方法，scale 会自动乘以 0.5  
--- 绘制矢量字体  
---@param ttfname string
---@param text string
---@param left number
---@param right number
---@param bottom number
---@param top number
---@param align number
---@param color lstg.Color
---@param scale number
function M.RenderTTF(ttfname, text, left, right, bottom, top, align, color, scale)
end

--------------------------------------------------------------------------------
--- 模型渲染
--- Model Rendering

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.1.0 移除]  
--- [LuaSTG Sub v0.15.0 重新添加]  
--- 指定位置、缩放、欧拉角（角度制）旋转参数，渲染模型  
--- [LuaSTG Ex Plus v?.?.? Add]  
--- [LuaSTG Sub v0.1.0 Remove]  
--- [LuaSTG Sub v0.15.0 Re-add]  
--- Rendering the model with position, scale, Euler angle (degress) rotation  
---@param modname string
---@param x number
---@param y number
---@param z number
---@param roll number
---@param pitch number
---@param yaw number
---@param xscale number
---@param yscale number
---@param zscale number
---@overload fun(modname:string, x:number, y:number, z:number)
---@overload fun(modname:string, x:number, y:number, z:number, roll:number, pitch:number, yaw:number)
function M.RenderModel(modname, x, y, z, roll, pitch, yaw, xscale, yscale, zscale)
end

--------------------------------------------------------------------------------
--- 调试渲染（碰撞体调试）
--- Debug Rendering (Colider Debugging)

--- [LuaSTG Sub v0.1.0 移除]  
--- [LuaSTG Sub v0.16.1 重新添加]  
--- 渲染碰撞判定，通过 F8 开启/关闭  
--- 会渲染 1、2、4、5 共四个碰撞组  
--- [LuaSTG Sub v0.1.0 Remove]  
--- [LuaSTG Sub v0.16.1 Re-add]  
--- Render collision detection, turn on/off with F8  
--- Will render 1, 2, 4, 5 total of four collision groups
function M.DrawCollider()
end

--- [LuaSTG Ex Plus 新增]  
--- [LuaSTG Sub v0.1.0 移除]  
--- [LuaSTG Sub v0.16.1 重新添加]  
--- 用指定颜色渲染指定碰撞组的碰撞体  
--- [LuaSTG Ex Plus v?.?.? Add]  
--- [LuaSTG Sub v0.1.0 Remove]  
--- [LuaSTG Sub v0.16.1 Re-add]  
--- Renders the colliders of the specified collision group with the specified color  
---@param group number
---@param color lstg.Color
function M.RenderGroupCollider(group, color)
end

--------------------------------------------------------------------------------
--- 屏幕后处理（高级功能，高度平台相关）

--- [LuaSTG Sub 更改]  
--- 应用屏幕后处理效果  
--- samplerstate 目前统一填 6  
--- floatbuffer 传入 float4 数组  
--- texparam 则是以 { texname:string, samplerstate:number } 组成的数组  
--- [LuaSTG Sub v0.20.10 更改]  
--- 恢复原来的 API 参数签名  
---@param fxname string
---@param texname string
---@param samplerstate number
---@param blendmode lstg.BlendMode
---@param floatbuffer number[][]
---@overload fun(render_target_name:string, post_effect_name:string, blend_mode:lstg.BlendMode, param:table<string, number|string|lstg.Color>)
function M.PostEffect(fxname, texname, samplerstate, blendmode, floatbuffer, texparam)
end

-- 参考 shader（boss_distortion.hlsl）
local _ = [[

// 引擎参数

SamplerState screen_texture_sampler : register(s4); // RenderTarget 纹理的采样器
Texture2D screen_texture            : register(t4); // RenderTarget 纹理
cbuffer engine_data : register(b1)
{
    float4 screen_texture_size; // 纹理大小
    float4 viewport;            // 视口
};

// 用户传递的参数

cbuffer user_data : register(b0)
{
    float4 center_pos;   // 指定效果的中心坐标
    float4 effect_color; // 指定效果的中心颜色,着色时使用colorburn算法
    float4 effect_param; // 多个参数：effect_size 指定效果的影响大小、effect_arg 变形系数、effect_color_size 颜色的扩散大小、timer 外部计时器
};

#define effect_size       effect_param.x
#define effect_arg        effect_param.y
#define effect_color_size effect_param.z
#define timer             effect_param.w

// 不变量

float PI = 3.14159265f;
float inner = 1.0f; // 边沿缩进
float cb_64 = 64.0f / 255.0f;

// 方法

float2 Distortion(float2 xy, float2 delta, float delta_len)
{
    float k = delta_len / effect_size;
    float p = pow((k - 1.0f), 0.75f);
    float arg = effect_arg * p;
    float2 delta1 = float2(sin(1.75f * 2.0f * PI * delta.x + 0.05f * delta_len + timer / 20.0f), sin(1.75f * 2.0f * PI * delta.y + 0.05f * delta_len + timer / 24.0f)); // 1.75f 此项越高，波纹越“破碎”
    float delta2 = arg * sin(0.005f * 2.0f * PI * delta_len+ timer / 40.0f); // 0.005f 此项越高，波纹越密
    return delta1 * delta2; // delta1：方向向量，delta2：向量长度，即返回像素移动的方向和距离
}

// 主函数

struct PS_Input
{
    float4 sxy : SV_Position;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct PS_Output
{
    float4 col : SV_Target;
};

PS_Output main(PS_Input input)
{
    float2 xy = input.uv * screen_texture_size.xy;  // 屏幕上真实位置
    if (xy.x < viewport.x || xy.x > viewport.z || xy.y < viewport.y || xy.y > viewport.w)
    {
        discard; // 抛弃不需要的像素，防止意外覆盖画面
    }
    float2 uv2 = input.uv;
    float2 delta = xy - center_pos.xy;  // 计算效果中心到纹理采样点的向量
    float delta_len = length(delta);
    delta = normalize(delta);
    if (delta_len <= effect_size)
    {
        float2 distDelta = Distortion(xy, delta, delta_len);
        float2 resultxy = xy + distDelta;
        if (resultxy.x > (viewport.x + inner) && resultxy.x < (viewport.z - inner) && resultxy.y > (viewport.y + inner) && resultxy.y < (viewport.w - inner))
        {
            uv2 += distDelta / screen_texture_size.xy;
        }
        else
        {
            uv2 = input.uv;
        }
    }
    
    float4 tex_color = screen_texture.Sample(screen_texture_sampler, uv2); // 对纹理进行采样
    if (delta_len <= effect_color_size)
    {
        // 扭曲着色
        float k = delta_len / effect_color_size;
        float ak = effect_color.a * pow((1.0f - k), 1.2f);
        float4 processed_color = float4(max(cb_64, effect_color.r), max(cb_64, effect_color.g), max(cb_64, effect_color.b), effect_color.a);
        float4 result_color = tex_color - ((1.0f - tex_color) * (1.0f - processed_color)) / processed_color;
        tex_color.r = ak * result_color.r + (1.0f - ak) * tex_color.r;
        tex_color.g = ak * result_color.g + (1.0f - ak) * tex_color.g;
        tex_color.b = ak * result_color.b + (1.0f - ak) * tex_color.b;
    }
    tex_color.a = 1.0f;
    
    PS_Output output;
    output.col = tex_color;
    return output;
}

]]
-- 参考 shader（texture_overlay.hlsl）：
local _ = [[

// 引擎参数

SamplerState screen_texture_sampler : register(s4); // RenderTarget 纹理的采样器
Texture2D screen_texture            : register(t4); // RenderTarget 纹理
cbuffer engine_data : register(b1)
{
    float4 screen_texture_size; // 纹理大小
    float4 viewport;            // 视口
};

// 用户传递的参数

SamplerState screen_texture_sampler1 : register(s0);
Texture2D screen_texture1            : register(t0);

// 方法

float overlay(float base, float blend)
{
    // 叠加混合模式处理过程
    if (base < 0.5f)
    {
        return 2.0f * base * blend;
    }
    else
    {
        return 1.0f - 2.0f * (1.0f - base) * (1.0f - blend);
    }
}

// 主函数

struct PS_Input
{
    float4 sxy : SV_Position;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct PS_Output
{
    float4 col : SV_Target;
};

PS_Output main(PS_Input input)
{
    float4 color_top = screen_texture.Sample(screen_texture_sampler, input.uv); // 作为顶层
    float4 color_bot = screen_texture1.Sample(screen_texture_sampler1, input.uv); // 作为底层
    float4 color_out;

    color_out.r = overlay(color_bot.r, color_top.r);
    color_out.g = overlay(color_bot.g, color_top.g);
    color_out.b = overlay(color_bot.b, color_top.b);

    color_out = lerp(color_bot, color_out, color_top.a);

    color_out.a = 1.0f - (1.0f - color_bot.a) * (1.0f - color_top.a);
    
    PS_Output output;
    output.col = color_out;
    return output;
}

]]
-- 参考调用：
local _ = [[

lstg.PushRenderTarget("A")
-- 渲染点东西
lstg.PopRenderTarget()

lstg.PushRenderTarget("B")
-- 渲染点东西
lstg.PopRenderTarget()

lstg.PostEffect("texture_overlay", "A", 6, "", {
    -- 不需要传递什么
}, {
    { "B", 6 }, -- 绑定到 t0、s0
})

lstg.PushRenderTarget("C")
-- 渲染点东西
lstg.PopRenderTarget()

lstg.PostEffect(
    "boss_distortion", -- effect 名称
    "C", 6,
    "",
    {
        -- 总共 3 个 float4
        { x1, y1, 0, 0 }, -- centerX, centerY, 剩下的仅用于对齐
        { fxr / 255.0, fxg / 255.0, fxb / 255.0, 125.0 / 255.0 }, -- color，浮点数，0.0 到 1.0，不是 0 到 255
        {
            _boss.aura_alpha * 400 * lstg.scale_3d, -- size
            1500 * _boss.aura_alpha / 128 * lstg.scale_3d, -- arg
            _boss.aura_alpha * 400 * lstg.scale_3d, -- colorsize
            _boss.timer, -- timer
        },
    },
    {} -- 没有纹理和采样器
)

]]

return M
