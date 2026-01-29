#include "core/Graphics/Direct3D11/LetterBoxingRenderer.hpp"
#include "core/Logger.hpp"

#define HRNew HRESULT hr = S_OK;
#define HRGet hr
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { Logger::error("Windows API failed: " x); return false; }

namespace core::Graphics::Direct3D11
{
	bool LetterBoxingRenderer::CreateResource()
	{
		assert(d3d11_device);
		assert(d3d11_device_context);

		HRNew;

		/*
			cbuffer TransformBuffer : register(b0)
			{
				float4x4 mvp;
			};

			struct ShaderInput
			{
				float2 pos : POSITION;
				float2 uv  : TEXCOORD;
			};

			struct ShaderOutput
			{
				float4 pos : SV_POSITION;
				float2 uv  : TEXCOORD;
			};

			ShaderOutput main(ShaderInput input)
			{
				ShaderOutput output;
				output.pos = mul(float4(input.pos, 0.0f, 1.0f), mvp);
				output.uv  = input.uv;
				return output;
			}
		*/
		static unsigned char const vertex_shader[] = { 68, 88, 66, 67, 59, 64, 214, 227, 106, 29, 100, 169, 229, 180, 69, 232, 69, 58, 239, 19, 1, 0, 0, 0, 56, 3, 0, 0, 5, 0, 0, 0, 52, 0, 0, 0, 244, 0, 0, 0, 72, 1, 0, 0, 160, 1, 0, 0, 188, 2, 0, 0, 82, 68, 69, 70, 184, 0, 0, 0, 1, 0, 0, 0, 76, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 0, 4, 254, 255, 0, 1, 0, 0, 144, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 84, 114, 97, 110, 115, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 60, 0, 0, 0, 1, 0, 0, 0, 100, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 0, 109, 118, 112, 0, 3, 0, 3, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 105, 99, 114, 111, 115, 111, 102, 116, 32, 40, 82, 41, 32, 72, 76, 83, 76, 32, 83, 104, 97, 100, 101, 114, 32, 67, 111, 109, 112, 105, 108, 101, 114, 32, 49, 48, 46, 49, 0, 73, 83, 71, 78, 76, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 3, 3, 0, 0, 80, 79, 83, 73, 84, 73, 79, 78, 0, 84, 69, 88, 67, 79, 79, 82, 68, 0, 171, 171, 79, 83, 71, 78, 80, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 3, 12, 0, 0, 83, 86, 95, 80, 79, 83, 73, 84, 73, 79, 78, 0, 84, 69, 88, 67, 79, 79, 82, 68, 0, 171, 171, 171, 83, 72, 68, 82, 20, 1, 0, 0, 64, 0, 1, 0, 69, 0, 0, 0, 89, 0, 0, 4, 70, 142, 32, 0, 0, 0, 0, 0, 4, 0, 0, 0, 95, 0, 0, 3, 50, 16, 16, 0, 0, 0, 0, 0, 95, 0, 0, 3, 50, 16, 16, 0, 1, 0, 0, 0, 103, 0, 0, 4, 242, 32, 16, 0, 0, 0, 0, 0, 1, 0, 0, 0, 101, 0, 0, 3, 50, 32, 16, 0, 1, 0, 0, 0, 104, 0, 0, 2, 1, 0, 0, 0, 54, 0, 0, 5, 50, 0, 16, 0, 0, 0, 0, 0, 70, 16, 16, 0, 0, 0, 0, 0, 54, 0, 0, 5, 66, 0, 16, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 0, 128, 63, 16, 0, 0, 8, 18, 32, 16, 0, 0, 0, 0, 0, 70, 2, 16, 0, 0, 0, 0, 0, 70, 131, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 8, 34, 32, 16, 0, 0, 0, 0, 0, 70, 2, 16, 0, 0, 0, 0, 0, 70, 131, 32, 0, 0, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 8, 66, 32, 16, 0, 0, 0, 0, 0, 70, 2, 16, 0, 0, 0, 0, 0, 70, 131, 32, 0, 0, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 8, 130, 32, 16, 0, 0, 0, 0, 0, 70, 2, 16, 0, 0, 0, 0, 0, 70, 131, 32, 0, 0, 0, 0, 0, 3, 0, 0, 0, 54, 0, 0, 5, 50, 32, 16, 0, 1, 0, 0, 0, 70, 16, 16, 0, 1, 0, 0, 0, 62, 0, 0, 1, 83, 84, 65, 84, 116, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		/*
			SamplerState sampler0 : register(s0);
			Texture2D    texture0 : register(t0);

			struct ShaderInput
			{
				float4 pos : SV_POSITION;
				float2 uv  : TEXCOORD;
			};

			struct ShaderOutput
			{
				float4 col : SV_TARGET;
			};

			ShaderOutput main(ShaderInput input)
			{
				ShaderOutput output;
				output.col = texture0.Sample(sampler0, input.uv);
				return output;
			}
		*/
		static unsigned char const pixel_shader[] = { 68, 88, 66, 67, 73, 50, 79, 193, 221, 29, 6, 76, 161, 213, 6, 101, 254, 234, 249, 59, 1, 0, 0, 0, 72, 2, 0, 0, 5, 0, 0, 0, 52, 0, 0, 0, 212, 0, 0, 0, 44, 1, 0, 0, 96, 1, 0, 0, 204, 1, 0, 0, 82, 68, 69, 70, 152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 28, 0, 0, 0, 0, 4, 255, 255, 0, 1, 0, 0, 110, 0, 0, 0, 92, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 101, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 4, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 115, 97, 109, 112, 108, 101, 114, 48, 0, 116, 101, 120, 116, 117, 114, 101, 48, 0, 77, 105, 99, 114, 111, 115, 111, 102, 116, 32, 40, 82, 41, 32, 72, 76, 83, 76, 32, 83, 104, 97, 100, 101, 114, 32, 67, 111, 109, 112, 105, 108, 101, 114, 32, 49, 48, 46, 49, 0, 171, 171, 73, 83, 71, 78, 80, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 3, 3, 0, 0, 83, 86, 95, 80, 79, 83, 73, 84, 73, 79, 78, 0, 84, 69, 88, 67, 79, 79, 82, 68, 0, 171, 171, 171, 79, 83, 71, 78, 44, 0, 0, 0, 1, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 83, 86, 95, 84, 65, 82, 71, 69, 84, 0, 171, 171, 83, 72, 68, 82, 100, 0, 0, 0, 64, 0, 0, 0, 25, 0, 0, 0, 90, 0, 0, 3, 0, 96, 16, 0, 0, 0, 0, 0, 88, 24, 0, 4, 0, 112, 16, 0, 0, 0, 0, 0, 85, 85, 0, 0, 98, 16, 0, 3, 50, 16, 16, 0, 1, 0, 0, 0, 101, 0, 0, 3, 242, 32, 16, 0, 0, 0, 0, 0, 69, 0, 0, 9, 242, 32, 16, 0, 0, 0, 0, 0, 70, 16, 16, 0, 1, 0, 0, 0, 70, 126, 16, 0, 0, 0, 0, 0, 0, 96, 16, 0, 0, 0, 0, 0, 62, 0, 0, 1, 83, 84, 65, 84, 116, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		/* input layout */ {
			D3D11_INPUT_ELEMENT_DESC input_layout_p2f_t2f[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			HRGet = d3d11_device->CreateInputLayout(input_layout_p2f_t2f, 2, vertex_shader, sizeof(vertex_shader), d3d11_input_layout.put());
			HRCheckCallReturnBool("ID3D11Device::CreateInputLayout");
		}

		/* vertex buffer */ {
			D3D11_BUFFER_DESC vertex_buffer_info = {};
			vertex_buffer_info.ByteWidth = 4 * sizeof(Vertex);
			vertex_buffer_info.Usage = D3D11_USAGE_DYNAMIC;
			vertex_buffer_info.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertex_buffer_info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			HRGet = d3d11_device->CreateBuffer(&vertex_buffer_info, NULL, d3d11_vertex_buffer.put());
			HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
		}

		/* index buffer */ {
			D3D11_BUFFER_DESC index_buffer_info = {};
			index_buffer_info.ByteWidth = 6 * sizeof(Index);
			index_buffer_info.Usage = D3D11_USAGE_DEFAULT;
			index_buffer_info.BindFlags = D3D11_BIND_INDEX_BUFFER;
			Index index_buffer_data[6] = { 0, 1, 2, 0, 2, 3 };
			D3D11_SUBRESOURCE_DATA index_buffer_data_info = {};
			index_buffer_data_info.pSysMem = index_buffer_data;
			HRGet = d3d11_device->CreateBuffer(&index_buffer_info, &index_buffer_data_info, d3d11_index_buffer.put());
			HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
		}

		/* constant buffer */ {
			D3D11_BUFFER_DESC constant_buffer_info = {};
			constant_buffer_info.ByteWidth = sizeof(TransformBuffer);
			constant_buffer_info.Usage = D3D11_USAGE_DYNAMIC;
			constant_buffer_info.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constant_buffer_info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			HRGet = d3d11_device->CreateBuffer(&constant_buffer_info, NULL, d3d11_constant_buffer.put());
			HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
		}

		/* vertex shader */ {
			HRGet = d3d11_device->CreateVertexShader(vertex_shader, sizeof(vertex_shader), NULL, d3d11_vertex_shader.put());
			HRCheckCallReturnBool("ID3D11Device::CreateVertexShader");
		}

		/* rasterizer state */ {
			D3D11_RASTERIZER_DESC state_info = {};
			state_info.FillMode = D3D11_FILL_SOLID;
			state_info.CullMode = D3D11_CULL_NONE;
			HRGet = d3d11_device->CreateRasterizerState(&state_info, d3d11_rasterizer_state.put());
			HRCheckCallReturnBool("ID3D11Device::CreateRasterizerState");
		}

		/* sampler state */ {
			D3D11_SAMPLER_DESC state_info = {};
			state_info.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			state_info.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			state_info.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			state_info.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			state_info.MaxAnisotropy = 1;
			state_info.ComparisonFunc = D3D11_COMPARISON_NEVER;
			state_info.MinLOD = -FLT_MAX;
			state_info.MaxLOD = FLT_MAX;
			HRGet = d3d11_device->CreateSamplerState(&state_info, d3d11_sampler_state_linear.put());
			HRCheckCallReturnBool("ID3D11Device::CreateSamplerState");

			state_info.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			HRGet = d3d11_device->CreateSamplerState(&state_info, d3d11_sampler_state_point.put());
			HRCheckCallReturnBool("ID3D11Device::CreateSamplerState");
		}

		/* pixel shader */ {
			HRGet = d3d11_device->CreatePixelShader(pixel_shader, sizeof(pixel_shader), NULL, d3d11_pixel_shader.put());
			HRCheckCallReturnBool("ID3D11Device::CreatePixelShader");
		}

		/* depth stencil state */ {
			D3D11_DEPTH_STENCILOP_DESC state_op_info = {
				.StencilFailOp = D3D11_STENCIL_OP_KEEP,
				.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
				.StencilPassOp = D3D11_STENCIL_OP_KEEP,
				.StencilFunc = D3D11_COMPARISON_ALWAYS,
			};
			D3D11_DEPTH_STENCIL_DESC state_info = {
				.DepthEnable = FALSE,
				.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
				.DepthFunc = D3D11_COMPARISON_LESS,
				.StencilEnable = FALSE,
				.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
				.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
				.FrontFace = state_op_info,
				.BackFace = state_op_info,
			};
			HRGet = d3d11_device->CreateDepthStencilState(&state_info, d3d11_depth_stencil_state.put());
			HRCheckCallReturnBool("ID3D11Device::CreateDepthStencilState");
		}

		/* blend state */ {
			D3D11_RENDER_TARGET_BLEND_DESC blend_state_info = {
				.BlendEnable = FALSE,
				.SrcBlend = D3D11_BLEND_ONE,
				.DestBlend = D3D11_BLEND_ZERO,
				.BlendOp = D3D11_BLEND_OP_ADD,
				.SrcBlendAlpha = D3D11_BLEND_ONE,
				.DestBlendAlpha = D3D11_BLEND_ZERO,
				.BlendOpAlpha = D3D11_BLEND_OP_ADD,
				.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
			};
			D3D11_BLEND_DESC state_info = {
				.AlphaToCoverageEnable = FALSE,
				.IndependentBlendEnable = FALSE,
				.RenderTarget = {
					blend_state_info,
					blend_state_info,
					blend_state_info,
					blend_state_info,
					blend_state_info,
					blend_state_info,
					blend_state_info,
					blend_state_info,
				},
			};
			HRGet = d3d11_device->CreateBlendState(&state_info, d3d11_blend_state.put());
			HRCheckCallReturnBool("ID3D11Device::CreateBlendState");
		}

		return true;
	}
	void LetterBoxingRenderer::DestroyResource()
	{
		d3d11_input_layout.reset();
		d3d11_vertex_buffer.reset();
		d3d11_index_buffer.reset();
		d3d11_constant_buffer.reset();
		d3d11_vertex_shader.reset();
		d3d11_rasterizer_state.reset();
		d3d11_sampler_state_linear.reset();
		d3d11_sampler_state_point.reset();
		d3d11_pixel_shader.reset();
		d3d11_depth_stencil_state.reset();
		d3d11_blend_state.reset();
	}

	inline bool GetTexture2DInfoFromView(ID3D11View* view, D3D11_TEXTURE2D_DESC& info)
	{
		assert(view);
		HRNew;
		win32::com_ptr<ID3D11Resource> resource;
		view->GetResource(resource.put());
		win32::com_ptr<ID3D11Texture2D> texture;
		HRGet = resource->QueryInterface(texture.put());
		HRCheckCallReturnBool("ID3D11Resource::QueryInterface -> ID3D11Texture2D");
		texture->GetDesc(&info);
		return true;
	}

	bool LetterBoxingRenderer::AttachDevice(ID3D11Device* device)
	{
		assert(device);
		d3d11_device = device;
		d3d11_device->GetImmediateContext(d3d11_device_context.put());
		return CreateResource();
	}
	void LetterBoxingRenderer::DetachDevice()
	{
		DestroyResource();
		d3d11_device.reset();
		d3d11_device_context.reset();
	}
	bool LetterBoxingRenderer::UpdateTransform(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool stretch)
	{
		assert(srv);
		assert(rtv);
		assert(d3d11_device_context);
		assert(d3d11_vertex_buffer);
		assert(d3d11_constant_buffer);

		HRNew;

		// info

		D3D11_TEXTURE2D_DESC srv_res_tex_info{};
		if (!GetTexture2DInfoFromView(srv, srv_res_tex_info)) return false;
		D3D11_TEXTURE2D_DESC rtv_res_tex_info = {};
		if (!GetTexture2DInfoFromView(rtv, rtv_res_tex_info)) return false;

		// vertex buffer

		float const window_w = float(rtv_res_tex_info.Width);
		float const window_h = float(rtv_res_tex_info.Height);

		if (stretch)
		{
			// stretch

			float const _ = 0.0f;

			vertex_buffer[0] = { 0.0f + _, window_h, 0.0f, 0.0f };
			vertex_buffer[1] = { window_w, window_h, 1.0f, 0.0f };
			vertex_buffer[2] = { window_w, 0.0f + _, 1.0f, 1.0f };
			vertex_buffer[3] = { 0.0f + _, 0.0f + _, 0.0f, 1.0f };
		}
		else
		{
			// letter boxing

			float const ______ = 0.0f;

			float const canvas_w = float(srv_res_tex_info.Width);
			float const canvas_h = float(srv_res_tex_info.Height);

			float const scale_w = window_w / canvas_w;
			float const scale_h = window_h / canvas_h;
			float const scale = std::min(scale_w, scale_h);

			float const draw_w = canvas_w * scale;
			float const draw_h = canvas_h * scale;
			float const draw_x = (window_w - draw_w) * 0.5f;
			float const draw_y = (window_h - draw_h) * 0.5f;

			vertex_buffer[0] = { draw_x + ______, draw_y + draw_h, 0.0f, 0.0f };
			vertex_buffer[1] = { draw_x + draw_w, draw_y + draw_h, 1.0f, 0.0f };
			vertex_buffer[2] = { draw_x + draw_w, draw_y + ______, 1.0f, 1.0f };
			vertex_buffer[3] = { draw_x + ______, draw_y + ______, 0.0f, 1.0f };
		}
		
		D3D11_MAPPED_SUBRESOURCE vertex_data_range = {};
		HRGet = d3d11_device_context->Map(d3d11_vertex_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_data_range);
		HRCheckCallReturnBool("ID3D11DeviceContext::Map");
		memcpy(vertex_data_range.pData, vertex_buffer, sizeof(vertex_buffer));
		d3d11_device_context->Unmap(d3d11_vertex_buffer.get(), 0);

		// constant buffer

		DirectX::XMStoreFloat4x4(
			&transform_buffer.mvp,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixOrthographicOffCenterLH(
					0.0f, window_w, 0.0f, window_h, 0.0f, 1.0f
				)
			)
		);

		D3D11_MAPPED_SUBRESOURCE constant_data_range = {};
		HRGet = d3d11_device_context->Map(d3d11_constant_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constant_data_range);
		HRCheckCallReturnBool("ID3D11DeviceContext::Map");
		memcpy(constant_data_range.pData, &transform_buffer, sizeof(transform_buffer));
		d3d11_device_context->Unmap(d3d11_constant_buffer.get(), 0);

		// state

		d3d11_viewport = { 0.0f, 0.0f, window_w , window_h ,0.0f, 1.0f };
		d3d11_scissor_rect = { 0, 0, static_cast<LONG>(rtv_res_tex_info.Width), static_cast<LONG>(rtv_res_tex_info.Height) };

		return true;
	}
	bool LetterBoxingRenderer::Draw(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* rtv, bool clear_rtv)
	{
		assert(srv);
		assert(rtv);
		assert(d3d11_device_context);

		D3D11_TEXTURE2D_DESC srv_res_tex_info{};
		if (!GetTexture2DInfoFromView(srv, srv_res_tex_info)) return false;
		D3D11_TEXTURE2D_DESC rtv_res_tex_info = {};
		if (!GetTexture2DInfoFromView(rtv, rtv_res_tex_info)) return false;

		bool const is_width_or_heigth_equal = false
			|| (srv_res_tex_info.Width == rtv_res_tex_info.Width && srv_res_tex_info.Height <= rtv_res_tex_info.Height)
			|| (srv_res_tex_info.Width <= rtv_res_tex_info.Width && srv_res_tex_info.Height == rtv_res_tex_info.Height);
		bool const is_width_or_heigth_not_equal = false
			|| srv_res_tex_info.Width != rtv_res_tex_info.Width
			|| srv_res_tex_info.Height != rtv_res_tex_info.Height;

		if (is_width_or_heigth_equal)
		{
			if (is_width_or_heigth_not_equal && clear_rtv)
			{
				FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				d3d11_device_context->ClearRenderTargetView(rtv, clear_color);
			}
			win32::com_ptr<ID3D11Resource> srv_res;
			win32::com_ptr<ID3D11Resource> rtv_res;
			srv->GetResource(srv_res.put());
			rtv->GetResource(rtv_res.put());
			UINT const dx = (rtv_res_tex_info.Width - srv_res_tex_info.Width) / 2;
			UINT const dy = (rtv_res_tex_info.Height - srv_res_tex_info.Height) / 2;
			d3d11_device_context->CopySubresourceRegion(
				rtv_res.get(), 0, dx, dy, 0,
				srv_res.get(), 0, NULL);
			return true;
		}

		assert(d3d11_input_layout);
		assert(d3d11_vertex_buffer);
		assert(d3d11_index_buffer);

		d3d11_device_context->ClearState();

		d3d11_device_context->IASetInputLayout(d3d11_input_layout.get());
		d3d11_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11Buffer* vertex_buffer_list[1] = { d3d11_vertex_buffer.get() };
		UINT stride_list[1] = { (UINT)sizeof(Vertex) };
		UINT offset_list[1] = { 0 };
		d3d11_device_context->IASetVertexBuffers(0, 1, vertex_buffer_list, stride_list, offset_list);
		d3d11_device_context->IASetIndexBuffer(d3d11_index_buffer.get(), DXGI_FORMAT_R16_UINT, 0);

		assert(d3d11_constant_buffer);
		assert(d3d11_vertex_shader);

		ID3D11Buffer* constant_buffer_list[1] = { d3d11_constant_buffer.get() };
		d3d11_device_context->VSSetConstantBuffers(0, 1, constant_buffer_list);
		d3d11_device_context->VSSetShader(d3d11_vertex_shader.get(), NULL, 0);

		assert(d3d11_rasterizer_state);

		d3d11_device_context->RSSetState(d3d11_rasterizer_state.get());
		d3d11_device_context->RSSetViewports(1, &d3d11_viewport);
		d3d11_device_context->RSSetScissorRects(1, &d3d11_scissor_rect);

		assert(d3d11_sampler_state_linear);
		assert(d3d11_sampler_state_point);
		assert(srv);
		assert(d3d11_pixel_shader);

		ID3D11SamplerState* sampler_state_list[1] = { d3d11_sampler_state_linear.get() };
		d3d11_device_context->PSSetSamplers(0, 1, sampler_state_list);
		ID3D11ShaderResourceView* shader_resource_view_list[1] = { srv };
		d3d11_device_context->PSSetShaderResources(0, 1, shader_resource_view_list);
		d3d11_device_context->PSSetShader(d3d11_pixel_shader.get(), NULL, 0);

		assert(d3d11_depth_stencil_state);
		assert(d3d11_blend_state);
		assert(rtv);

		d3d11_device_context->OMSetDepthStencilState(d3d11_depth_stencil_state.get(), D3D11_DEFAULT_STENCIL_REFERENCE);
		FLOAT const blend_factor[4]{};
		d3d11_device_context->OMSetBlendState(d3d11_blend_state.get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
		ID3D11RenderTargetView* render_target_view_list[1] = { rtv };
		d3d11_device_context->OMSetRenderTargets(1, render_target_view_list, NULL);

		if (clear_rtv)
		{
			FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			d3d11_device_context->ClearRenderTargetView(rtv, clear_color);
		}
		d3d11_device_context->DrawIndexed(6, 0, 0);

		return true;
	}

	LetterBoxingRenderer::LetterBoxingRenderer() = default;
	LetterBoxingRenderer::~LetterBoxingRenderer() { DetachDevice(); }
}
