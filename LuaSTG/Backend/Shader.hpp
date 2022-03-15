#pragma once

namespace LuaSTG::Core {

    static char const g_VertexShader11[] = R"(

cbuffer view_proj_buffer : register(b0)
{
    float4x4 view_proj;
};
#if defined(WORLD_MATRIX)
cbuffer world_buffer : register(b1)
{
    float4x4 world;
};
#endif

struct VS_Input
{
    float3 pos : POSITION0;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct VS_Output
{
    float4 sxy : SV_POSITION;
#if defined(FOG_ENABLE)
    float4 pos : POSITION0;
#endif
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

VS_Output main(VS_Input input)
{
    float4 pos_world = float4(input.pos, 1.0f);
#if defined(WORLD_MATRIX)
    pos_world = mul(world, pos_world);
#endif

    VS_Output output;
    output.sxy = mul(view_proj, pos_world);
#if defined(FOG_ENABLE)
    output.pos = pos_world;
#endif
    output.uv = input.uv;
    output.col = input.col;

    return output;
};

)";

    static char const g_PixelShader11[] = R"(

#if defined(FOG_ENABLE)
cbuffer camera_data : register(b0)
{
    float4 camera_pos;
};
cbuffer fog_data : register(b1)
{
    float4 fog_color;
    float4 fog_range;
};
#endif

#if !defined(VERTEX_COLOR_BLEND_ONE)
Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);
#endif

float channel_minimum = 1.0f / 255.0f;

struct PS_Input
{
    float4 sxy : SV_POSITION;
#if defined(FOG_ENABLE)
    float4 pos : POSITION0;
#endif
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_Output
{
    float4 col : SV_TARGET;
};

PS_Output main(PS_Input input)
{
    // 对纹理进行采样
    #if !defined(VERTEX_COLOR_BLEND_ONE)
        float4 color = texture0.Sample(sampler0, input.uv);
    #else
        float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    #endif

    // 顶点色混合，顶点色是直通颜色（没有预乘 alpha）；最终的结果是预乘 alpha 的
    #if defined(VERTEX_COLOR_BLEND_MUL)
        color = color * input.col;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            color.rgb *= input.col.a; // 需要少乘以纹理色的 alpha
        #endif
    #elif defined(VERTEX_COLOR_BLEND_ADD)
        #if !defined(PREMUL_ALPHA)
            // 已经是直通颜色
        #else // PREMUL_ALPHA
            // 先解除预乘 alpha
            if (color.a < channel_minimum)
            {
                discard; // 这里原本是完全透明的，应该舍弃
            }
            color.rgb /= color.a;
        #endif
        color.rgb += input.col.rgb;
        color.r = min(color.r, 1.0f);
        color.g = min(color.g, 1.0f);
        color.b = min(color.b, 1.0f);
        color.a *= input.col.a;
        color.rgb *= color.a;
    #elif defined(VERTEX_COLOR_BLEND_ONE)
        color = input.col;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            // 已经乘过 alpha
        #endif
    #else // VERTEX_COLOR_BLEND_ZERO
        // color = color;
        #if !defined(PREMUL_ALPHA)
            color.rgb *= color.a;
        #else // PREMUL_ALPHA
            // 已经乘过 alpha
        #endif
    #endif

    // 雾颜色混合，雾颜色是直通颜色（没有预乘 alpha）；最终的结果是预乘 alpha 的
    #if defined(FOG_ENABLE)
        // camera_pos.xyz 和 input.pos.xyz 都是在世界坐标系下的坐标，求得的距离也是基于世界坐标系的
        float dist = distance(camera_pos.xyz, input.pos.xyz);
        #if defined(FOG_EXP)
            // 指数雾，fog_range.x 是雾密度
            float k = clamp(1.0f - exp(-(dist * fog_range.x)), 0.0f, 1.0f);
        #elif defined(FOG_EXP2)
            // 二次指数雾，fog_range.x 是雾密度
            float k = clamp(1.0f - exp(-pow(dist * fog_range.x, 2.0f)), 0.0f, 1.0f);
        #else // FOG_LINEAR
            // 线性雾，fog_range.x 是雾起始距离，fog_range.y 是雾浓度最大处的距离，fog_range.w 是雾范围（fog_range.y - fog_range.x）
            float k = clamp((dist - fog_range.x) / fog_range.w, 0.0f, 1.0f);
        #endif
        // 基于预乘 alpha 的雾颜色混合，原理请见后面
        float k1 = 1.0f - k;
        float alpha = k1 * color.a + k * color.a * fog_color.a;
        float ka = k1 * (k1 + k * fog_color.a);
        float kb = k * alpha;
        color = float4(ka * color.rgb + kb * fog_color.rgb, alpha);
    #endif

    // 填充输出
    PS_Output output;
    output.col = color;

    return output;
}

)";

}
