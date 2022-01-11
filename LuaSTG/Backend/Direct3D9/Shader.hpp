#pragma once

// 坑点：
// - 顶点着色器的输出和像素着色器的输入千万不能用 POSITION[n]，太他妈傻逼了，在 Direct3D9 必须用 COLOR[n]，因为 POSITION[n] 是屏幕坐标，Direct3D11 后有 SV_POSITION 就能用 POSITION[n] 了

namespace LuaSTG::Core {

static char const g_VertexShader[] = R"(

float4x4 view_proj   : register(c0); // [c0, c1, c2, c3]
#if defined(WORLD_MATRIX)
float4x4 world       : register(c4); // [c4, c5, c6, c7]
#endif

struct VS_Input
{
    float3 pos : POSITION0;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct VS_Output
{
    float4 sxy : POSITION;
#if defined(FOG_ENABLE)
    float4 pos : COLOR1;
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

static char const g_PixelShader[] = R"(

#if defined(FOG_ENABLE)
float4   camera_pos  : register(c0);
float4   fog_color   : register(c1);
float4   fog_range   : register(c2);
#endif

sampler texture0 : register(s0);

struct PS_Input
{
    float4 sxy : VPOS;
#if defined(FOG_ENABLE)
    float4 pos : COLOR1;
#endif
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_Output
{
    float4 col : COLOR;
};

PS_Output main(PS_Input input)
{
    // sample texture
    float4 color = tex2D(texture0, input.uv);
    
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
