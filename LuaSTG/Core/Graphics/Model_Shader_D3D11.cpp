#include "Core/Graphics/Model_D3D11.hpp"
#include "Platform/RuntimeLoader/Direct3DCompiler.hpp"

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

// inv alpha mask

OM_INPUT PS_Main_InvAlphaMask(PS_INPUT input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    if (solid_color.a >= alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_InvAlphaMask(PS_INPUT input)
{
    float4 solid_color = base_color;
    if (solid_color.a >= alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_InvAlphaMask_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 tex_color = texture0.Sample(sampler0, input.uv);
    float4 solid_color = base_color * input.col * float4(pow(tex_color.rgb, 2.2f), tex_color.a);
    if (solid_color.a >= alpha.x)
    {
        discard;
    }
    solid_color = ApplySimpleLight(input.norm, input.wpos, solid_color);
    solid_color = ApplyFog(input.wpos, solid_color);
    OM_INPUT output;
    output.col = pow(solid_color, 1.0f / 2.2f);
    return output; 
}

OM_INPUT PS_Main_NoBaseTexture_InvAlphaMask_VertexColor(PS_S4F_P4F_N4F_C4F_T2F input)
{
    float4 solid_color = base_color * input.col;
    if (solid_color.a >= alpha.x)
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

#define IDX(x) (size_t)static_cast<uint8_t>(x)

static Platform::RuntimeLoader::Direct3DCompiler g_d3dcompiler_loader;

namespace Core::Graphics
{
    bool ModelSharedComponent_D3D11::createShader()
    {
        auto* device = m_device->GetD3D11Device();
        assert(device);

        HRESULT hr = S_OK;

        // built-in: compile shader

        Microsoft::WRL::ComPtr<ID3DBlob> vs;
        Microsoft::WRL::ComPtr<ID3DBlob> vs_vc;

        auto fxc = [&](std::string_view const& name, D3D_SHADER_MACRO const* macro, std::string_view const& entry, int type, ID3DBlob** blob) -> bool
        {
            Microsoft::WRL::ComPtr<ID3DBlob> err;
            UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
        #ifdef _DEBUG
            compile_flags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
        #endif
            hr = gHR = g_d3dcompiler_loader.Compile(
                built_in_shader.data(), built_in_shader.size(), name.data(),
                macro, NULL, entry.data(), type ? "ps_4_0" : "vs_4_0", compile_flags, 0, blob, &err);
            if (FAILED(hr))
            {
                assert(false);
                spdlog::error("[core] D3DCompile 调用失败");
                spdlog::error("[core] 编译着色器 '{}' 失败：{}", name, (char*)err->GetBufferPointer());
                return false;
            }
            return true;
        };

        if (!fxc("model-vs", NULL, "VS_Main", 0, &vs)) return false;
        if (!fxc("model-vs-vertex-color", NULL, "VS_Main_VertexColor", 0, &vs_vc)) return false;

        hr = gHR = device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &shader_vertex);
        if (FAILED(hr)) return false;
        hr = gHR = device->CreateVertexShader(vs_vc->GetBufferPointer(), vs_vc->GetBufferSize(), NULL, &shader_vertex_vc);
        if (FAILED(hr)) return false;

        const D3D_SHADER_MACRO fog_none[] = {
            { NULL, NULL },
        };
        const D3D_SHADER_MACRO fog_line[] = {
            { "FOG_ENABLE", "1"},
            { "FOG_LINEAR", "1"},
            { NULL, NULL },
        };
        const D3D_SHADER_MACRO fog_exp1[] = {
            { "FOG_ENABLE", "1"},
            { "FOG_EXP", "1"},
            { NULL, NULL },
        };
        const D3D_SHADER_MACRO fog_exp2[] = {
            { "FOG_ENABLE", "1"},
            { "FOG_EXP2", "1"},
            { NULL, NULL },
        };

        auto fxc_ps = [&](std::string_view const& name, std::string_view const& entry, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps[IDX(IRenderer::FogState::MAX_COUNT)]) -> bool
        {
            Microsoft::WRL::ComPtr<ID3DBlob> ps_bc;

            if (!fxc(name, fog_none, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(IRenderer::FogState::Disable)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_line, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(IRenderer::FogState::Linear)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_exp1, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(IRenderer::FogState::Exp)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_exp2, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(IRenderer::FogState::Exp2)]);
            if (FAILED(hr)) return false;

            return true;
        };

        if (!fxc_ps("model-ps", "PS_Main", shader_pixel)) return false;
        if (!fxc_ps("model-ps-alpha", "PS_Main_AlphaMask", shader_pixel_alpha)) return false;
        if (!fxc_ps("model-ps-no-texture", "PS_Main_NoBaseTexture", shader_pixel_nt)) return false;
        if (!fxc_ps("model-ps-alpha-no-texture", "PS_Main_NoBaseTexture_AlphaMask", shader_pixel_alpha_nt)) return false;

        if (!fxc_ps("model-ps-vertex-color", "PS_Main_VertexColor", shader_pixel_vc)) return false;
        if (!fxc_ps("model-ps-alpha-vertex-color", "PS_Main_AlphaMask_VertexColor", shader_pixel_alpha_vc)) return false;
        if (!fxc_ps("model-ps-no-texture-vertex-color", "PS_Main_NoBaseTexture_VertexColor", shader_pixel_nt_vc)) return false;
        if (!fxc_ps("model-ps-alpha-no-texture-vertex-color", "PS_Main_NoBaseTexture_AlphaMask_VertexColor", shader_pixel_alpha_nt_vc)) return false;

        if (!fxc_ps("model-ps-inv-alpha", "PS_Main_InvAlphaMask", shader_pixel_inv_alpha)) return false;
        if (!fxc_ps("model-ps-inv-alpha-no-texture", "PS_Main_NoBaseTexture_InvAlphaMask", shader_pixel_inv_alpha_nt)) return false;
        if (!fxc_ps("model-ps-inv-alpha-vertex-color", "PS_Main_InvAlphaMask_VertexColor", shader_pixel_inv_alpha_vc)) return false;
        if (!fxc_ps("model-ps-inv-alpha-no-texture-vertex-color", "PS_Main_NoBaseTexture_InvAlphaMask_VertexColor", shader_pixel_inv_alpha_nt_vc)) return false;

        // built-in: input layout

        D3D11_INPUT_ELEMENT_DESC ia_layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = gHR = device->CreateInputLayout(ia_layout, 3, vs->GetBufferPointer(), vs->GetBufferSize(), &input_layout);
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC ia_layout_vc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = gHR = device->CreateInputLayout(ia_layout_vc, 4, vs_vc->GetBufferPointer(), vs_vc->GetBufferSize(), &input_layout_vc);
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }
}
