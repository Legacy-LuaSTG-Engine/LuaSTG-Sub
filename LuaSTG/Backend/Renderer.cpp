#include "Core/Renderer.hpp"
#include "Backend/Model.hpp"
#include "Backend/Shader.hpp"
#include "Backend/framework.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace LuaSTG::Core
{
	bool compileShaderMacro(char const* name, void const* data, size_t size, char const* target, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		UINT flag_ = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
		flag_ |= D3DCOMPILE_DEBUG;
		flag_ |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif
		Microsoft::WRL::ComPtr<ID3DBlob> errmsg_;
		HRESULT hr = gHR = D3DCompile(data, size, name, defs, NULL, "main", target, flag_, 0, ppBlob, &errmsg_);
		if (FAILED(hr))
		{
			spdlog::error("[luastg] D3DCompile 调用失败");
			spdlog::error("[luastg] 编译着色器 '{}' 失败：{}", name, (char*)errmsg_->GetBufferPointer());
			return false;
		}
		return true;
	}
	bool compileVertexShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "vs_4_0", defs, ppBlob);
	}
	bool compilePixelShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "ps_4_0", defs, ppBlob);
	}

	inline void TextureID_retain(TextureID& texture)
	{
		if (texture.handle)
			((ID3D11ShaderResourceView*)(texture.handle))->AddRef();
	}
	inline void TextureID_release(TextureID& texture)
	{
		if (texture.handle)
			((ID3D11ShaderResourceView*)(texture.handle))->Release();
		texture.handle = NULL;
	}

	struct RendererStateSet
	{
		TextureID texture;
		Box viewport = {};
		Rect scissor_rect = {};
		float fog_near_or_density = 0.0f;
		float fog_far = 0.0f;
		Color fog_color = {};
		VertexColorBlendState vertex_color_blend_state = VertexColorBlendState::Mul;
		SamplerState sampler_state = SamplerState::LinearClamp;
		FogState fog_state = FogState::Disable;
		TextureAlphaType texture_alpha_type = TextureAlphaType::Normal;
		DepthState depth_state = DepthState::Disable;
		BlendState blend_state = BlendState::Alpha;
	};

	struct CameraStateSet
	{
		Box ortho = {};
		Vector3 eye = {};
		Vector3 lookat = {};
		Vector3 headup = {};
		float fov = 0.0f;
		float aspect = 0.0f;
		float znear = 0.0f;
		float zfar = 0.0f;
		bool is_3D = false;
		
		bool isEqual(Box const& box)
		{
			return !is_3D && ortho == box;
		}
		bool isEqual(Vector3 const& eye_, Vector3 const& lookat_, Vector3 const& headup_, float fov_, float aspect_, float znear_, float zfar_)
		{
			return is_3D
				&& eye == eye_
				&& lookat == lookat_
				&& headup == headup_
				&& fov == fov_
				&& aspect == aspect_
				&& znear == znear_
				&& zfar == zfar_;
		}
	};

	struct VertexIndexBuffer
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
		INT vertex_offset = 0;
		UINT index_offset = 0;
	};
	
	class Renderer::Renderer11
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> _device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _devctx;
	private:
		VertexIndexBuffer _vi_buffer[1];
		size_t _vi_buffer_index = 0;
		const size_t _vi_buffer_count = 1;
		DrawList _draw_list;
		void setVertexIndexBuffer(size_t index = 0xFFFF)
		{
			auto& vi = _vi_buffer[(index == 0xFFFF) ? _vi_buffer_index : index];
			ID3D11Buffer* vbo[1] = { vi.vertex_buffer.Get() };
			UINT stride[1] = { sizeof(DrawVertex2D) };
			UINT offset[1] = { 0 };
			_devctx->IASetVertexBuffers(0, 1, vbo, stride, offset);
			constexpr DXGI_FORMAT format = sizeof(DrawIndex2D) < 4 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			_devctx->IASetIndexBuffer(vi.index_buffer.Get(), format, 0);
		}
		void clearDrawList()
		{
			for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
			{
				TextureID_release(_draw_list.command.data[j_].texture);
			}
			_draw_list.vertex.size = 0;
			_draw_list.index.size = 0;
			_draw_list.command.size = 0;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _vp_matrix_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _world_matrix_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _camera_pos_buffer; // 在 postEffect 的时候被替换了
		Microsoft::WRL::ComPtr<ID3D11Buffer> _fog_data_buffer; // 同时也用于储存 postEffect 的 纹理大小和视口范围
		Microsoft::WRL::ComPtr<ID3D11Buffer> _user_float_buffer; // 在 postEffect 的时候用这个
	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> _input_layout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertex_shader[IDX(FogState::MAX_COUNT)]; // FogState
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixel_shader[IDX(VertexColorBlendState::MAX_COUNT)][IDX(FogState::MAX_COUNT)][IDX(TextureAlphaType::MAX_COUNT)]; // VertexColorBlendState, FogState, TextureAlphaType
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _raster_state;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler_state[IDX(SamplerState::MAX_COUNT)];
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depth_state[IDX(DepthState::MAX_COUNT)];
		Microsoft::WRL::ComPtr<ID3D11BlendState> _blend_state[IDX(BlendState::MAX_COUNT)];
		CameraStateSet _camera_state_set;
		RendererStateSet _state_set;
		bool _state_dirty = false;
	private:
		bool uploadVertexIndexBuffer(bool discard)
		{
			HRESULT hr = 0;
			auto& vi_ = _vi_buffer[_vi_buffer_index];
			const D3D11_MAP map_type_ = discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
			// copy vertex data
			if (_draw_list.vertex.size > 0)
			{
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				hr = gHR = _devctx->Map(vi_.vertex_buffer.Get(), 0, map_type_, 0, &res_);
				if (FAILED(hr))
				{
					spdlog::error("[luastg] ID3D11DeviceContext::Map -> #vertex_buffer[{}] 调用失败，无法上传顶点", _vi_buffer_index);
					return false;
				}
				std::memcpy((DrawVertex2D*)res_.pData + vi_.vertex_offset, _draw_list.vertex.data, _draw_list.vertex.size * sizeof(DrawVertex2D));
				_devctx->Unmap(vi_.vertex_buffer.Get(), 0);
			}
			// copy index data
			if (_draw_list.index.size > 0)
			{
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				hr = gHR = _devctx->Map(vi_.index_buffer.Get(), 0, map_type_, 0, &res_);
				if (FAILED(hr))
				{
					spdlog::error("[luastg] ID3D11DeviceContext::Map -> #index_buffer[{}] 调用失败，无法上传顶点索引", _vi_buffer_index);
					return false;
				}
				std::memcpy((DrawIndex2D*)res_.pData + vi_.index_offset, _draw_list.index.data, _draw_list.index.size * sizeof(DrawIndex2D));
				_devctx->Unmap(vi_.index_buffer.Get(), 0);
			}
			return true;
		}
		bool batchFlush(bool discard = false)
		{
			if (!discard)
			{
				HRESULT hr = 0;
				// upload data
				if ((_draw_list.vertex.capacity - _vi_buffer[_vi_buffer_index].vertex_offset) < _draw_list.vertex.size
					|| (_draw_list.index.capacity - _vi_buffer[_vi_buffer_index].index_offset) < _draw_list.index.size)
				{
					// next  buffer
					_vi_buffer_index = (_vi_buffer_index + 1) % _vi_buffer_count;
					_vi_buffer[_vi_buffer_index].vertex_offset = 0;
					_vi_buffer[_vi_buffer_index].index_offset = 0;
					// discard and copy
					if (!uploadVertexIndexBuffer(true))
					{
						clearDrawList();
						return false;
					}
					// bind buffer
					if (_vi_buffer_count > 1)
					{
						setVertexIndexBuffer(); // need to switch v/i buffers
					}
				}
				else
				{
					// copy no overwrite
					if (!uploadVertexIndexBuffer(false))
					{
						clearDrawList();
						return false;
					}
				}
				// draw
				if (_draw_list.command.size > 0)
				{
					auto& vi_ = _vi_buffer[_vi_buffer_index];
					for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
					{
						const DrawCommand& cmd_ = _draw_list.command.data[j_];
						if (cmd_.vertex_count > 0 && cmd_.index_count > 0)
						{
							ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)cmd_.texture.handle;
							_devctx->PSSetShaderResources(0, 1, &srv);
							_devctx->DrawIndexed(cmd_.index_count, vi_.index_offset, vi_.vertex_offset);
						}
						vi_.vertex_offset += cmd_.vertex_count;
						vi_.index_offset += cmd_.index_count;
					}
				}
				// unbound: solve some debug warning
				ID3D11ShaderResourceView* null_srv = NULL;
				_devctx->PSSetShaderResources(0, 1, &null_srv);
			}
			// clear
			clearDrawList();
			setTexture(_state_set.texture);
			return true;
		}
		void setSamplerState(SamplerState state, UINT index)
		{
			ID3D11SamplerState* samp_ = _sampler_state[IDX(state)].Get();
			_devctx->PSSetSamplers(index, 1, &samp_);
		}
		bool createBuffers()
		{
			HRESULT hr = 0;

			for (auto& vi_ : _vi_buffer)
			{
				{
					D3D11_BUFFER_DESC desc_ = {
						.ByteWidth = sizeof(_draw_list.vertex.data),
						.Usage = D3D11_USAGE_DYNAMIC,
						.BindFlags = D3D11_BIND_VERTEX_BUFFER,
						.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
						.MiscFlags = 0,
						.StructureByteStride = 0,
					};
					hr = gHR = _device->CreateBuffer(&desc_, NULL, &vi_.vertex_buffer);
					if (FAILED(hr))
						return false;
				}

				{
					D3D11_BUFFER_DESC desc_ = {
						.ByteWidth = sizeof(_draw_list.index.data),
						.Usage = D3D11_USAGE_DYNAMIC,
						.BindFlags = D3D11_BIND_INDEX_BUFFER,
						.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
						.MiscFlags = 0,
						.StructureByteStride = 0,
					};
					hr = gHR = _device->CreateBuffer(&desc_, NULL, &vi_.index_buffer);
					if (FAILED(hr))
						return false;
				}
			}

			{
				D3D11_BUFFER_DESC desc_ = {
					.ByteWidth = sizeof(DirectX::XMFLOAT4X4),
					.Usage = D3D11_USAGE_DYNAMIC,
					.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
					.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
					.MiscFlags = 0,
					.StructureByteStride = 0,
				};
				hr = gHR = _device->CreateBuffer(&desc_, NULL, &_vp_matrix_buffer);
				if (FAILED(hr))
					return false;

				hr = gHR = _device->CreateBuffer(&desc_, NULL, &_world_matrix_buffer);
				if (FAILED(hr))
					return false;

				desc_.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4);
				hr = gHR = _device->CreateBuffer(&desc_, NULL, &_camera_pos_buffer);
				if (FAILED(hr))
					return false;

				desc_.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4);
				hr = gHR = _device->CreateBuffer(&desc_, NULL, &_fog_data_buffer);
				if (FAILED(hr))
					return false;

				desc_.ByteWidth = 8 * sizeof(DirectX::XMFLOAT4); // 用户最多可用 8 个 float4
				hr = gHR = _device->CreateBuffer(&desc_, NULL, &_user_float_buffer);
				if (FAILED(hr))
					return false;
			}

			return true;
		}
		bool createStates()
		{
			HRESULT hr = 0;

			{
				D3D11_RASTERIZER_DESC desc_ = {
					.FillMode = D3D11_FILL_SOLID,
					.CullMode = D3D11_CULL_NONE, // 2D 图片精灵可能有负缩放
					.FrontCounterClockwise = FALSE,
					.DepthBias = D3D11_DEFAULT_DEPTH_BIAS,
					.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
					.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
					.DepthClipEnable = TRUE,
					.ScissorEnable = TRUE,
					.MultisampleEnable = FALSE,
					.AntialiasedLineEnable = FALSE,
				};
				hr = gHR = _device->CreateRasterizerState(&desc_, &_raster_state);
				if (FAILED(hr))
					return false;
			}

			{
				D3D11_SAMPLER_DESC black_border_ = {
					.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
					.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
					.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
					.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
					.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS,
					.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY,
					.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
					.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f },
					.MinLOD = -FLT_MAX,
					.MaxLOD = FLT_MAX,
				};
				D3D11_SAMPLER_DESC white_border_ = {
					.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
					.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
					.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
					.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
					.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS,
					.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY,
					.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
					.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
					.MinLOD = -FLT_MAX,
					.MaxLOD = FLT_MAX,
				};

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::PointWrap)]);
				if (FAILED(hr))
					return false;

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::PointClamp)]);
				if (FAILED(hr))
					return false;

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::PointBorderBlack)]);
				if (FAILED(hr))
					return false;

				white_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				white_border_.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				white_border_.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				white_border_.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				hr = gHR = _device->CreateSamplerState(&white_border_, &_sampler_state[IDX(SamplerState::PointBorderWhite)]);
				if (FAILED(hr))
					return false;

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::LinearWrap)]);
				if (FAILED(hr))
					return false;

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::LinearClamp)]);
				if (FAILED(hr))
					return false;

				black_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				black_border_.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				black_border_.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				black_border_.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				hr = gHR = _device->CreateSamplerState(&black_border_, &_sampler_state[IDX(SamplerState::LinearBorderBlack)]);
				if (FAILED(hr))
					return false;

				white_border_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				white_border_.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				white_border_.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				white_border_.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				hr = gHR = _device->CreateSamplerState(&white_border_, &_sampler_state[IDX(SamplerState::LinearBorderWhite)]);
				if (FAILED(hr))
					return false;
			}

			{
				D3D11_DEPTH_STENCIL_DESC desc_ = {
					.DepthEnable = FALSE,
					.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
					.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
					.StencilEnable = FALSE,
					.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
					.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
					.FrontFace = D3D11_DEPTH_STENCILOP_DESC{
						.StencilFailOp = D3D11_STENCIL_OP_KEEP,
						.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
						.StencilPassOp = D3D11_STENCIL_OP_KEEP,
						.StencilFunc = D3D11_COMPARISON_ALWAYS,
					},
					.BackFace = D3D11_DEPTH_STENCILOP_DESC{
						.StencilFailOp = D3D11_STENCIL_OP_KEEP,
						.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
						.StencilPassOp = D3D11_STENCIL_OP_KEEP,
						.StencilFunc = D3D11_COMPARISON_ALWAYS,
					},
				};
				hr = gHR = _device->CreateDepthStencilState(&desc_, &_depth_state[IDX(DepthState::Disable)]);
				if (FAILED(hr))
					return false;

				desc_.DepthEnable = TRUE;
				hr = gHR = _device->CreateDepthStencilState(&desc_, &_depth_state[IDX(DepthState::Enable)]);
				if (FAILED(hr))
					return false;
			}

			{
				D3D11_BLEND_DESC desc_ = {
					.AlphaToCoverageEnable = FALSE,
					.IndependentBlendEnable = FALSE,
					.RenderTarget = {},
				};
				D3D11_RENDER_TARGET_BLEND_DESC blendt_ = {
					.BlendEnable = FALSE,
					.SrcBlend = D3D11_BLEND_ZERO,
					.DestBlend = D3D11_BLEND_ONE,
					.BlendOp = D3D11_BLEND_OP_ADD,
					.SrcBlendAlpha = D3D11_BLEND_ZERO,
					.DestBlendAlpha = D3D11_BLEND_ONE,
					.BlendOpAlpha = D3D11_BLEND_OP_ADD,
					.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
				};
				auto copy_ = [&]() -> void
				{
					for (auto& v : desc_.RenderTarget)
					{
						v = blendt_;
					}
				};
				copy_();

				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Disable)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendEnable = TRUE;

				blendt_.BlendOp = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Alpha)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ZERO;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_ZERO;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::One)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_MIN;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ONE;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_MIN;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Min)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_MAX;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ONE;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_MAX;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Max)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlend = D3D11_BLEND_DEST_COLOR;
				blendt_.DestBlend = D3D11_BLEND_ZERO;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Mul)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Screen)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ONE;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Add)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_SUBTRACT;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ONE;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Sub)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
				blendt_.SrcBlend = D3D11_BLEND_ONE;
				blendt_.DestBlend = D3D11_BLEND_ONE;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
				blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::RevSub)]);
				if (FAILED(hr))
					return false;

				blendt_.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
				blendt_.SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
				blendt_.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
				blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendt_.SrcBlendAlpha = D3D11_BLEND_ZERO;
				blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
				copy_();
				hr = gHR = _device->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Inv)]);
				if (FAILED(hr))
					return false;
			}

			return true;
		}
		bool createShaders()
		{
			HRESULT hr = 0;

			Microsoft::WRL::ComPtr<ID3DBlob> blob_;
			
			/* vertex shader */ {
				Microsoft::WRL::ComPtr<ID3DBlob> vs_blob_;

			#define loadVS(A, DEF)\
				if (!compileVertexShaderMacro11("LuaSTG VertexShader: " #A, g_VertexShader11, sizeof(g_VertexShader11), DEF, &vs_blob_)) return false;\
				hr = gHR = _device->CreateVertexShader(vs_blob_->GetBufferPointer(), vs_blob_->GetBufferSize(), NULL, &_vertex_shader[IDX(A)]);\
				if (FAILED(hr)) return false;

				const D3D_SHADER_MACRO vs_def_fog0_[] = {
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO vs_def_fog1_[] = {
					{ "FOG_ENABLE", "1"},
					{ NULL, NULL },
				};

				loadVS(FogState::Disable, vs_def_fog0_);
				blob_ = vs_blob_; // 用这个保存签名

				loadVS(FogState::Linear, vs_def_fog1_);
				loadVS(FogState::Exp, vs_def_fog1_);
				loadVS(FogState::Exp2, vs_def_fog1_);
			}
			
			/* input layout */ {
				D3D11_INPUT_ELEMENT_DESC layout_[] =
				{
					// DrawVertex2D
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
				hr = gHR = _device->CreateInputLayout(layout_, 3, blob_->GetBufferPointer(), blob_->GetBufferSize(), &_input_layout);
				if (FAILED(hr))
					return false;
			}

			/* pixel shader normal set */ {
				Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;

			#define loadPS(A, B, C, DEF)\
				if (!compilePixelShaderMacro11("LuaSTG PixelShader: " #A ", " #B ", " #C, g_PixelShader11, sizeof(g_PixelShader11), DEF, &ps_blob_)) return false;\
				hr = gHR = _device->CreatePixelShader(ps_blob_->GetBufferPointer(), ps_blob_->GetBufferSize(), NULL, &_pixel_shader[IDX(A)][IDX(B)][IDX(C)]);\
				if (FAILED(hr)) return false;

				const D3D_SHADER_MACRO ps_def_zero_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::Normal, ps_def_zero_);
				loadPS(VertexColorBlendState::One , FogState::Disable, TextureAlphaType::Normal, ps_def_one0_);
				loadPS(VertexColorBlendState::Add , FogState::Disable, TextureAlphaType::Normal, ps_def_add0_);
				loadPS(VertexColorBlendState::Mul , FogState::Disable, TextureAlphaType::Normal, ps_def_mul0_);

				const D3D_SHADER_MACRO ps_def_zero_line_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_line_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_line_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_line_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::Normal, ps_def_zero_line_);
				loadPS(VertexColorBlendState::One , FogState::Linear, TextureAlphaType::Normal, ps_def_one0_line_);
				loadPS(VertexColorBlendState::Add , FogState::Linear, TextureAlphaType::Normal, ps_def_add0_line_);
				loadPS(VertexColorBlendState::Mul , FogState::Linear, TextureAlphaType::Normal, ps_def_mul0_line_);

				const D3D_SHADER_MACRO ps_def_zero_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp0_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::Normal, ps_def_zero_exp0_);
				loadPS(VertexColorBlendState::One , FogState::Exp, TextureAlphaType::Normal, ps_def_one0_exp0_);
				loadPS(VertexColorBlendState::Add , FogState::Exp, TextureAlphaType::Normal, ps_def_add0_exp0_);
				loadPS(VertexColorBlendState::Mul , FogState::Exp, TextureAlphaType::Normal, ps_def_mul0_exp0_);

				const D3D_SHADER_MACRO ps_def_zero_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp2_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::Normal, ps_def_zero_exp2_);
				loadPS(VertexColorBlendState::One , FogState::Exp2, TextureAlphaType::Normal, ps_def_one0_exp2_);
				loadPS(VertexColorBlendState::Add , FogState::Exp2, TextureAlphaType::Normal, ps_def_add0_exp2_);
				loadPS(VertexColorBlendState::Mul , FogState::Exp2, TextureAlphaType::Normal, ps_def_mul0_exp2_);
			}
			
			/* pixel shader premul alpha set */ {
				Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;

				const D3D_SHADER_MACRO ps_def_zero_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_nfog_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_zero_nfog_mula_);
				loadPS(VertexColorBlendState::One , FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_one0_nfog_mula_);
				loadPS(VertexColorBlendState::Add , FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_add0_nfog_mula_);
				loadPS(VertexColorBlendState::Mul , FogState::Disable, TextureAlphaType::PremulAlpha, ps_def_mul0_nfog_mula_);

				const D3D_SHADER_MACRO ps_def_zero_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_line_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_LINEAR", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_zero_line_mula_);
				loadPS(VertexColorBlendState::One , FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_one0_line_mula_);
				loadPS(VertexColorBlendState::Add , FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_add0_line_mula_);
				loadPS(VertexColorBlendState::Mul , FogState::Linear, TextureAlphaType::PremulAlpha, ps_def_mul0_line_mula_);

				const D3D_SHADER_MACRO ps_def_zero_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp0_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_zero_exp0_mula_);
				loadPS(VertexColorBlendState::One , FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_one0_exp0_mula_);
				loadPS(VertexColorBlendState::Add , FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_add0_exp0_mula_);
				loadPS(VertexColorBlendState::Mul , FogState::Exp, TextureAlphaType::PremulAlpha, ps_def_mul0_exp0_mula_);

				const D3D_SHADER_MACRO ps_def_zero_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ZERO", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_one0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ONE", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_add0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_ADD", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};
				const D3D_SHADER_MACRO ps_def_mul0_exp2_mula_[] = {
					{ "VERTEX_COLOR_BLEND_MUL", "1"},
					{ "FOG_ENABLE", "1"},
					{ "FOG_EXP2", "1"},
					{ "PREMUL_ALPHA", "1"},
					{ NULL, NULL },
				};

				loadPS(VertexColorBlendState::Zero, FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_zero_exp2_mula_);
				loadPS(VertexColorBlendState::One , FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_one0_exp2_mula_);
				loadPS(VertexColorBlendState::Add , FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_add0_exp2_mula_);
				loadPS(VertexColorBlendState::Mul , FogState::Exp2, TextureAlphaType::PremulAlpha, ps_def_mul0_exp2_mula_);
			}
			
			return true;
		}
		void initState()
		{
			_state_dirty = true;

			if (!_camera_state_set.is_3D)
			{
				setOrtho(_camera_state_set.ortho);
			}
			else
			{
				setPerspective(_camera_state_set.eye, _camera_state_set.lookat, _camera_state_set.headup, _camera_state_set.fov, _camera_state_set.aspect, _camera_state_set.znear, _camera_state_set.zfar);
			}

			setViewport(_state_set.viewport);
			setScissorRect(_state_set.scissor_rect);

			setVertexColorBlendState(_state_set.vertex_color_blend_state);
			setSamplerState(_state_set.sampler_state);
			setFogState(_state_set.fog_state, _state_set.fog_color, _state_set.fog_near_or_density, _state_set.fog_far);
			setDepthState(_state_set.depth_state);
			setBlendState(_state_set.blend_state);
			setTexture(_state_set.texture);
			setTextureAlphaType(_state_set.texture_alpha_type);

			_state_dirty = false;
		}
	public:
		bool attachDevice(void* dev)
		{
			if (!dev)
			{
				return false;
			}
			spdlog::info("[luastg] 开始创建渲染器");
			_device = (ID3D11Device*)dev;
			_device->GetImmediateContext(&_devctx);
			if (!createBuffers())
			{
				spdlog::error("[luastg] 无法创建渲染器所需的顶点、索引缓冲区和着色器常量缓冲区");
				return false;
			}
			if (!createStates())
			{
				spdlog::error("[luastg] 无法创建渲染器所需的渲染状态");
				return false;
			}
			if (!createShaders())
			{
				spdlog::error("[luastg] 无法创建渲染器所需的内置着色器");
				return false;
			}
			spdlog::info("[luastg] 已创建渲染器");

			if (model_shared_)
			{
				spdlog::info("[luastg] 创建模型渲染器共享组件");
				if (!model_shared_->attachDevice((ID3D11Device*)dev))
				{
					spdlog::error("[luastg] 无法创建模型渲染器共享组件");
					return false;
				}
				spdlog::info("[luastg] 已创建模型渲染器共享组件");
			}

			return true;
		}
		void detachDevice()
		{
			batchFlush(true);

			model_shared_ = nullptr;

			TextureID_release(_state_set.texture);

			_device.Reset();
			_devctx.Reset();

			for (auto& v : _vi_buffer)
			{
				v.vertex_buffer.Reset();
				v.index_buffer.Reset();
				v.vertex_offset = 0;
				v.index_offset = 0;
			}
			_vi_buffer_index = 0;

			_vp_matrix_buffer.Reset();
			_world_matrix_buffer.Reset();
			_camera_pos_buffer.Reset();
			_fog_data_buffer.Reset();
			_user_float_buffer.Reset();

			_input_layout.Reset();
			for (auto& v : _vertex_shader)
			{
				v.Reset();
			}
			for (auto& i : _pixel_shader)
			{
				for (auto& j : i)
				{
					for (auto& v : j)
					{
						v.Reset();
					}
				}
			}
			_raster_state.Reset();
			for (auto& v : _sampler_state)
			{
				v.Reset();
			}
			for (auto& v : _depth_state)
			{
				v.Reset();
			}
			for (auto& v : _blend_state)
			{
				v.Reset();
			}
			spdlog::info("[luastg] 已关闭渲染器");
		}

		bool beginScene()
		{
			// [IA Stage]

			_devctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			_devctx->IASetInputLayout(_input_layout.Get());
			setVertexIndexBuffer();

			// [VS State]

			ID3D11Buffer* mats[2] = {
				_vp_matrix_buffer.Get(),
				_world_matrix_buffer.Get(),
			};
			_devctx->VSSetConstantBuffers(0, 2, mats);

			// [RS Stage]

			_devctx->RSSetState(_raster_state.Get());

			// [PS State]

			ID3D11Buffer* psdata[2] = {
				_camera_pos_buffer.Get(),
				_fog_data_buffer.Get(),
			};
			_devctx->PSSetConstantBuffers(0, 2, psdata);

			// [OM Stage]

			// [Internal State]

			initState();

			return true;
		}
		bool endScene()
		{
			if (!batchFlush())
				return false;
			TextureID_release(_state_set.texture);
			return true;
		}

		void clearRenderTarget(Color const& color)
		{
			batchFlush();
			ID3D11RenderTargetView* rtv = NULL;
			_devctx->OMGetRenderTargets(1, &rtv, NULL);
			if (rtv)
			{
				FLOAT const clear_color[4] = {
					(float)color.r / 255.0f,
					(float)color.g / 255.0f,
					(float)color.b / 255.0f,
					(float)color.a / 255.0f,
				};
				_devctx->ClearRenderTargetView(rtv, clear_color);
				rtv->Release();
			}
		}
		void clearDepthBuffer(float zvalue)
		{
			batchFlush();
			ID3D11DepthStencilView* dsv = NULL;
			_devctx->OMGetRenderTargets(1, NULL, &dsv);
			if (dsv)
			{
				_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, zvalue, D3D11_DEFAULT_STENCIL_REFERENCE);
				dsv->Release();
			}
		}

		void setOrtho(Box const& box)
		{
			if (_state_dirty || !_camera_state_set.isEqual(box))
			{
				batchFlush();
				_camera_state_set.ortho = box;
				_camera_state_set.is_3D = false;
				DirectX::XMFLOAT4X4 f4x4;
				DirectX::XMStoreFloat4x4(&f4x4, DirectX::XMMatrixOrthographicOffCenterLH(box.left, box.right, box.bottom, box.top, box.front, box.back));
				/* upload vp matrix */ {
					D3D11_MAPPED_SUBRESOURCE res_ = {};
					HRESULT hr = gHR = _devctx->Map(_vp_matrix_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
					if (SUCCEEDED(hr))
					{
						std::memcpy(res_.pData, &f4x4, sizeof(f4x4));
						_devctx->Unmap(_vp_matrix_buffer.Get(), 0);
					}
					else
					{
						spdlog::error("[luastg] ID3D11DeviceContext::Map -> #view_projection_matrix_buffer 调用失败，无法上传摄像机变换矩阵");
					}
				}
			}
		}
		void setPerspective(Vector3 const& eye, Vector3 const& lookat, Vector3 const& headup, float fov, float aspect, float znear, float zfar)
		{
			if (_state_dirty || !_camera_state_set.isEqual(eye, lookat, headup, fov, aspect, znear, zfar))
			{
				batchFlush();
				_camera_state_set.eye = eye;
				_camera_state_set.lookat = lookat;
				_camera_state_set.headup = headup;
				_camera_state_set.fov = fov;
				_camera_state_set.aspect = aspect;
				_camera_state_set.znear = znear;
				_camera_state_set.zfar = zfar;
				_camera_state_set.is_3D = true;
				DirectX::XMFLOAT3 const eyef3(eye.x, eye.y, eye.z);
				DirectX::XMFLOAT3 const lookatf3(lookat.x, lookat.y, lookat.z);
				DirectX::XMFLOAT3 const headupf3(headup.x, headup.y, headup.z);
				DirectX::XMFLOAT4X4 f4x4;
				DirectX::XMStoreFloat4x4(&f4x4,
					DirectX::XMMatrixMultiply(
						DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eyef3), DirectX::XMLoadFloat3(&lookatf3), DirectX::XMLoadFloat3(&headupf3)),
						DirectX::XMMatrixPerspectiveFovLH(fov, aspect, znear, zfar)));
				float const camera_pos[8] = {
					eye.x, eye.y, eye.z, 0.0f,
					lookatf3.x - eyef3.x, lookatf3.y - eyef3.y, lookatf3.z - eyef3.z, 0.0f,
				};
				/* upload vp matrix */ {
					D3D11_MAPPED_SUBRESOURCE res_ = {};
					HRESULT hr = gHR = _devctx->Map(_vp_matrix_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
					if (SUCCEEDED(hr))
					{
						std::memcpy(res_.pData, &f4x4, sizeof(f4x4));
						_devctx->Unmap(_vp_matrix_buffer.Get(), 0);
					}
					else
					{
						spdlog::error("[luastg] ID3D11DeviceContext::Map -> #view_projection_matrix_buffer 调用失败，无法上传摄像机变换矩阵");
					}
				}
				/* upload camera pos */ {
					D3D11_MAPPED_SUBRESOURCE res_ = {};
					HRESULT hr = gHR = _devctx->Map(_camera_pos_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
					if (SUCCEEDED(hr))
					{
						std::memcpy(res_.pData, camera_pos, sizeof(camera_pos));
						_devctx->Unmap(_camera_pos_buffer.Get(), 0);
					}
					else
					{
						spdlog::error("[luastg] ID3D11DeviceContext::Map -> #camera_position_buffer 调用失败，无法上传摄像机位置");
					}
				}
			}
		}

		void setViewport(Box const& box)
		{
			if (_state_dirty || _state_set.viewport != box)
			{
				batchFlush();
				_state_set.viewport = box;
				D3D11_VIEWPORT const vp = {
					.TopLeftX = box.left,
					.TopLeftY = box.top,
					.Width = box.right - box.left,
					.Height = box.bottom - box.top,
					.MinDepth = box.front,
					.MaxDepth = box.back,
				};
				_devctx->RSSetViewports(1, &vp);
			}
		}
		void setScissorRect(Rect const& rect)
		{
			if (_state_dirty || _state_set.scissor_rect != rect)
			{
				batchFlush();
				_state_set.scissor_rect = rect;
				D3D11_RECT const rc = {
					.left = (LONG)rect.left,
					.top = (LONG)rect.top,
					.right = (LONG)rect.right,
					.bottom = (LONG)rect.bottom,
				};
				_devctx->RSSetScissorRects(1, &rc);
			}
		}
		void setViewportAndScissorRect()
		{
			_state_dirty = true;
			setViewport(_state_set.viewport);
			setScissorRect(_state_set.scissor_rect);
			_state_dirty = false;
		}
		
		void setVertexColorBlendState(VertexColorBlendState state)
		{
			if (_state_dirty || _state_set.vertex_color_blend_state != state)
			{
				batchFlush();
				_state_set.vertex_color_blend_state = state;
				_devctx->PSSetShader(_pixel_shader[IDX(state)][IDX(_state_set.fog_state)][IDX(_state_set.texture_alpha_type)].Get(), NULL, 0);
			}
		}
		void setSamplerState(SamplerState state)
		{
			if (_state_dirty || _state_set.sampler_state != state)
			{
				batchFlush();
				_state_set.sampler_state = state;
				setSamplerState(state, 0);
			}
		}
		void setFogState(FogState state, Color const& color, float density_or_znear, float zfar)
		{
			if (_state_dirty || _state_set.fog_state != state || _state_set.fog_color != color || _state_set.fog_near_or_density != density_or_znear || _state_set.fog_far != zfar)
			{
				batchFlush();
				_state_set.fog_state = state;
				_state_set.fog_color = color;
				_state_set.fog_near_or_density = density_or_znear;
				_state_set.fog_far = zfar;
				_devctx->VSSetShader(_vertex_shader[IDX(state)].Get(), NULL, 0);
				float const fog_color_and_range[8] = {
					(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f,
					density_or_znear, zfar, 0.0f, zfar - density_or_znear,
				};
				/* upload */ {
					D3D11_MAPPED_SUBRESOURCE res_ = {};
					HRESULT hr = gHR = _devctx->Map(_fog_data_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
					if (SUCCEEDED(hr))
					{
						std::memcpy(res_.pData, fog_color_and_range, sizeof(fog_color_and_range));
						_devctx->Unmap(_fog_data_buffer.Get(), 0);
					}
					else
					{
						spdlog::error("[luastg] ID3D11DeviceContext::Map -> #fog_data_buffer 调用失败，无法上传雾颜色、范围和密度信息");
					}
				}
				_devctx->PSSetShader(_pixel_shader[IDX(_state_set.vertex_color_blend_state)][IDX(state)][IDX(_state_set.texture_alpha_type)].Get(), NULL, 0);
			}
		}
		void setDepthState(DepthState state)
		{
			if (_state_dirty || _state_set.depth_state != state)
			{
				batchFlush();
				_state_set.depth_state = state;
				_devctx->OMSetDepthStencilState(_depth_state[IDX(state)].Get(), D3D11_DEFAULT_STENCIL_REFERENCE);
			}
		}
		void setBlendState(BlendState state)
		{
			if (_state_dirty || _state_set.blend_state != state)
			{
				batchFlush();
				_state_set.blend_state = state;
				FLOAT const factor[4] = {};
				_devctx->OMSetBlendState(_blend_state[IDX(state)].Get(), factor, D3D11_DEFAULT_SAMPLE_MASK);
			}
		}
		void setTexture(TextureID texture)
		{
			if (_draw_list.command.size > 0 && _draw_list.command.data[_draw_list.command.size - 1].texture == texture)
			{
				// no op
			}
			else
			{
				// new command
				if ((_draw_list.command.capacity - _draw_list.command.size) < 1)
				{
					batchFlush(); // no space
				}
				_draw_list.command.size += 1;
				DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
				cmd_.texture = texture;
				cmd_.vertex_count = 0;
				cmd_.index_count = 0;
				TextureID_retain(cmd_.texture);
			}
			if (_state_set.texture != texture)
			{
				TextureID_release(_state_set.texture);
				_state_set.texture = texture;
				TextureID_retain(_state_set.texture);
			}
		}
		void setTextureAlphaType(TextureAlphaType state)
		{
			if (_state_dirty || _state_set.texture_alpha_type != state)
			{
				batchFlush();
				_state_set.texture_alpha_type = state;
				_devctx->PSSetShader(_pixel_shader[IDX(_state_set.vertex_color_blend_state)][IDX(_state_set.fog_state)][IDX(state)].Get(), NULL, 0);
			}
		}

		bool flush()
		{
			return batchFlush();
		}
		void drawTriangle(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 3 || (_draw_list.index.capacity - _draw_list.index.size) < 3)
			{
				batchFlush();
			}
			assert(_draw_list.command.size > 0);
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
			DrawVertex2D* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
			vbuf_[0] = v1;
			vbuf_[1] = v2;
			vbuf_[2] = v3;
			_draw_list.vertex.size += 3;
			DrawIndex2D* ibuf_ = _draw_list.index.data + _draw_list.index.size;
			ibuf_[0] = cmd_.vertex_count;
			ibuf_[1] = cmd_.vertex_count + 1;
			ibuf_[2] = cmd_.vertex_count + 2;
			_draw_list.index.size += 3;
			cmd_.vertex_count += 3;
			cmd_.index_count += 3;
		}
		void drawTriangle(DrawVertex2D const* pvert)
		{
			drawTriangle(pvert[0], pvert[1], pvert[2]);
		}
		void drawQuad(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3, DrawVertex2D const& v4)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 4 || (_draw_list.index.capacity - _draw_list.index.size) < 6)
			{
				batchFlush();
			}
			assert(_draw_list.command.size > 0);
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
			DrawVertex2D* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
			vbuf_[0] = v1;
			vbuf_[1] = v2;
			vbuf_[2] = v3;
			vbuf_[3] = v4;
			_draw_list.vertex.size += 4;
			DrawIndex2D* ibuf_ = _draw_list.index.data + _draw_list.index.size;
			ibuf_[0] = cmd_.vertex_count;
			ibuf_[1] = cmd_.vertex_count + 1;
			ibuf_[2] = cmd_.vertex_count + 2;
			ibuf_[3] = cmd_.vertex_count;
			ibuf_[4] = cmd_.vertex_count + 2;
			ibuf_[5] = cmd_.vertex_count + 3;
			_draw_list.index.size += 6;
			cmd_.vertex_count += 4;
			cmd_.index_count += 6;
		}
		void drawQuad(DrawVertex2D const* pvert)
		{
			drawQuad(pvert[0], pvert[1], pvert[2], pvert[3]);
		}
		void drawRaw(DrawVertex2D const* pvert, uint16_t nvert, DrawIndex2D const* pidx, uint16_t nidx)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < nvert || (_draw_list.index.capacity - _draw_list.index.size) < nidx)
			{
				batchFlush();
			}

			assert(_draw_list.command.size > 0);
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];

			DrawVertex2D* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
			std::memcpy(vbuf_, pvert, nvert * sizeof(DrawVertex2D));
			_draw_list.vertex.size += nvert;

			DrawIndex2D* ibuf_ = _draw_list.index.data + _draw_list.index.size;
			for (size_t idx_ = 0; idx_ < nidx; idx_ += 1)
			{
				ibuf_[idx_] = cmd_.vertex_count + pidx[idx_];
			}
			_draw_list.index.size += nidx;

			cmd_.vertex_count += nvert;
			cmd_.index_count += nidx;
		}
		void drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex2D** ppvert, DrawIndex2D** ppidx, uint16_t* idxoffset)
		{
			if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < nvert || (_draw_list.index.capacity - _draw_list.index.size) < nidx)
			{
				batchFlush();
			}

			assert(_draw_list.command.size > 0);
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];

			*ppvert = _draw_list.vertex.data + _draw_list.vertex.size;
			_draw_list.vertex.size += nvert;

			*ppidx = _draw_list.index.data + _draw_list.index.size;
			_draw_list.index.size += nidx;

			*idxoffset = cmd_.vertex_count; // 输出顶点索引偏移
			cmd_.vertex_count += nvert;
			cmd_.index_count += nidx;
		}

		ShaderID createPostEffectShader(char const* name, void const* data, size_t size)
		{
			Microsoft::WRL::ComPtr<ID3DBlob> ps_blob_;
			if (!compilePixelShaderMacro11(name, data, size, NULL, &ps_blob_))
				return ShaderID();
			Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_;
			if (FAILED(_device->CreatePixelShader(ps_blob_->GetBufferPointer(), ps_blob_->GetBufferSize(), NULL, &ps_)))
				return ShaderID();
			return ShaderID(ps_.Detach());
		}
		void destroyPostEffectShader(ShaderID& ps)
		{
			if (ps.handle)
			{
				((ID3D11PixelShader*)ps.handle)->Release();
				ps.handle = nullptr;
			}
		}
		void postEffect(ShaderID const& ps, TextureID const& rt, SamplerState rtsv, Vector4 const* cv, size_t cv_n, TextureID const* tv, SamplerState const* sv, size_t tv_sv_n, BlendState blend)
		{
			batchFlush();
			
			float sw_ = 0.0f;
			float sh_ = 0.0f;
			/* get current rendertarget size */ {
				ID3D11RenderTargetView* rtv_ = NULL;
				_devctx->OMGetRenderTargets(1, &rtv_, NULL);
				if (rtv_)
				{
					Microsoft::WRL::ComPtr<ID3D11Resource> res_;
					rtv_->GetResource(&res_);
					Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d_;
					HRESULT hr = gHR = res_.As(&tex2d_);
					if (SUCCEEDED(hr))
					{
						D3D11_TEXTURE2D_DESC desc_ = {};
						tex2d_->GetDesc(&desc_);
						sw_ = (float)desc_.Width;
						sh_ = (float)desc_.Height;
					}
					else
					{
						spdlog::error("[luastg] ID3D11Resource::QueryInterface -> #ID3D11Texture2D 调用失败");
					}
					rtv_->Release();
				}
			}
			if (sw_ < 1.0f || sh_ < 1.0f)
			{
				spdlog::warn("[luastg] LuaSTG::Core::Renderer::postEffect 调用提前中止，当前渲染管线未绑定渲染目标");
				return;
			}

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_bak = (ID3D11ShaderResourceView*)_state_set.texture.handle;
			RendererStateSet sbak = _state_set;
			CameraStateSet cbak = _camera_state_set;

			setOrtho(Box{ .left = 0.0f, .top = sh_, .front = 0.0f, .right = sw_, .bottom = 0.0f, .back = 1.0f });

			setViewport(Box{ .left = 0.0f, .top = 0.0f, .front = 0.0f, .right = sw_, .bottom = sh_, .back = 1.0f });
			setScissorRect(Rect{ .left = 0.0f, .top = 0.0f, .right = sw_, .bottom = sh_ });

			setVertexColorBlendState(VertexColorBlendState::Zero);
			setFogState(FogState::Disable, Color{ .color = 0 }, 0.0f, 0.0f);
			setDepthState(DepthState::Disable);
			setBlendState(blend);

			ID3D11ShaderResourceView* main_srv_ = (ID3D11ShaderResourceView*)rt.handle;
			_devctx->PSSetShaderResources(4, 1, &main_srv_);
			setSamplerState(rtsv, 4);
			for (DWORD stage = 0; stage < std::min<DWORD>((DWORD)tv_sv_n, 4); stage += 1)
			{
				ID3D11ShaderResourceView* srv_ = (ID3D11ShaderResourceView*)tv[stage].handle;
				_devctx->PSSetShaderResources(stage, 1, &srv_);
				setSamplerState(sv[stage], stage);
			}
			/* upload built-in value */ {
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = _devctx->Map(_user_float_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (FAILED(hr))
				{
					spdlog::error("[luastg] ID3D11DeviceContext::Map -> #user_float_buffer 调用失败，上传数据失败，LuaSTG::Core::Renderer::postEffect 调用提前中止");
					return;
				}
				std::memcpy(res_.pData, cv, std::min<UINT>((UINT)cv_n, 8) * sizeof(Vector4));
				_devctx->Unmap(_user_float_buffer.Get(), 0);
			}
			float ps_cbdata[8] = {
				sw_, sh_, 0.0f, 0.0f,
				sbak.viewport.left, sbak.viewport.top, sbak.viewport.right, sbak.viewport.bottom,
			};
			/* upload built-in value */ {
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = _devctx->Map(_fog_data_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (FAILED(hr))
				{
					spdlog::error("[luastg] ID3D11DeviceContext::Map -> #engine_built_in_value_buffer 调用失败，无法上传渲染目标尺寸和视口信息，LuaSTG::Core::Renderer::postEffect 调用提前中止");
					return;
				}
				std::memcpy(res_.pData, ps_cbdata, sizeof(ps_cbdata));
				_devctx->Unmap(_fog_data_buffer.Get(), 0);
			}
			ID3D11Buffer* psdata_eff_ = _user_float_buffer.Get();
			_devctx->PSSetConstantBuffers(0, 1, &psdata_eff_);
			_devctx->PSSetShader((ID3D11PixelShader*)ps.handle, NULL, 0);

			// work with batch draw
			if (tv_sv_n > 0)
			{
				setTexture(tv[0]);
			}
			else
			{
				setTexture(TextureID());
			}

			drawQuad(
				DrawVertex2D{ .x = 0.f, .y = sh_, .z = 0.5f, .color = 0xFFFFFFFF, .u = 0.0f, .v = 0.0f },
				DrawVertex2D{ .x = sw_, .y = sh_, .z = 0.5f, .color = 0xFFFFFFFF, .u = 1.0f, .v = 0.0f },
				DrawVertex2D{ .x = sw_, .y = 0.f, .z = 0.5f, .color = 0xFFFFFFFF, .u = 1.0f, .v = 1.0f },
				DrawVertex2D{ .x = 0.f, .y = 0.f, .z = 0.5f, .color = 0xFFFFFFFF, .u = 0.0f, .v = 1.0f });

			batchFlush();

			ID3D11Buffer* psdata_ = _camera_pos_buffer.Get();
			_devctx->PSSetConstantBuffers(0, 1, &psdata_);
			ID3D11SamplerState* void_samp_[4] = { NULL, NULL, NULL, NULL };
			_devctx->PSSetSamplers(1, 4, void_samp_);
			ID3D11ShaderResourceView* void_srv_[4] = { NULL, NULL, NULL, NULL };
			_devctx->PSSetShaderResources(1, 4, void_srv_);

			_state_set = sbak;
			_camera_state_set = cbak;
			TextureID_retain(_state_set.texture);
			initState();
		}
	private:
		ScopeObject<ModelSharedComponent> model_shared_;
	public:
		bool createModel(char const* gltf_path, IModel** model)
		{
			if (!model_shared_)
			{
				spdlog::info("[luastg] 创建模型渲染器共享组件");
				*(~model_shared_) = new ModelSharedComponent();
				if (!model_shared_->attachDevice(_device.Get()))
				{
					spdlog::error("[luastg] 无法创建模型渲染器共享组件");
					return false;
				}
				spdlog::info("[luastg] 已创建模型渲染器共享组件");
			}

			ScopeObject<Model> scope_model;
			*(~scope_model) = new Model(gltf_path, model_shared_);
			if (!scope_model->attachDevice())
			{
				spdlog::error("[luastg] LuaSTG::Core::Renderer::createModel 失败");
				return false;
			}

			scope_model->retain();
			*model = *scope_model;

			return true;
		}
		bool drawModel(IModel* model)
		{
			if (!model)
			{
				assert(false);
				return false;
			}

			if (!endScene())
			{
				return false;
			}

			((Model*)model)->draw();

			if (!beginScene())
			{
				return false;
			}

			return true;
		}
	};
}

#define RendererClass Renderer11

#define self ((RendererClass*)_pImpl)

namespace LuaSTG::Core
{
	bool Renderer::attachDevice(void* dev)
	{
		return self->attachDevice(dev);
	}
	void Renderer::detachDevice()
	{
		self->detachDevice();
	}
	
	bool Renderer::beginScene()
	{
		return self->beginScene();
	}
	bool Renderer::endScene()
	{
		return self->endScene();
	}

	void Renderer::clearRenderTarget(Color const& color)
	{
		self->clearRenderTarget(color);
	}
	void Renderer::clearDepthBuffer(float zvalue)
	{
		self->clearDepthBuffer(zvalue);
	}

	void Renderer::setOrtho(Box const& box)
	{
		self->setOrtho(box);
	}
	void Renderer::setPerspective(Vector3 const& eye, Vector3 const& lookat, Vector3 const& headup, float fov, float aspect, float znear, float zfar)
	{
		self->setPerspective(eye, lookat, headup, fov, aspect, znear, zfar);
	}

	void Renderer::setViewport(Box const& box)
	{
		self->setViewport(box);
	}
	void Renderer::setScissorRect(Rect const& rect)
	{
		self->setScissorRect(rect);
	}
	void Renderer::setViewportAndScissorRect()
	{
		self->setViewportAndScissorRect();
	}

	void Renderer::setVertexColorBlendState(VertexColorBlendState state)
	{
		self->setVertexColorBlendState(state);
	}
	void Renderer::setSamplerState(SamplerState state)
	{
		self->setSamplerState(state);
	}
	void Renderer::setFogState(FogState state, Color const& color, float density_or_znear, float zfar)
	{
		self->setFogState(state, color, density_or_znear, zfar);
	}
	void Renderer::setDepthState(DepthState state)
	{
		self->setDepthState(state);
	}
	void Renderer::setBlendState(BlendState state)
	{
		self->setBlendState(state);
	}
	void Renderer::setTexture(TextureID texture)
	{
		self->setTexture(texture);
	}
	void Renderer::setTextureAlphaType(TextureAlphaType state)
	{
		self->setTextureAlphaType(state);
	}

	bool Renderer::flush()
	{
		return self->flush();
	}
	void Renderer::drawTriangle(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3)
	{
		self->drawTriangle(v1, v2, v3);
	}
	void Renderer::drawTriangle(DrawVertex2D const* pvert)
	{
		self->drawTriangle(pvert);
	}
	void Renderer::drawQuad(DrawVertex2D const& v1, DrawVertex2D const& v2, DrawVertex2D const& v3, DrawVertex2D const& v4)
	{
		self->drawQuad(v1, v2, v3, v4);
	}
	void Renderer::drawQuad(DrawVertex2D const* pvert)
	{
		self->drawQuad(pvert);
	}
	void Renderer::drawRaw(DrawVertex2D const* pvert, uint16_t nvert, DrawIndex2D const* pidx, uint16_t nidx)
	{
		self->drawRaw(pvert, nvert, pidx, nidx);
	}
	void Renderer::drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex2D** ppvert, DrawIndex2D** ppidx, uint16_t* idxoffset)
	{
		self->drawRequest(nvert, nidx, ppvert, ppidx, idxoffset);
	}

	ShaderID Renderer::createPostEffectShader(char const* name, void const* data, size_t size)
	{
		return self->createPostEffectShader(name, data, size);
	}
	void Renderer::destroyPostEffectShader(ShaderID& ps)
	{
		self->destroyPostEffectShader(ps);
	}
	void Renderer::postEffect(ShaderID const& ps, TextureID const& rt, SamplerState rtsv, Vector4 const* cv, size_t cv_n, TextureID const* tv, SamplerState const* sv, size_t tv_sv_n, BlendState blend)
	{
		self->postEffect(ps, rt, rtsv, cv, cv_n, tv, sv, tv_sv_n, blend);
	}

	bool Renderer::createModel(char const* gltf_path, IModel** model)
	{
		return self->createModel(gltf_path, model);
	}
	bool Renderer::drawModel(IModel* model)
	{
		return self->drawModel(model);
	}

	Renderer::Renderer() : _pImpl(nullptr)
	{
		_pImpl = new RendererClass;
	}
	Renderer::Renderer(Renderer&& right) noexcept : _pImpl(nullptr)
	{
		std::swap(_pImpl, right._pImpl);
	}
	Renderer::~Renderer()
	{
		RendererClass* cls = self;
		delete cls;
	}
}
