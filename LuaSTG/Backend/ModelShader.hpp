#pragma once
#include <string_view>

#define TO_STRING(X) R"()" #X

static std::string_view const built_in_shader(TO_STRING(
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
cbuffer alphaCull : register(b1)
{
    float4 base_color;
    float4 alpha;
};
cbuffer lightInfo : register(b2)
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
    float light_factor = max(0.0f, dot(v_normal, -sunshine_dir));
    //float3 pixel_to_eye = normalize(CameraPos.xyz - wpos.xyz);
    //float reflact_factor = pow(max(0.0f, dot(reflect(sunshine_dir, v_normal), pixel_to_eye)), 10.0f);
    return float4((ambient.rgb * ambient.a + sunshine_color.rgb * sunshine_color.a * light_factor) * solid_color.rgb, solid_color.a);
}

OM_INPUT PS_Main(PS_INPUT input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
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
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture(PS_INPUT input)
{
    float4 solid_color = base_color;
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
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
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 solid_color = base_color * input.col;
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
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
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

));

#undef TO_STRING
