#include "Model.hpp"
#include "Backend/ModelShader.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace LuaSTG::Core
{
    bool ModelSharedComponent::createShader()
    {
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
            hr = gHR = D3DCompile(
                built_in_shader.data(), built_in_shader.size(), name.data(),
                macro, NULL, entry.data(), type ? "ps_4_0" : "vs_4_0", compile_flags, 0, blob, &err);
            if (FAILED(hr))
            {
                assert(false);
                spdlog::error("[luastg] D3DCompile 调用失败");
                spdlog::error("[luastg] 编译着色器 '{}' 失败：{}", name, (char*)err->GetBufferPointer());
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

        auto fxc_ps = [&](std::string_view const& name, std::string_view const& entry, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps[IDX(FogState::MAX_COUNT)]) -> bool
        {
            Microsoft::WRL::ComPtr<ID3DBlob> ps_bc;

            if (!fxc(name, fog_none, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(FogState::Disable)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_line, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(FogState::Linear)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_exp1, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(FogState::Exp)]);
            if (FAILED(hr)) return false;

            if (!fxc(name, fog_exp2, entry, 1, &ps_bc)) return false;
            hr = gHR = device->CreatePixelShader(ps_bc->GetBufferPointer(), ps_bc->GetBufferSize(), NULL, &ps[IDX(FogState::Exp2)]);
            if (FAILED(hr)) return false;

            return true;
        };

        if (!fxc_ps("model-ps"                 , "PS_Main"                        , shader_pixel)) return false;
        if (!fxc_ps("model-ps-alpha"           , "PS_Main_AlphaMask"              , shader_pixel_alpha)) return false;
        if (!fxc_ps("model-ps-no-texture"      , "PS_Main_NoBaseTexture"          , shader_pixel_nt)) return false;
        if (!fxc_ps("model-ps-alpha-no-texture", "PS_Main_NoBaseTexture_AlphaMask", shader_pixel_alpha_nt)) return false;

        if (!fxc_ps("model-ps-vertex-color"                 , "PS_Main_VertexColor"                        , shader_pixel_vc)) return false;
        if (!fxc_ps("model-ps-alpha-vertex-color"           , "PS_Main_AlphaMask_VertexColor"              , shader_pixel_alpha_vc)) return false;
        if (!fxc_ps("model-ps-no-texture-vertex-color"      , "PS_Main_NoBaseTexture_VertexColor"          , shader_pixel_nt_vc)) return false;
        if (!fxc_ps("model-ps-alpha-no-texture-vertex-color", "PS_Main_NoBaseTexture_AlphaMask_VertexColor", shader_pixel_alpha_nt_vc)) return false;

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
