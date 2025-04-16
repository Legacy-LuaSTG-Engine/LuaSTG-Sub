#pragma once

namespace core::Graphics::Direct3D11
{
	class LetterBoxingRenderer
	{
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
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_device_context;
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> d3d11_input_layout;
		Vertex vertex_buffer[4]{};
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_index_buffer;
		TransformBuffer transform_buffer{};
		Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11_constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> d3d11_vertex_shader;
		D3D11_VIEWPORT d3d11_viewport{};
		D3D11_RECT d3d11_scissor_rect{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> d3d11_rasterizer_state;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> d3d11_sampler_state_linear;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> d3d11_sampler_state_point;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> d3d11_pixel_shader;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> d3d11_depth_stencil_state;
		Microsoft::WRL::ComPtr<ID3D11BlendState> d3d11_blend_state;
	private:
		bool CreateResource();
		void DestroyResource();
	public:
		bool AttachDevice(ID3D11Device* device);
		void DetachDevice();
		bool UpdateTransform(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool stretch = false);
		bool Draw(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool clear_rtv);
	public:
		LetterBoxingRenderer();
		~LetterBoxingRenderer();
	};
}
