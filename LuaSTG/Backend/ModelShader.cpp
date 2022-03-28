#include "Model.hpp"
#include "Backend/ModelShader.hpp"

namespace LuaSTG::Core
{
    bool ModelSharedComponent::createShader()
    {
        HRESULT hr = S_OK;

        // built-in: compile shader

        Microsoft::WRL::ComPtr<ID3DBlob> err;
        Microsoft::WRL::ComPtr<ID3DBlob> vs;
        Microsoft::WRL::ComPtr<ID3DBlob> vs_vc;
        Microsoft::WRL::ComPtr<ID3DBlob> ps;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_a;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_nt;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_a_nt;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_vc;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_a_vc;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_nt_vc;
        Microsoft::WRL::ComPtr<ID3DBlob> ps_a_nt_vc;
        UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
        #ifdef _DEBUG
        compile_flags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
        #endif
        #define HR_CHECK_ERROR \
            if (FAILED(hr))\
            {\
                if (err)\
                {\
                    OutputDebugStringA((char*)err->GetBufferPointer());\
                    OutputDebugStringA("\n");\
                }\
                assert(false);\
                return false;\
            }

        std::string_view const& src = built_in_shader;

        hr = D3DCompile(src.data(), src.size(), "model-vs", NULL, NULL, "VS_Main", "vs_4_0", compile_flags, 0, &vs, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-vs-vertex-color", NULL, NULL, "VS_Main_VertexColor", "vs_4_0", compile_flags, 0, &vs_vc, &err);
        HR_CHECK_ERROR;

        hr = D3DCompile(src.data(), src.size(), "model-ps", NULL, NULL, "PS_Main", "ps_4_0", compile_flags, 0, &ps, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-alpha", NULL, NULL, "PS_Main_AlphaMask", "ps_4_0", compile_flags, 0, &ps_a, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-no-texture", NULL, NULL, "PS_Main_NoBaseTexture", "ps_4_0", compile_flags, 0, &ps_nt, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-alpha-no-texture", NULL, NULL, "PS_Main_NoBaseTexture_AlphaMask", "ps_4_0", compile_flags, 0, &ps_a_nt, &err);
        HR_CHECK_ERROR;

        hr = D3DCompile(src.data(), src.size(), "model-ps-vertex-color", NULL, NULL, "PS_Main_VertexColor", "ps_4_0", compile_flags, 0, &ps_vc, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-alpha-vertex-color", NULL, NULL, "PS_Main_AlphaMask_VertexColor", "ps_4_0", compile_flags, 0, &ps_a_vc, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-no-texture-vertex-color", NULL, NULL, "PS_Main_NoBaseTexture_VertexColor", "ps_4_0", compile_flags, 0, &ps_nt_vc, &err);
        HR_CHECK_ERROR;
        hr = D3DCompile(src.data(), src.size(), "model-ps-alpha-no-texture-vertex-color", NULL, NULL, "PS_Main_NoBaseTexture_AlphaMask_VertexColor", "ps_4_0", compile_flags, 0, &ps_a_nt_vc, &err);
        HR_CHECK_ERROR;

        #undef HR_CHECK_ERROR

        // built-in: create shader

        #define HR_CHECK_ERROR \
            if (FAILED(hr))\
            {\
                assert(false);\
                return false;\
            }

        hr = device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &shader_vertex);
        HR_CHECK_ERROR;
        hr = device->CreateVertexShader(vs_vc->GetBufferPointer(), vs_vc->GetBufferSize(), NULL, &shader_vertex_vc);
        HR_CHECK_ERROR;

        hr = device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &shader_pixel);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_a->GetBufferPointer(), ps_a->GetBufferSize(), NULL, &shader_pixel_alpha);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_nt->GetBufferPointer(), ps_nt->GetBufferSize(), NULL, &shader_pixel_nt);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_a_nt->GetBufferPointer(), ps_a_nt->GetBufferSize(), NULL, &shader_pixel_alpha_nt);
        HR_CHECK_ERROR;

        hr = device->CreatePixelShader(ps_vc->GetBufferPointer(), ps_vc->GetBufferSize(), NULL, &shader_pixel_vc);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_a_vc->GetBufferPointer(), ps_a_vc->GetBufferSize(), NULL, &shader_pixel_alpha_vc);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_nt_vc->GetBufferPointer(), ps_nt_vc->GetBufferSize(), NULL, &shader_pixel_nt_vc);
        HR_CHECK_ERROR;
        hr = device->CreatePixelShader(ps_a_nt_vc->GetBufferPointer(), ps_a_nt_vc->GetBufferSize(), NULL, &shader_pixel_alpha_nt_vc);
        HR_CHECK_ERROR;

        #undef HR_CHECK_ERROR

        // built-in: input layout

        D3D11_INPUT_ELEMENT_DESC ia_layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = device->CreateInputLayout(ia_layout, 3, vs->GetBufferPointer(), vs->GetBufferSize(), &input_layout);
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
        hr = device->CreateInputLayout(ia_layout_vc, 4, vs_vc->GetBufferPointer(), vs_vc->GetBufferSize(), &input_layout_vc);
        if (FAILED(hr))
        {
            assert(false);
            return false;
        }

        return true;
    }
}
