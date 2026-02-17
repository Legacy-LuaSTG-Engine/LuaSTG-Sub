#pragma once
#include "d3d11/pch.h"

namespace d3d11 {
    class LetterBoxingRenderer {
    public:
        LetterBoxingRenderer();
        LetterBoxingRenderer(const LetterBoxingRenderer&) = delete;
        LetterBoxingRenderer(LetterBoxingRenderer&&) = delete;
        ~LetterBoxingRenderer();

        LetterBoxingRenderer& operator=(const LetterBoxingRenderer&) = delete;
        LetterBoxingRenderer& operator=(LetterBoxingRenderer&&) = delete;

        bool AttachDevice(ID3D11Device* device);
        void DetachDevice();
        bool UpdateTransform(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool stretch = false);
        bool Draw(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool clear_rtv);

    private:
        struct Vertex
        {
            DirectX::XMFLOAT2 pos;
            DirectX::XMFLOAT2 uv;
            inline Vertex() = default;
            inline Vertex(float x, float y, float u, float v) : pos(x, y), uv(u, v) {}
        };
        
        using Index = uint16_t;
        
        struct TransformBuffer
        {
            DirectX::XMFLOAT4X4 mvp;
        };

        bool CreateResource();
        void DestroyResource();

        win32::com_ptr<ID3D11Device> d3d11_device;
        win32::com_ptr<ID3D11DeviceContext> d3d11_device_context;

        win32::com_ptr<ID3D11InputLayout> d3d11_input_layout;
        Vertex vertex_buffer[4]{};
        win32::com_ptr<ID3D11Buffer> d3d11_vertex_buffer;
        win32::com_ptr<ID3D11Buffer> d3d11_index_buffer;
        TransformBuffer transform_buffer{};
        win32::com_ptr<ID3D11Buffer> d3d11_constant_buffer;
        win32::com_ptr<ID3D11VertexShader> d3d11_vertex_shader;
        D3D11_VIEWPORT d3d11_viewport{};
        D3D11_RECT d3d11_scissor_rect{};
        win32::com_ptr<ID3D11RasterizerState> d3d11_rasterizer_state;
        win32::com_ptr<ID3D11SamplerState> d3d11_sampler_state_linear;
        win32::com_ptr<ID3D11SamplerState> d3d11_sampler_state_point;
        win32::com_ptr<ID3D11PixelShader> d3d11_pixel_shader;
        win32::com_ptr<ID3D11DepthStencilState> d3d11_depth_stencil_state;
        win32::com_ptr<ID3D11BlendState> d3d11_blend_state;
    };
}
