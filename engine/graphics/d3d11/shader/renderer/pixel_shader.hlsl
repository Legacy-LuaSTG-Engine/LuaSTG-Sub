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

float4 sampleTexture0(float2 uv) {
#if !defined(VERTEX_COLOR_BLEND_ONE)
    float4 color = texture0.Sample(sampler0, uv);
#if defined(PREMUL_ALPHA)
    if (color.a == 0.0f) {
        color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else {
        color.rgb /= color.a;
    }
#endif
    return color;
#else // VERTEX_COLOR_BLEND_ONE
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
}

float4 applyVertexColor(float4 color, float4 vertex_color) {
#if defined(VERTEX_COLOR_BLEND_MUL)
    return color * vertex_color;
#elif defined(VERTEX_COLOR_BLEND_ADD)
    return float4(min(color.rgb + vertex_color.rgb, float3(1.0f, 1.0f, 1.0f)), color.a * vertex_color.a);
#elif defined(VERTEX_COLOR_BLEND_ONE)
    return vertex_color;
#else // VERTEX_COLOR_BLEND_ZERO
    return color;
#endif
}

float4 applyFogColor(float4 color, float3 pixel_position) {
#if defined(FOG_ENABLE)
    // camera_pos.xyz 和 input.pos.xyz 都是在世界坐标系下的坐标，求得的距离也是基于世界坐标系的
    float d = distance(camera_pos.xyz, pixel_position);
#if defined(FOG_EXP)
    // 指数雾，fog_range.x 是雾密度
    float k = 1.0f - exp(-(d * fog_range.x));
#elif defined(FOG_EXP2)
    // 二次指数雾，fog_range.x 是雾密度
    float k = 1.0f - exp(-pow(d * fog_range.x, 2.0f));
#else // FOG_LINEAR
    // 线性雾，fog_range.x 是雾起始距离，fog_range.y 是雾浓度最大处的距离，fog_range.w 是雾范围（fog_range.y - fog_range.x）
    float k = (d - fog_range.x) / fog_range.w;
#endif
    return lerp(color, float4(fog_color.rgb, fog_color.a * color.a), clamp(k, 0.0f, 1.0f));
#else
    return color;
#endif
}

PS_Output main(PS_Input input)
{
    float4 color = sampleTexture0(input.uv);
#if defined(VERTEX_COLOR_BLEND_ONE)
    if (color.a == 0.0f) {
        discard;
    }
#endif
    float4 vertex_color_applied = applyVertexColor(color, input.col);
    PS_Output output;
#if defined(FOG_ENABLE)
    output.col = applyFogColor(vertex_color_applied, input.pos.xyz);
#else
    output.col = vertex_color_applied;
#endif
    output.col.rgb *= output.col.a;
    return output;
}
