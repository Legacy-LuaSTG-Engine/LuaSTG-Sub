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

Texture2D    texture0 : register(t0);
SamplerState sampler0 : register(s0);

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
    // sample texture
    float4 color = texture0.Sample(sampler0, input.uv);
    
    // vertex color blend
#if defined(VERTEX_COLOR_BLEND_MUL)
    color = input.col * color;
#elif defined(VERTEX_COLOR_BLEND_ADD)
    color.rgb += input.col.rgb;
    color.a *= input.col.a;
#elif defined(VERTEX_COLOR_BLEND_ONE)
    color = input.col;
#else // VERTEX_COLOR_BLEND_ZERO
    // color = color;
#endif
    
    // fog color blend
#if defined(FOG_ENABLE)
    float mc_distance = distance(camera_pos.xyz, input.pos.xyz);
    float4 fog_value = fog_color;
    fog_value.a *= color.a;
#if defined(FOG_EXP)
    float fog_factor = clamp(exp(-(mc_distance * fog_range.x)), 0.0f, 1.0f);
    color = lerp(fog_value, color, fog_factor);
#elif defined(FOG_EXP2)
    float fog_factor = clamp(exp(-pow(mc_distance * fog_range.x, 2.0f)), 0.0f, 1.0f);
    color = lerp(fog_value, color, fog_factor);
#else // FOG_LINEAR
    float fog_factor = clamp((mc_distance - fog_range.x) / fog_range.w, 0.0f, 1.0f);
    color = lerp(color, fog_value, fog_factor);
#endif
#endif

#if defined(PREMUL_ALPHA)
    color.rgb *= color.a;
#endif

    // output color
    PS_Output output;
    output.col = color;

    return output;
};

)";

}
