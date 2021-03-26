#include "Graphic/DearImGuiRenderer.h"
#include <cassert>
#include "imgui.h"
#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <DirectXMath.h>

namespace slow::Graphic
{
    // pre-compile vertex shader (vs_4_0)
    /*
        cbuffer vertexBuffer : register(b0)
        {
            float4x4 mvp;
        };
        
        struct VS_INPUT
        {
            float2 pos : POSITION;
            float4 col : COLOR0;
            float2 uv  : TEXCOORD0;
        };
        
        struct VS_OUTPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR0;
            float2 uv  : TEXCOORD0;
        };
        
        VS_OUTPUT main(VS_INPUT input)
        {
            VS_OUTPUT output;
            output.pos = mul(mvp, float4(input.pos.xy, 0.f, 1.f));
            output.col = input.col;
            output.uv  = input.uv;
            return output;
        }
    */
    const BYTE g_VSBlob[] = {
         68,  88,  66,  67, 252,  64, 
        149, 176,  99,  77,  19, 144, 
        112, 165, 101, 255, 215,  73, 
        202, 136,   1,   0,   0,   0, 
         92,   3,   0,   0,   5,   0, 
          0,   0,  52,   0,   0,   0, 
        244,   0,   0,   0, 100,   1, 
          0,   0, 216,   1,   0,   0, 
        224,   2,   0,   0,  82,  68, 
         69,  70, 184,   0,   0,   0, 
          1,   0,   0,   0,  76,   0, 
          0,   0,   1,   0,   0,   0, 
         28,   0,   0,   0,   0,   4, 
        254, 255,   0,   1,   0,   0, 
        144,   0,   0,   0,  60,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   1,   0, 
          0,   0,   1,   0,   0,   0, 
        118, 101, 114, 116, 101, 120, 
         66, 117, 102, 102, 101, 114, 
          0, 171, 171, 171,  60,   0, 
          0,   0,   1,   0,   0,   0, 
        100,   0,   0,   0,  64,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0, 124,   0, 
          0,   0,   0,   0,   0,   0, 
         64,   0,   0,   0,   2,   0, 
          0,   0, 128,   0,   0,   0, 
          0,   0,   0,   0, 109, 118, 
        112,   0,   3,   0,   3,   0, 
          4,   0,   4,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
         77, 105,  99, 114, 111, 115, 
        111, 102, 116,  32,  40,  82, 
         41,  32,  72,  76,  83,  76, 
         32,  83, 104,  97, 100, 101, 
        114,  32,  67, 111, 109, 112, 
        105, 108, 101, 114,  32,  49, 
         48,  46,  49,   0,  73,  83, 
         71,  78, 104,   0,   0,   0, 
          3,   0,   0,   0,   8,   0, 
          0,   0,  80,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   3,   0,   0,   0, 
          0,   0,   0,   0,   3,   3, 
          0,   0,  89,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   3,   0,   0,   0, 
          1,   0,   0,   0,  15,  15, 
          0,   0,  95,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   3,   0,   0,   0, 
          2,   0,   0,   0,   3,   3, 
          0,   0,  80,  79,  83,  73, 
         84,  73,  79,  78,   0,  67, 
         79,  76,  79,  82,   0,  84, 
         69,  88,  67,  79,  79,  82, 
         68,   0,  79,  83,  71,  78, 
        108,   0,   0,   0,   3,   0, 
          0,   0,   8,   0,   0,   0, 
         80,   0,   0,   0,   0,   0, 
          0,   0,   1,   0,   0,   0, 
          3,   0,   0,   0,   0,   0, 
          0,   0,  15,   0,   0,   0, 
         92,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          3,   0,   0,   0,   1,   0, 
          0,   0,  15,   0,   0,   0, 
         98,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          3,   0,   0,   0,   2,   0, 
          0,   0,   3,  12,   0,   0, 
         83,  86,  95,  80,  79,  83, 
         73,  84,  73,  79,  78,   0, 
         67,  79,  76,  79,  82,   0, 
         84,  69,  88,  67,  79,  79, 
         82,  68,   0, 171,  83,  72, 
         68,  82,   0,   1,   0,   0, 
         64,   0,   1,   0,  64,   0, 
          0,   0,  89,   0,   0,   4, 
         70, 142,  32,   0,   0,   0, 
          0,   0,   4,   0,   0,   0, 
         95,   0,   0,   3,  50,  16, 
         16,   0,   0,   0,   0,   0, 
         95,   0,   0,   3, 242,  16, 
         16,   0,   1,   0,   0,   0, 
         95,   0,   0,   3,  50,  16, 
         16,   0,   2,   0,   0,   0, 
        103,   0,   0,   4, 242,  32, 
         16,   0,   0,   0,   0,   0, 
          1,   0,   0,   0, 101,   0, 
          0,   3, 242,  32,  16,   0, 
          1,   0,   0,   0, 101,   0, 
          0,   3,  50,  32,  16,   0, 
          2,   0,   0,   0, 104,   0, 
          0,   2,   1,   0,   0,   0, 
         56,   0,   0,   8, 242,   0, 
         16,   0,   0,   0,   0,   0, 
         86,  21,  16,   0,   0,   0, 
          0,   0,  70, 142,  32,   0, 
          0,   0,   0,   0,   1,   0, 
          0,   0,  50,   0,   0,  10, 
        242,   0,  16,   0,   0,   0, 
          0,   0,  70, 142,  32,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   6,  16,  16,   0, 
          0,   0,   0,   0,  70,  14, 
         16,   0,   0,   0,   0,   0, 
          0,   0,   0,   8, 242,  32, 
         16,   0,   0,   0,   0,   0, 
         70,  14,  16,   0,   0,   0, 
          0,   0,  70, 142,  32,   0, 
          0,   0,   0,   0,   3,   0, 
          0,   0,  54,   0,   0,   5, 
        242,  32,  16,   0,   1,   0, 
          0,   0,  70,  30,  16,   0, 
          1,   0,   0,   0,  54,   0, 
          0,   5,  50,  32,  16,   0, 
          2,   0,   0,   0,  70,  16, 
         16,   0,   2,   0,   0,   0, 
         62,   0,   0,   1,  83,  84, 
         65,  84, 116,   0,   0,   0, 
          6,   0,   0,   0,   1,   0, 
          0,   0,   0,   0,   0,   0, 
          6,   0,   0,   0,   3,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   1,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   2,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0
    };
    
    // pre-compile pixel shader (ps_4_0)
    /*
        sampler sampler0 : register(s0);
        Texture2D texture0 : register(t0);
        
        struct PS_INPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR0;
            float2 uv  : TEXCOORD0;
        };
        
        struct PS_OUTPUT
        {
            float4 col : SV_Target;
        };
        
        PS_OUTPUT main(PS_INPUT input)
        {
            PS_OUTPUT output;
            output.col = input.col * texture0.Sample(sampler0, input.uv);
            return output;
        }
    */
    const BYTE g_PSBlob[] = {
         68,  88,  66,  67, 205, 241, 
        209,  14, 123,  92, 155, 163, 
         35, 245, 251,  96, 102,  14, 
         90, 181,   1,   0,   0,   0, 
        148,   2,   0,   0,   5,   0, 
          0,   0,  52,   0,   0,   0, 
        212,   0,   0,   0,  72,   1, 
          0,   0, 124,   1,   0,   0, 
         24,   2,   0,   0,  82,  68, 
         69,  70, 152,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   2,   0,   0,   0, 
         28,   0,   0,   0,   0,   4, 
        255, 255,   0,   1,   0,   0, 
        110,   0,   0,   0,  92,   0, 
          0,   0,   3,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   1,   0, 
          0,   0,   1,   0,   0,   0, 
        101,   0,   0,   0,   2,   0, 
          0,   0,   5,   0,   0,   0, 
          4,   0,   0,   0, 255, 255, 
        255, 255,   0,   0,   0,   0, 
          1,   0,   0,   0,  13,   0, 
          0,   0, 115,  97, 109, 112, 
        108, 101, 114,  48,   0, 116, 
        101, 120, 116, 117, 114, 101, 
         48,   0,  77, 105,  99, 114, 
        111, 115, 111, 102, 116,  32, 
         40,  82,  41,  32,  72,  76, 
         83,  76,  32,  83, 104,  97, 
        100, 101, 114,  32,  67, 111, 
        109, 112, 105, 108, 101, 114, 
         32,  49,  48,  46,  49,   0, 
        171, 171,  73,  83,  71,  78, 
        108,   0,   0,   0,   3,   0, 
          0,   0,   8,   0,   0,   0, 
         80,   0,   0,   0,   0,   0, 
          0,   0,   1,   0,   0,   0, 
          3,   0,   0,   0,   0,   0, 
          0,   0,  15,   0,   0,   0, 
         92,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          3,   0,   0,   0,   1,   0, 
          0,   0,  15,  15,   0,   0, 
         98,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          3,   0,   0,   0,   2,   0, 
          0,   0,   3,   3,   0,   0, 
         83,  86,  95,  80,  79,  83, 
         73,  84,  73,  79,  78,   0, 
         67,  79,  76,  79,  82,   0, 
         84,  69,  88,  67,  79,  79, 
         82,  68,   0, 171,  79,  83, 
         71,  78,  44,   0,   0,   0, 
          1,   0,   0,   0,   8,   0, 
          0,   0,  32,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   3,   0,   0,   0, 
          0,   0,   0,   0,  15,   0, 
          0,   0,  83,  86,  95,  84, 
         97, 114, 103, 101, 116,   0, 
        171, 171,  83,  72,  68,  82, 
        148,   0,   0,   0,  64,   0, 
          0,   0,  37,   0,   0,   0, 
         90,   0,   0,   3,   0,  96, 
         16,   0,   0,   0,   0,   0, 
         88,  24,   0,   4,   0, 112, 
         16,   0,   0,   0,   0,   0, 
         85,  85,   0,   0,  98,  16, 
          0,   3, 242,  16,  16,   0, 
          1,   0,   0,   0,  98,  16, 
          0,   3,  50,  16,  16,   0, 
          2,   0,   0,   0, 101,   0, 
          0,   3, 242,  32,  16,   0, 
          0,   0,   0,   0, 104,   0, 
          0,   2,   1,   0,   0,   0, 
         69,   0,   0,   9, 242,   0, 
         16,   0,   0,   0,   0,   0, 
         70,  16,  16,   0,   2,   0, 
          0,   0,  70, 126,  16,   0, 
          0,   0,   0,   0,   0,  96, 
         16,   0,   0,   0,   0,   0, 
         56,   0,   0,   7, 242,  32, 
         16,   0,   0,   0,   0,   0, 
         70,  14,  16,   0,   0,   0, 
          0,   0,  70,  30,  16,   0, 
          1,   0,   0,   0,  62,   0, 
          0,   1,  83,  84,  65,  84, 
        116,   0,   0,   0,   3,   0, 
          0,   0,   1,   0,   0,   0, 
          0,   0,   0,   0,   3,   0, 
          0,   0,   1,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   1,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          1,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0, 
          0,   0,   0,   0,   0,   0
    };
};

namespace slow::Graphic
{
    using Microsoft::WRL::ComPtr;
    
    struct DearImGuiRenderer::Implement
    {
        ComPtr<ID3D11Device>             imDevice;
        ComPtr<ID3D11DeviceContext>      imDeviceContext;
        
        ComPtr<ID3D11InputLayout>        imInputLayout;
        ComPtr<ID3D11Buffer>             imVertexBuffer;
        ComPtr<ID3D11Buffer>             imIndexBuffer;
        size_t                           imVertexBufferSize = 4096u;
        size_t                           imIndexBufferSize  = 8192u;
        
        ComPtr<ID3D11Buffer>             imConstantBuffer;
        ComPtr<ID3D11VertexShader>       imVertexShader;
        
        ComPtr<ID3D11RasterizerState>    imRasterizerState;
        ComPtr<ID3D11SamplerState>       imSamplerState;
        ComPtr<ID3D11ShaderResourceView> imFontAtlas;
        ComPtr<ID3D11PixelShader>        imPixelShader;
        
        ComPtr<ID3D11DepthStencilState>  imDepthStencilState;
        ComPtr<ID3D11BlendState>         imBlendState;
        
        void reset()
        {
            imDevice.Reset();
            imDeviceContext.Reset();
            
            resetPipeline();
        };
        void resetPipeline()
        {
            imInputLayout.Reset();
            imVertexBuffer.Reset();
            imIndexBuffer.Reset();
            imVertexBufferSize = 4096u;
            imIndexBufferSize = 8192u;
            
            imConstantBuffer.Reset();
            imVertexShader.Reset();
            
            imRasterizerState.Reset();
            imSamplerState.Reset();
            imFontAtlas.Reset();
            imPixelShader.Reset();
            
            imDepthStencilState.Reset();
            imBlendState.Reset();
        };
    };
    
    #define self (*implememt)
    
    bool DearImGuiRenderer::createPipeline()
    {
        // check
        if (!self.imDevice)
        {
            return false;
        }
        ID3D11Device* dev_ = self.imDevice.Get();
        HRESULT hr = 0;
        
        // input layout
        const D3D11_INPUT_ELEMENT_DESC layout_[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT  , 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT  , 0,  8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            #ifdef IMGUI_USE_BGRA_PACKED_COLOR
            { "COLOR"   , 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            #else
            { "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            #endif
        };
        hr = dev_->CreateInputLayout(layout_, 3, g_VSBlob, sizeof(g_VSBlob), self.imInputLayout.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // shader
        hr = dev_->CreateVertexShader(g_VSBlob, sizeof(g_VSBlob), NULL, self.imVertexShader.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        hr = dev_->CreatePixelShader(g_PSBlob, sizeof(g_PSBlob), NULL, self.imPixelShader.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // rasterizer
        D3D11_RASTERIZER_DESC rs_;
        ZeroMemory(&rs_, sizeof(D3D11_RASTERIZER_DESC));
        rs_.FillMode = D3D11_FILL_SOLID;
        rs_.CullMode = D3D11_CULL_NONE;
        rs_.DepthClipEnable = TRUE;
        rs_.ScissorEnable = TRUE;
        hr = dev_->CreateRasterizerState(&rs_, self.imRasterizerState.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        D3D11_SAMPLER_DESC samp_;
        ZeroMemory(&samp_, sizeof(D3D11_SAMPLER_DESC));
        samp_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samp_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samp_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samp_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samp_.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        hr = dev_->CreateSamplerState(&samp_, self.imSamplerState.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // depth
        D3D11_DEPTH_STENCIL_DESC ds_;
        ZeroMemory(&ds_, sizeof(D3D11_DEPTH_STENCIL_DESC));
        hr = dev_->CreateDepthStencilState(&ds_, self.imDepthStencilState.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // blend
        D3D11_RENDER_TARGET_BLEND_DESC blend0_;
        ZeroMemory(&blend0_, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
        blend0_.BlendEnable = TRUE;
        blend0_.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend0_.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend0_.BlendOp = D3D11_BLEND_OP_ADD;
        blend0_.SrcBlendAlpha = D3D11_BLEND_ONE;
        blend0_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend0_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend0_.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        D3D11_BLEND_DESC blend_;
        ZeroMemory(&blend_, sizeof(D3D11_BLEND_DESC));
        blend_.RenderTarget[0] = blend0_;
        hr = dev_->CreateBlendState(&blend_, self.imBlendState.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        return true;
    };
    void DearImGuiRenderer::destroyPipeline()
    {
        self.resetPipeline();
    };
    
    bool DearImGuiRenderer::uploadFontAtlas()
    {
        // check
        if (!ImGui::GetCurrentContext())
        {
            return false;
        }
        if (!self.imDevice)
        {
            return false;
        }
        ID3D11Device* dev_ = self.imDevice.Get();
        HRESULT hr = 0;
        
        // pixel data
        ImGuiIO& io = ImGui::GetIO();
        uint8_t* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        // upload
        D3D11_TEXTURE2D_DESC tex_;
        ZeroMemory(&tex_, sizeof(D3D11_TEXTURE2D_DESC));
        tex_.Width = width;
        tex_.Height = height;
        tex_.MipLevels = 1;
        tex_.ArraySize = 1;
        #ifdef IMGUI_USE_BGRA_PACKED_COLOR
        tex_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        #else
        tex_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        #endif
        tex_.SampleDesc.Count = 1;
        tex_.Usage = D3D11_USAGE_DEFAULT;
        tex_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA res_;
        ZeroMemory(&res_, sizeof(D3D11_SUBRESOURCE_DATA));
        res_.pSysMem = pixels;
        res_.SysMemPitch = width * 4;
        ComPtr<ID3D11Texture2D> tex2d_;
        hr = dev_->CreateTexture2D(&tex_, &res_, tex2d_.GetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // view
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_;
        ZeroMemory(&srv_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srv_.Format = tex_.Format;
        srv_.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_.Texture2D.MipLevels = tex_.MipLevels;
        hr = dev_->CreateShaderResourceView(tex2d_.Get(), &srv_, self.imFontAtlas.ReleaseAndGetAddressOf());
        if (hr != S_OK)
        {
            return false;
        }
        
        // setup back-end
        io.Fonts->SetTexID((ImTextureID)self.imFontAtlas.Get());
        
        return true;
    };
    void DearImGuiRenderer::update()
    {
        if (!self.imBlendState)
        {
            createPipeline();
        }
        if (!self.imFontAtlas)
        {
            uploadFontAtlas();
        }
    };
    bool DearImGuiRenderer::uploadDrawData()
    {
        // check
        if (!ImGui::GetCurrentContext())
        {
            return false;
        }
        if (!self.imDevice || !self.imDeviceContext)
        {
            return false;
        }
        ImDrawData* draw_data_ = ImGui::GetDrawData();
        ID3D11Device* dev_ = self.imDevice.Get();
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        
        // create or grow VB
        if (!self.imVertexBuffer || self.imVertexBufferSize < draw_data_->TotalVtxCount)
        {
            while (self.imVertexBufferSize < draw_data_->TotalVtxCount)
            {
                self.imVertexBufferSize *= 2;
            }
            D3D11_BUFFER_DESC desc_;
            ZeroMemory(&desc_, sizeof(D3D11_BUFFER_DESC));
            desc_.ByteWidth = self.imVertexBufferSize * sizeof(ImDrawVert);
            desc_.Usage = D3D11_USAGE_DYNAMIC;
            desc_.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&desc_, NULL, self.imVertexBuffer.ReleaseAndGetAddressOf()))
            {
                return false; // error
            }
        }
        // create or grow IB
        if (!self.imIndexBuffer || self.imIndexBufferSize < draw_data_->TotalIdxCount)
        {
            while (self.imIndexBufferSize < draw_data_->TotalIdxCount)
            {
                self.imIndexBufferSize *= 2;
            }
            D3D11_BUFFER_DESC desc_;
            ZeroMemory(&desc_, sizeof(D3D11_BUFFER_DESC));
            desc_.ByteWidth = self.imIndexBufferSize * sizeof(ImDrawIdx);
            desc_.Usage = D3D11_USAGE_DYNAMIC;
            desc_.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&desc_, NULL, self.imIndexBuffer.ReleaseAndGetAddressOf()))
            {
                return false; // error
            }
        }
        
        // upload VB
        D3D11_MAPPED_SUBRESOURCE vb_ = {};
        if (S_OK == ctx_->Map(self.imVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vb_))
        {
            ImDrawVert* vtx_dst_ = (ImDrawVert*)vb_.pData;
            for (int n = 0; n < draw_data_->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list_ = draw_data_->CmdLists[n];
                CopyMemory(vtx_dst_, cmd_list_->VtxBuffer.Data, cmd_list_->VtxBuffer.Size * sizeof(ImDrawVert));
                vtx_dst_ += cmd_list_->VtxBuffer.Size;
            }
            ctx_->Unmap(self.imVertexBuffer.Get(), 0);
        }
        else
        {
            return false; // error
        }
        // upload IB
        D3D11_MAPPED_SUBRESOURCE ib_ = {};
        if (S_OK == ctx_->Map(self.imIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ib_))
        {
            ImDrawIdx* idx_dst_ = (ImDrawIdx*)ib_.pData;
            for (int n = 0; n < draw_data_->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list_ = draw_data_->CmdLists[n];
                CopyMemory(idx_dst_, cmd_list_->IdxBuffer.Data, cmd_list_->IdxBuffer.Size * sizeof(ImDrawIdx));
                idx_dst_ += cmd_list_->IdxBuffer.Size;
            }
            ctx_->Unmap(self.imIndexBuffer.Get(), 0);
        }
        else
        {
            return false; // error
        }
        
        // create CB
        if (!self.imConstantBuffer)
        {
            D3D11_BUFFER_DESC cbuf_;
            ZeroMemory(&cbuf_, sizeof(D3D11_BUFFER_DESC));
            cbuf_.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
            cbuf_.Usage = D3D11_USAGE_DYNAMIC;
            cbuf_.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbuf_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&cbuf_, NULL, self.imConstantBuffer.ReleaseAndGetAddressOf()))
            {
                return false; // error
            }
        }
        
        // upload constant buffer
        const DirectX::XMMATRIX proj_ =  DirectX::XMMatrixOrthographicOffCenterLH(
            draw_data_->DisplayPos.x,
            draw_data_->DisplayPos.x + draw_data_->DisplaySize.x,
            draw_data_->DisplayPos.y + draw_data_->DisplaySize.y,
            draw_data_->DisplayPos.y,
            0.0f, 1.0f);
        DirectX::XMFLOAT4X4 mvp_;
        DirectX::XMStoreFloat4x4(&mvp_, proj_);
        D3D11_MAPPED_SUBRESOURCE mvpdata_ = {};
        if (S_OK == ctx_->Map(self.imConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mvpdata_))
        {
            CopyMemory(mvpdata_.pData, &mvp_, sizeof(DirectX::XMFLOAT4X4));
            ctx_->Unmap(self.imConstantBuffer.Get(), 0);
        }
        else
        {
            return false; // error
        }
        
        return true;
    };
    void DearImGuiRenderer::setRenderState()
    {
        // check
        if (!ImGui::GetCurrentContext())
        {
            return;
        }
        if (!self.imDeviceContext)
        {
            return;
        }
        ImDrawData* draw_data_ = ImGui::GetDrawData();
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        
        // IA
        ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx_->IASetInputLayout(self.imInputLayout.Get());
        ID3D11Buffer* const buffer_[1] = { self.imVertexBuffer.Get() };
        UINT stride_[1] = { sizeof(ImDrawVert) };
        UINT offset_[1] = { 0 };
        ctx_->IASetVertexBuffers(0, 1, buffer_, stride_, offset_);
        assert(sizeof(ImDrawIdx) == 2 || sizeof(ImDrawIdx) == 4);
        DXGI_FORMAT format_ = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        ctx_->IASetIndexBuffer(self.imIndexBuffer.Get(), format_, 0);
        
        // VS
        ID3D11Buffer* const constant_[1] = { self.imConstantBuffer.Get() };
        ctx_->VSSetConstantBuffers(0, 1, constant_);
        ctx_->VSSetShader(self.imVertexShader.Get(), NULL, 0);
        
        // RS
        const D3D11_VIEWPORT vp_[1] = {
            {
                0.0f, 0.0f,
                draw_data_->DisplaySize.x, draw_data_->DisplaySize.y,
                0.0f, 1.0f
            }
        };
        ctx_->RSSetViewports(1, vp_);
        ctx_->RSSetState(self.imRasterizerState.Get());
        
        // PS
        ID3D11SamplerState* const sampler_[1] = { self.imSamplerState.Get() };
        ctx_->PSSetSamplers(0, 1, sampler_);
        ctx_->PSSetShader(self.imPixelShader.Get(), NULL, 0);
        
        // OM
        ctx_->OMSetDepthStencilState(self.imDepthStencilState.Get(), 0);
        const FLOAT blend_factor_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        ctx_->OMSetBlendState(self.imBlendState.Get(), blend_factor_, 0xFFFFFFFF);
        
        // Other
        ctx_->GSSetShader(NULL, NULL, 0);
        ctx_->HSSetShader(NULL, NULL, 0);
        ctx_->DSSetShader(NULL, NULL, 0);
        ctx_->CSSetShader(NULL, NULL, 0);
    };
    void DearImGuiRenderer::draw()
    {
        // check
        if (!ImGui::GetCurrentContext())
        {
            return;
        }
        if (!self.imFontAtlas)
        {
            return;
        }
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        {
            return;
        }
        
        // upload data
        if (!uploadDrawData())
        {
            return;
        }
        
        // render state
        setRenderState();
        
        // render
        int idx_offset_ = 0;
        int vtx_offset_ = 0;
        ImVec2 clip_pos_ = draw_data->DisplayPos;
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        setRenderState();
                    }
                    else
                    {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                }
                else
                {
                    const D3D11_RECT sr_[1] = {
                        {
                            (LONG)(pcmd->ClipRect.x - clip_pos_.x),
                            (LONG)(pcmd->ClipRect.y - clip_pos_.y),
                            (LONG)(pcmd->ClipRect.z - clip_pos_.x),
                            (LONG)(pcmd->ClipRect.w - clip_pos_.y),
                        }
                    };
                    ctx_->RSSetScissorRects(1, sr_);
                    ID3D11ShaderResourceView* const srv_[1] = { (ID3D11ShaderResourceView*)pcmd->TextureId };
                    ctx_->PSSetShaderResources(0, 1, srv_);
                    ctx_->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + idx_offset_, pcmd->VtxOffset + vtx_offset_);
                }
            }
            idx_offset_ += cmd_list->IdxBuffer.Size;
            vtx_offset_ += cmd_list->VtxBuffer.Size;
        }
    };
    
    bool DearImGuiRenderer::bind(Device& device)
    {
        unbind();
        
        // check
        if (!ImGui::GetCurrentContext())
        {
            return false;
        }
        if (!device.validate())
        {
            return false;
        }
        
        // setup back-end info
        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "slow.Graphic.DearImGuiRenderer (Direct3D11)";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        
        // get device
        
        self.imDevice = (ID3D11Device*)device.getDeviceHandle();
        self.imDeviceContext = (ID3D11DeviceContext*)device.getDeviceContextHandle();
        
        return true;
    };
    void DearImGuiRenderer::unbind()
    {
        self.reset();
        if (ImGui::GetCurrentContext())
        {
            ImGuiIO& io = ImGui::GetIO();
            io.BackendRendererName = "<null>";
            if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
            {
                io.BackendFlags ^= ImGuiBackendFlags_RendererHasVtxOffset;
            }
            io.Fonts->SetTexID(0);
        }
    };
    
    DearImGuiRenderer::DearImGuiRenderer()
    {
        implememt = new Implement;
    };
    DearImGuiRenderer::~DearImGuiRenderer()
    {
        unbind();
        delete implememt;
    };
    DearImGuiRenderer& DearImGuiRenderer::get()
    {
        static DearImGuiRenderer instance;
        return instance;
    };
};
