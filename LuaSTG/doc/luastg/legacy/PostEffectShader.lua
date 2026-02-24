---@diagnostic disable: missing-return, duplicate-set-field

--------------------------------------------------------------------------------
--- example

local example_hlsl = [[
    Texture2D    g_texture : register(t0);
    SamplerState g_texture_sampler : register(s0);
    
    Texture2D    g_render_target : register(t1);
    SamplerState g_render_target_sampler : register(s1);
    
    cbuffer g_buffer : register(b0)
    {
        float2 g_render_target_size;
        float4 g_viewport;
    };
    
    struct PS_Input
    {
        float4 pos : SV_Position;
        float2 uv  : TEXCOORD0;
        float4 col : COLOR0;
    };
    
    struct PS_Output
    {
        float4 col : SV_Target;
    };

    PS_Output main(PS_Input input)
    {
        float2 xy = input.uv * g_render_target_size;
        if (xy.x < g_viewport.x || xy.y < g_viewport.y || xy.x > g_viewport.z || xy.y > g_viewport.w)
        {
            discard;
        }
    
        float4 color_mask = g_texture.Sample(g_texture_sampler, input.uv);
        float mask_value = (color_mask.r + color_mask.g + color_mask.b) / 3.0f;
        float4 color_buffer = g_render_target.Sample(g_render_target_sampler, input.uv);
        float4 color = color_buffer * float4(mask_value, mask_value, mask_value, mask_value);
    
        PS_Output output;
        output.col = color;
        return output;
    }
]]

local function example()
    local lstg = require("lstg")

    -- create

    lstg.CreateRenderTarget("rt:background")
    lstg.CreateRenderTarget("rt:mask")
    local shader = lstg.CreatePostEffectShader("example.hlsl")

    -- render

    lstg.PushRenderTarget("rt:background")
    lstg.RenderClear(lstg.Color(255, 0, 0, 0))
    lstg.PopRenderTarget()

    lstg.PushRenderTarget("rt:mask")
    lstg.RenderClear(lstg.Color(255, 255, 255, 255))
    lstg.PopRenderTarget()

    local w, h = lstg.GetTextureSize("rt:background")
    shader:setFloat2("g_render_target_size", w, h)
    shader:setFloat4("g_viewport",
        --[[ left   ]] 10,
        --[[ top    ]] 10,
        --[[ right  ]] 640 - 10,
        --[[ bottom ]] 480 - 10)
    shader:setTexture("g_render_target", "rt:background")
    shader:setTexture("g_texture", "rt:mask")
end

--------------------------------------------------------------------------------
--- lstg.PostEffectShader

---@class lstg.PostEffectShader
local PostEffectShader = {}

---@param name string
---@param resource_name string
function PostEffectShader:setTexture(name, resource_name)
end

---@param name string
---@param value number
function PostEffectShader:setFloat(name, value)
end

---@param name string
---@param x number
---@param y number
function PostEffectShader:setFloat2(name, x, y)
end

---@param name string
---@param x number
---@param y number
---@param z number
function PostEffectShader:setFloat3(name, x, y, z)
end

---@param name string
---@param x number
---@param y number
---@param z number
---@param w number
function PostEffectShader:setFloat4(name, x, y, z, w)
end

local M = {}

--------------------------------------------------------------------------------
--- create

---@param file_path string
---@return lstg.PostEffectShader
function M.CreatePostEffectShader(file_path)
end

--------------------------------------------------------------------------------
--- draw

---@param post_effect_shader lstg.PostEffectShader
---@param blend lstg.BlendMode
---@overload fun(fxname:string, texname:string, samplerstate:number, blendmode:lstg.BlendMode, floatbuffer:number[][], texparam:string[][])
function M.PostEffect(post_effect_shader, blend)
end

return M
