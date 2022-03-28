#pragma once
#include <string_view>

static std::string_view const built_in_shader(R"(
// pipeline data flow

struct VS_INPUT
{
    float3 pos  : POSITION;
    float3 norm : NORMAL;
    float2 uv   : TEXCOORD;
};
struct VS_P3F_N3F_C4F_T2F
{
    float3 pos  : POSITION;
    float3 norm : NORMAL;
    float4 col  : COLOR;
    float2 uv   : TEXCOORD;
};
struct PS_INPUT
{
    float4 pos  : SV_POSITION;
    float4 wpos : POSITION;
    float4 norm : NORMAL;
    float2 uv   : TEXCOORD;
};
struct PS_S4F_P4F_N4F_C4F_T2F
{
    float4 pos  : SV_POSITION;
    float4 wpos : POSITION;
    float4 norm : NORMAL;
    float4 col  : COLOR;
    float2 uv   : TEXCOORD;
};
struct OM_INPUT
{
    float4 col : SV_Target;
};

// vertex stage

cbuffer constantBuffer0 : register(b0)
{
    float4x4 ProjectionMatrix;
};
cbuffer constantBuffer1 : register(b1)
{
    float4x4 LocalWorldMatrix;
    float4x4 NormalLocalWorldMatrix;
};

PS_INPUT VS_Main(VS_INPUT input)
{
    float4 wpos = mul(LocalWorldMatrix, float4(input.pos, 1.0f));
    PS_INPUT output;
    output.pos = mul(ProjectionMatrix, wpos);
    output.wpos = wpos;
    output.norm = mul(NormalLocalWorldMatrix, float4(input.norm, 0.0f)); // no move
    output.uv = input.uv;
    return output;
};

PS_S4F_P4F_N4F_C4F_T2F VS_Main_VertexColor(VS_P3F_N3F_C4F_T2F input)
{
    float4 wpos = mul(LocalWorldMatrix, float4(input.pos, 1.0f));
    PS_S4F_P4F_N4F_C4F_T2F output;
    output.pos = mul(ProjectionMatrix, wpos);
    output.wpos = wpos;
    output.norm = mul(NormalLocalWorldMatrix, float4(input.norm, 0.0f)); // no move
    output.col = input.col;
    output.uv = input.uv;
    return output;
};

// pixel stage

cbuffer cameraInfo : register(b0)
{
    float4 CameraPos;
    float4 CameraLookTo;
};
cbuffer fogInfo : register(b1)
{
    float4 fog_color;
    float4 fog_range;
};
cbuffer alphaCull : register(b2)
{
    float4 base_color;
    float4 alpha;
};
cbuffer lightInfo : register(b3)
{
    float4 ambient;
    float4 sunshine_pos;
    float4 sunshine_dir;
    float4 sunshine_color;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 ApplySimpleLight(float4 norm, float4 wpos, float4 solid_color)
{
    float3 v_normal = normalize(norm.xyz);
    float light_factor = max(0.0f, dot(v_normal, -sunshine_dir.xyz));
    //float3 pixel_to_eye = normalize(CameraPos.xyz - wpos.xyz);
    //float reflact_factor = pow(max(0.0f, dot(reflect(sunshine_dir, v_normal), pixel_to_eye)), 10.0f);
    return float4((ambient.rgb * ambient.a + sunshine_color.rgb * sunshine_color.a * light_factor) * solid_color.rgb, solid_color.a);
}

float4 ApplyFog(float4 wpos, float4 solid_color)
{
    #if defined(FOG_ENABLE)
        // camera_pos.xyz 和 input.pos.xyz 都是在世界坐标系下的坐标，求得的距离也是基于世界坐标系的
        float dist = distance(CameraPos.xyz, wpos.xyz);
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
        return float4(lerp(solid_color.rgb, fog_color.rgb, k), solid_color.a);
    #else
        return solid_color;
    #endif
}

OM_INPUT PS_Main(PS_INPUT input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_AlphaMask(PS_INPUT input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    if (solid_color.a < alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture(PS_INPUT input)
{
    float4 solid_color = base_color;
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_AlphaMask(PS_INPUT input)
{
    float4 solid_color = base_color;
    if (solid_color.a < alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

// 2

OM_INPUT PS_Main_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * input.col * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_AlphaMask_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * input.col * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    if (solid_color.a < alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 solid_color = base_color * input.col;
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_AlphaMask_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 solid_color = base_color * input.col;
    if (solid_color.a < alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

)");
