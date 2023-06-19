﻿#include "Core/Graphics/Renderer_D3D11.hpp"
#include "Core/Graphics/Model_D3D11.hpp"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace Core::Graphics
{
	void PostEffectShader_D3D11::onDeviceCreate()
	{
		createResources();
	}
	void PostEffectShader_D3D11::onDeviceDestroy()
	{
		d3d11_ps.Reset();
	}

	PostEffectShader_D3D11::PostEffectShader_D3D11(Device_D3D11* p_device, StringView path, bool is_path_)
		: m_device(p_device)
		, source(path)
		, is_path(is_path_)
	{
		if (!createResources())
			throw std::runtime_error("PostEffectShader_D3D11::PostEffectShader_D3D11");
		m_device->addEventListener(this);
	}
	PostEffectShader_D3D11::~PostEffectShader_D3D11()
	{
		m_device->removeEventListener(this);
	}
}

namespace Core::Graphics
{
	inline ID3D11ShaderResourceView* get_view(Texture2D_D3D11* p)
	{
		return p ? p->GetView() : NULL;
	}
	inline ID3D11ShaderResourceView* get_view(ITexture2D* p)
	{
		return get_view(static_cast<Texture2D_D3D11*>(p));
	}
	inline ID3D11ShaderResourceView* get_view(ScopeObject<Texture2D_D3D11>& p)
	{
		return get_view(p.get());
	}
	inline ID3D11ShaderResourceView* get_view(ScopeObject<ITexture2D>& p)
	{
		return get_view(static_cast<Texture2D_D3D11*>(p.get()));
	}

	void Renderer_D3D11::setVertexIndexBuffer(size_t index)
	{
		assert(m_device->GetD3D11DeviceContext());

		auto& vi = _vi_buffer[(index == 0xFFFFFFFFu) ? _vi_buffer_index : index];
		ID3D11Buffer* vbo[1] = { vi.vertex_buffer.Get() };
		UINT stride[1] = { sizeof(IRenderer::DrawVertex) };
		UINT offset[1] = { 0 };
		m_device->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, vbo, stride, offset);

		constexpr DXGI_FORMAT format = sizeof(DrawIndex) < 4 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		m_device->GetD3D11DeviceContext()->IASetIndexBuffer(vi.index_buffer.Get(), format, 0);
	}
	bool Renderer_D3D11::uploadVertexIndexBuffer(bool discard)
	{
		assert(m_device->GetD3D11DeviceContext());
		ZoneScoped;
		TracyD3D11Zone(tracy::xTracyD3D11Ctx(), "UploadVertexIndexBuffer");
		HRESULT hr = 0;
		auto& vi_ = _vi_buffer[_vi_buffer_index];
		const D3D11_MAP map_type_ = discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
		// copy vertex data
		if (_draw_list.vertex.size > 0)
		{
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			hr = gHR = m_device->GetD3D11DeviceContext()->Map(vi_.vertex_buffer.Get(), 0, map_type_, 0, &res_);
			if (FAILED(hr))
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #vertex_buffer[{}] 调用失败，无法上传顶点", _vi_buffer_index);
				return false;
			}
			std::memcpy((DrawVertex*)res_.pData + vi_.vertex_offset, _draw_list.vertex.data, _draw_list.vertex.size * sizeof(DrawVertex));
			m_device->GetD3D11DeviceContext()->Unmap(vi_.vertex_buffer.Get(), 0);
		}
		// copy index data
		if (_draw_list.index.size > 0)
		{
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			hr = gHR = m_device->GetD3D11DeviceContext()->Map(vi_.index_buffer.Get(), 0, map_type_, 0, &res_);
			if (FAILED(hr))
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #index_buffer[{}] 调用失败，无法上传顶点索引", _vi_buffer_index);
				return false;
			}
			std::memcpy((DrawIndex*)res_.pData + vi_.index_offset, _draw_list.index.data, _draw_list.index.size * sizeof(DrawIndex));
			m_device->GetD3D11DeviceContext()->Unmap(vi_.index_buffer.Get(), 0);
		}
		return true;
	}
	void Renderer_D3D11::clearDrawList()
	{
		for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
		{
			_draw_list.command.data[j_].texture.reset();
		}
		_draw_list.vertex.size = 0;
		_draw_list.index.size = 0;
		_draw_list.command.size = 0;
	}

	bool Renderer_D3D11::createBuffers()
	{
		assert(m_device->GetD3D11Device());

		HRESULT hr = 0;

		{
			{
				D3D11_BUFFER_DESC desc_ = {
					.ByteWidth = 4 * sizeof(DrawVertex),
					.Usage = D3D11_USAGE_DYNAMIC,
					.BindFlags = D3D11_BIND_VERTEX_BUFFER,
					.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
					.MiscFlags = 0,
					.StructureByteStride = 0,
				};
				hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_fx_vbuffer);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(_fx_vbuffer.Get());
			}
			{
				DrawIndex const idx_[6] = { 0, 1, 2, 0, 2, 3 };
				D3D11_BUFFER_DESC desc_ = {
					.ByteWidth = 6 * sizeof(DrawIndex),
					.Usage = D3D11_USAGE_DEFAULT,
					.BindFlags = D3D11_BIND_INDEX_BUFFER,
					.CPUAccessFlags = 0,
					.MiscFlags = 0,
					.StructureByteStride = 0,
				};
				D3D11_SUBRESOURCE_DATA data_ = {
					.pSysMem = &idx_,
					.SysMemPitch = desc_.ByteWidth,
					.SysMemSlicePitch = desc_.ByteWidth,
				};
				hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, &data_, &_fx_ibuffer);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(_fx_ibuffer.Get());
			}
		}

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
				hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &vi_.vertex_buffer);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(vi_.vertex_buffer.Get());
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
				hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &vi_.index_buffer);
				if (FAILED(hr))
					return false;
				M_D3D_SET_DEBUG_NAME_SIMPLE(vi_.index_buffer.Get());
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
			hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_vp_matrix_buffer);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_vp_matrix_buffer.Get());

			hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_world_matrix_buffer);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_world_matrix_buffer.Get());

			desc_.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4);
			hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_camera_pos_buffer);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_camera_pos_buffer.Get());

			desc_.ByteWidth = 2 * sizeof(DirectX::XMFLOAT4);
			hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_fog_data_buffer);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_fog_data_buffer.Get());

			desc_.ByteWidth = 8 * sizeof(DirectX::XMFLOAT4); // 用户最多可用 8 个 float4
			hr = gHR = m_device->GetD3D11Device()->CreateBuffer(&desc_, NULL, &_user_float_buffer);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_user_float_buffer.Get());
		}

		return true;
	}
	bool Renderer_D3D11::createStates()
	{
		assert(m_device->GetD3D11Device());

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
			hr = gHR = m_device->GetD3D11Device()->CreateRasterizerState(&desc_, &_raster_state);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_raster_state.Get());
		}

		{
			Graphics::SamplerState sampler_state;

			// point

			sampler_state.filer = Filter::Point;
			sampler_state.address_u = TextureAddressMode::Wrap;
			sampler_state.address_v = TextureAddressMode::Wrap;
			sampler_state.address_w = TextureAddressMode::Wrap;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::PointWrap)]))
				return false;

			sampler_state.filer = Filter::Point;
			sampler_state.address_u = TextureAddressMode::Clamp;
			sampler_state.address_v = TextureAddressMode::Clamp;
			sampler_state.address_w = TextureAddressMode::Clamp;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::PointClamp)]))
				return false;

			sampler_state.filer = Filter::Point;
			sampler_state.address_u = TextureAddressMode::Border;
			sampler_state.address_v = TextureAddressMode::Border;
			sampler_state.address_w = TextureAddressMode::Border;
			sampler_state.border_color = BorderColor::Black;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::PointBorderBlack)]))
				return false;

			sampler_state.filer = Filter::Point;
			sampler_state.address_u = TextureAddressMode::Border;
			sampler_state.address_v = TextureAddressMode::Border;
			sampler_state.address_w = TextureAddressMode::Border;
			sampler_state.border_color = BorderColor::White;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::PointBorderWhite)]))
				return false;

			// linear

			sampler_state.filer = Filter::Linear;
			sampler_state.address_u = TextureAddressMode::Wrap;
			sampler_state.address_v = TextureAddressMode::Wrap;
			sampler_state.address_w = TextureAddressMode::Wrap;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::LinearWrap)]))
				return false;

			sampler_state.filer = Filter::Linear;
			sampler_state.address_u = TextureAddressMode::Clamp;
			sampler_state.address_v = TextureAddressMode::Clamp;
			sampler_state.address_w = TextureAddressMode::Clamp;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::LinearClamp)]))
				return false;

			sampler_state.filer = Filter::Linear;
			sampler_state.address_u = TextureAddressMode::Border;
			sampler_state.address_v = TextureAddressMode::Border;
			sampler_state.address_w = TextureAddressMode::Border;
			sampler_state.border_color = BorderColor::Black;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::LinearBorderBlack)]))
				return false;

			sampler_state.filer = Filter::Linear;
			sampler_state.address_u = TextureAddressMode::Border;
			sampler_state.address_v = TextureAddressMode::Border;
			sampler_state.address_w = TextureAddressMode::Border;
			sampler_state.border_color = BorderColor::White;
			if (!m_device->createSamplerState(sampler_state, ~_sampler_state[IDX(SamplerState::LinearBorderWhite)]))
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
			hr = gHR = m_device->GetD3D11Device()->CreateDepthStencilState(&desc_, &_depth_state[IDX(DepthState::Disable)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_depth_state[IDX(DepthState::Disable)].Get());

			desc_.DepthEnable = TRUE;
			hr = gHR = m_device->GetD3D11Device()->CreateDepthStencilState(&desc_, &_depth_state[IDX(DepthState::Enable)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_depth_state[IDX(DepthState::Enable)].Get());
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

			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Disable)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Disable)].Get());

			blendt_.BlendEnable = TRUE;

			blendt_.BlendOp = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Alpha)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Alpha)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ZERO;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_ZERO;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::One)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::One)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_MIN;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ONE;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_MIN;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Min)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Min)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_MAX;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ONE;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_MAX;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Max)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Max)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlend = D3D11_BLEND_DEST_COLOR;
			blendt_.DestBlend = D3D11_BLEND_ZERO;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Mul)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Mul)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Screen)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Screen)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ONE;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Add)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Add)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_SUBTRACT;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ONE;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Sub)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Sub)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
			blendt_.SrcBlend = D3D11_BLEND_ONE;
			blendt_.DestBlend = D3D11_BLEND_ONE;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ONE;
			blendt_.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::RevSub)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::RevSub)].Get());

			blendt_.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
			blendt_.SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
			blendt_.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
			blendt_.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendt_.SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendt_.DestBlendAlpha = D3D11_BLEND_ONE;
			copy_();
			hr = gHR = m_device->GetD3D11Device()->CreateBlendState(&desc_, &_blend_state[IDX(BlendState::Inv)]);
			if (FAILED(hr))
				return false;
			M_D3D_SET_DEBUG_NAME_SIMPLE(_blend_state[IDX(BlendState::Inv)].Get());
		}

		return true;
	}
	void Renderer_D3D11::initState()
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
		setSamplerState(_state_set.sampler_state, 0);
		setFogState(_state_set.fog_state, _state_set.fog_color, _state_set.fog_near_or_density, _state_set.fog_far);
		setDepthState(_state_set.depth_state);
		setBlendState(_state_set.blend_state);
		setTexture(_state_texture.get());
		bindTextureAlphaType(_state_texture.get());
		
		_state_dirty = false;
	}
	void Renderer_D3D11::setSamplerState(SamplerState state, UINT index)
	{
		ID3D11SamplerState* d3d11_sampler = static_cast<SamplerState_D3D11*>(_sampler_state[IDX(state)].get())->GetState();
		m_device->GetD3D11DeviceContext()->PSSetSamplers(index, 1, &d3d11_sampler);
	}
	bool Renderer_D3D11::uploadVertexIndexBufferFromDrawList()
	{
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
		return true;
	}
	void Renderer_D3D11::bindTextureSamplerState(ITexture2D* texture)
	{
		ISamplerState* sampler_from_texture = texture ? texture->getSamplerState() : nullptr;
		ISamplerState* sampler = sampler_from_texture ? sampler_from_texture : _sampler_state[IDX(_state_set.sampler_state)].get();
		ID3D11SamplerState* d3d11_sampler = static_cast<SamplerState_D3D11*>(sampler)->GetState();
		m_device->GetD3D11DeviceContext()->PSSetSamplers(0, 1, &d3d11_sampler);
	}
	void Renderer_D3D11::bindTextureAlphaType(ITexture2D* texture)
	{
		TextureAlphaType const state = (texture ? texture->isPremultipliedAlpha() : false) ? TextureAlphaType::PremulAlpha : TextureAlphaType::Normal;
		if (_state_dirty || _state_set.texture_alpha_type != state)
		{
			_state_set.texture_alpha_type = state;
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->PSSetShader(_pixel_shader[IDX(_state_set.vertex_color_blend_state)][IDX(_state_set.fog_state)][IDX(state)].Get(), NULL, 0);
		}
	}
	bool Renderer_D3D11::batchFlush(bool discard)
	{
		ZoneScoped;
		if (!discard)
		{
			TracyD3D11Zone(tracy::xTracyD3D11Ctx(), "BatchFlush");
			// upload data
			if (!uploadVertexIndexBufferFromDrawList()) return false;
			// draw
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			if (_draw_list.command.size > 0)
			{
				VertexIndexBuffer& vi_ = _vi_buffer[_vi_buffer_index];
				for (size_t j_ = 0; j_ < _draw_list.command.size; j_ += 1)
				{
					DrawCommand& cmd_ = _draw_list.command.data[j_];
					if (cmd_.vertex_count > 0 && cmd_.index_count > 0)
					{
						ID3D11ShaderResourceView* srv[1] = { get_view(cmd_.texture) };
						ctx->PSSetShaderResources(0, 1, srv);
						bindTextureSamplerState(cmd_.texture.get());
						bindTextureAlphaType(cmd_.texture.get());
						ctx->DrawIndexed(cmd_.index_count, vi_.index_offset, vi_.vertex_offset);
					}
					vi_.vertex_offset += cmd_.vertex_count;
					vi_.index_offset += cmd_.index_count;
				}
			}
			// unbound: solve some debug warning
			ID3D11ShaderResourceView* null_srv[1] = { NULL };
			ctx->PSSetShaderResources(0, 1, null_srv);
		}
		// clear
		clearDrawList();
		setTexture(_state_texture.get());
		return true;
	}

	bool Renderer_D3D11::createResources()
	{
		assert(m_device->GetD3D11Device());

		spdlog::info("[core] 开始创建渲染器");
		
		if (!createBuffers())
		{
			spdlog::error("[core] 无法创建渲染器所需的顶点、索引缓冲区和着色器常量缓冲区");
			return false;
		}
		if (!createStates())
		{
			spdlog::error("[core] 无法创建渲染器所需的渲染状态");
			return false;
		}
		if (!createShaders())
		{
			spdlog::error("[core] 无法创建渲染器所需的内置着色器");
			return false;
		}

		spdlog::info("[core] 已创建渲染器");

		return true;
	}
	void Renderer_D3D11::onDeviceCreate()
	{
		createResources();
	}
	void Renderer_D3D11::onDeviceDestroy()
	{
		batchFlush(true);

		_state_texture.reset();

		_fx_vbuffer.Reset();
		_fx_ibuffer.Reset();
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
			v.reset();
		}
		for (auto& v : _depth_state)
		{
			v.Reset();
		}
		for (auto& v : _blend_state)
		{
			v.Reset();
		}

		spdlog::info("[core] 已关闭渲染器");
	}

	bool Renderer_D3D11::beginBatch()
	{
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);

		// [IA Stage]

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetInputLayout(_input_layout.Get());
		setVertexIndexBuffer();

		// [VS State]

		ID3D11Buffer* mats[2] = {
			_vp_matrix_buffer.Get(),
			_world_matrix_buffer.Get(),
		};
		ctx->VSSetConstantBuffers(0, 2, mats);

		// [RS Stage]

		ctx->RSSetState(_raster_state.Get());

		// [PS State]

		ID3D11Buffer* psdata[2] = {
			_camera_pos_buffer.Get(),
			_fog_data_buffer.Get(),
		};
		ctx->PSSetConstantBuffers(0, 2, psdata);

		// [OM Stage]

		// [Internal State]

		initState();

		return true;
	}
	bool Renderer_D3D11::endBatch()
	{
		if (!batchFlush())
			return false;
		_state_texture.reset();
		return true;
	}
	bool Renderer_D3D11::flush()
	{
		return batchFlush();
	}

	void Renderer_D3D11::clearRenderTarget(Color4B const& color)
	{
		batchFlush();
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		ID3D11RenderTargetView* rtv = NULL;
		ctx->OMGetRenderTargets(1, &rtv, NULL);
		if (rtv)
		{
			FLOAT const clear_color[4] = {
				(float)color.r / 255.0f,
				(float)color.g / 255.0f,
				(float)color.b / 255.0f,
				(float)color.a / 255.0f,
			};
			ctx->ClearRenderTargetView(rtv, clear_color);
			rtv->Release();
		}
	}
	void Renderer_D3D11::clearDepthBuffer(float zvalue)
	{
		batchFlush();
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		ID3D11DepthStencilView* dsv = NULL;
		ctx->OMGetRenderTargets(1, NULL, &dsv);
		if (dsv)
		{
			ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, zvalue, D3D11_DEFAULT_STENCIL_REFERENCE);
			dsv->Release();
		}
	}
	void Renderer_D3D11::setRenderAttachment(IRenderTarget* p_rt, IDepthStencilBuffer* p_ds)
	{
		batchFlush();
		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		ID3D11RenderTargetView* rtv[1] = { p_rt ? static_cast<RenderTarget_D3D11*>(p_rt)->GetView() : NULL };
		ID3D11DepthStencilView* dsv = p_ds ? static_cast<DepthStencilBuffer_D3D11*>(p_ds)->GetView() : NULL;
		ctx->OMSetRenderTargets(1, rtv, dsv);
	}

	void Renderer_D3D11::setOrtho(BoxF const& box)
	{
		if (_state_dirty || !_camera_state_set.isEqual(box))
		{
			batchFlush();
			_camera_state_set.ortho = box;
			_camera_state_set.is_3D = false;
			DirectX::XMFLOAT4X4 f4x4;
			DirectX::XMStoreFloat4x4(&f4x4, DirectX::XMMatrixOrthographicOffCenterLH(box.a.x, box.b.x, box.b.y, box.a.y, box.a.z, box.b.z));
			/* upload vp matrix */ {
				auto* ctx = m_device->GetD3D11DeviceContext();
				assert(ctx);
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = ctx->Map(_vp_matrix_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (SUCCEEDED(hr))
				{
					std::memcpy(res_.pData, &f4x4, sizeof(f4x4));
					ctx->Unmap(_vp_matrix_buffer.Get(), 0);
				}
				else
				{
					spdlog::error("[core] ID3D11DeviceContext::Map -> #view_projection_matrix_buffer 调用失败，无法上传摄像机变换矩阵");
				}
			}
		}
	}
	void Renderer_D3D11::setPerspective(Vector3F const& eye, Vector3F const& lookat, Vector3F const& headup, float fov, float aspect, float znear, float zfar)
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
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			/* upload vp matrix */ {
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = ctx->Map(_vp_matrix_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (SUCCEEDED(hr))
				{
					std::memcpy(res_.pData, &f4x4, sizeof(f4x4));
					ctx->Unmap(_vp_matrix_buffer.Get(), 0);
				}
				else
				{
					spdlog::error("[core] ID3D11DeviceContext::Map -> #view_projection_matrix_buffer 调用失败，无法上传摄像机变换矩阵");
				}
			}
			/* upload camera pos */ {
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = ctx->Map(_camera_pos_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (SUCCEEDED(hr))
				{
					std::memcpy(res_.pData, camera_pos, sizeof(camera_pos));
					ctx->Unmap(_camera_pos_buffer.Get(), 0);
				}
				else
				{
					spdlog::error("[core] ID3D11DeviceContext::Map -> #camera_position_buffer 调用失败，无法上传摄像机位置");
				}
			}
		}
	}

	void Renderer_D3D11::setViewport(BoxF const& box)
	{
		if (_state_dirty || _state_set.viewport != box)
		{
			batchFlush();
			_state_set.viewport = box;
			D3D11_VIEWPORT const vp = {
				.TopLeftX = box.a.x,
				.TopLeftY = box.a.y,
				.Width = box.b.x - box.a.x,
				.Height = box.b.y - box.a.y,
				.MinDepth = box.a.z,
				.MaxDepth = box.b.z,
			};
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->RSSetViewports(1, &vp);
		}
	}
	void Renderer_D3D11::setScissorRect(RectF const& rect)
	{
		if (_state_dirty || _state_set.scissor_rect != rect)
		{
			batchFlush();
			_state_set.scissor_rect = rect;
			D3D11_RECT const rc = {
				.left = (LONG)rect.a.x,
				.top = (LONG)rect.a.y,
				.right = (LONG)rect.b.x,
				.bottom = (LONG)rect.b.y,
			};
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->RSSetScissorRects(1, &rc);
		}
	}
	void Renderer_D3D11::setViewportAndScissorRect()
	{
		_state_dirty = true;
		setViewport(_state_set.viewport);
		setScissorRect(_state_set.scissor_rect);
		_state_dirty = false;
	}

	void Renderer_D3D11::setVertexColorBlendState(VertexColorBlendState state)
	{
		if (_state_dirty || _state_set.vertex_color_blend_state != state)
		{
			batchFlush();
			_state_set.vertex_color_blend_state = state;
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->PSSetShader(_pixel_shader[IDX(state)][IDX(_state_set.fog_state)][IDX(_state_set.texture_alpha_type)].Get(), NULL, 0);
		}
	}
	void Renderer_D3D11::setFogState(FogState state, Color4B const& color, float density_or_znear, float zfar)
	{
		if (_state_dirty || _state_set.fog_state != state || _state_set.fog_color != color || _state_set.fog_near_or_density != density_or_znear || _state_set.fog_far != zfar)
		{
			batchFlush();
			_state_set.fog_state = state;
			_state_set.fog_color = color;
			_state_set.fog_near_or_density = density_or_znear;
			_state_set.fog_far = zfar;
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->VSSetShader(_vertex_shader[IDX(state)].Get(), NULL, 0);
			float const fog_color_and_range[8] = {
				(float)color.r / 255.0f,
				(float)color.g / 255.0f,
				(float)color.b / 255.0f,
				(float)color.a / 255.0f,
				density_or_znear, zfar, 0.0f, zfar - density_or_znear,
			};
			/* upload */ {
				D3D11_MAPPED_SUBRESOURCE res_ = {};
				HRESULT hr = gHR = ctx->Map(_fog_data_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
				if (SUCCEEDED(hr))
				{
					std::memcpy(res_.pData, fog_color_and_range, sizeof(fog_color_and_range));
					ctx->Unmap(_fog_data_buffer.Get(), 0);
				}
				else
				{
					spdlog::error("[core] ID3D11DeviceContext::Map -> #fog_data_buffer 调用失败，无法上传雾颜色、范围和密度信息");
				}
			}
			ctx->PSSetShader(_pixel_shader[IDX(_state_set.vertex_color_blend_state)][IDX(state)][IDX(_state_set.texture_alpha_type)].Get(), NULL, 0);
		}
	}
	void Renderer_D3D11::setDepthState(DepthState state)
	{
		if (_state_dirty || _state_set.depth_state != state)
		{
			batchFlush();
			_state_set.depth_state = state;
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			ctx->OMSetDepthStencilState(_depth_state[IDX(state)].Get(), D3D11_DEFAULT_STENCIL_REFERENCE);
		}
	}
	void Renderer_D3D11::setBlendState(BlendState state)
	{
		if (_state_dirty || _state_set.blend_state != state)
		{
			batchFlush();
			_state_set.blend_state = state;
			auto* ctx = m_device->GetD3D11DeviceContext();
			assert(ctx);
			FLOAT const factor[4] = {};
			ctx->OMSetBlendState(_blend_state[IDX(state)].Get(), factor, D3D11_DEFAULT_SAMPLE_MASK);
		}
	}

	inline bool is_same(Texture2D_D3D11* a, ITexture2D* b)
	{
		if (a && b)
			return a->GetView() == static_cast<Texture2D_D3D11*>(b)->GetView();
		else if (!a && !b)
			return true;
		else
			return false;
	}
	inline bool is_same(ScopeObject<Texture2D_D3D11>& a, ITexture2D* b)
	{
		return is_same(*a, b);
	}

	void Renderer_D3D11::setTexture(ITexture2D* texture)
	{
		if (_draw_list.command.size > 0 && is_same(_draw_list.command.data[_draw_list.command.size - 1].texture, texture))
		{
			// 可以合并
		}
		else
		{
			// 新的渲染命令
			if ((_draw_list.command.capacity - _draw_list.command.size) < 1)
			{
				batchFlush(); // 需要腾出空间
			}
			_draw_list.command.size += 1;
			DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
			cmd_.texture = static_cast<Texture2D_D3D11*>(texture);
			cmd_.vertex_count = 0;
			cmd_.index_count = 0;
		}
		// 更新当前状态的纹理
		if (!is_same(_state_texture, texture))
		{
			_state_texture = static_cast<Texture2D_D3D11*>(texture);
		}
	}

	bool Renderer_D3D11::drawTriangle(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3)
	{
		if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 3 || (_draw_list.index.capacity - _draw_list.index.size) < 3)
		{
			if (!batchFlush()) return false;
		}
		assert(_draw_list.command.size > 0);
		DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
		DrawVertex* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
		vbuf_[0] = v1;
		vbuf_[1] = v2;
		vbuf_[2] = v3;
		_draw_list.vertex.size += 3;
		DrawIndex* ibuf_ = _draw_list.index.data + _draw_list.index.size;
		ibuf_[0] = cmd_.vertex_count;
		ibuf_[1] = cmd_.vertex_count + 1;
		ibuf_[2] = cmd_.vertex_count + 2;
		_draw_list.index.size += 3;
		cmd_.vertex_count += 3;
		cmd_.index_count += 3;
		return true;
	}
	bool Renderer_D3D11::drawTriangle(DrawVertex const* pvert)
	{
		return drawTriangle(pvert[0], pvert[1], pvert[2]);
	}
	bool Renderer_D3D11::drawQuad(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3, DrawVertex const& v4)
	{
		if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < 4 || (_draw_list.index.capacity - _draw_list.index.size) < 6)
		{
			if (!batchFlush()) return false;
		}
		assert(_draw_list.command.size > 0);
		DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];
		DrawVertex* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
		vbuf_[0] = v1;
		vbuf_[1] = v2;
		vbuf_[2] = v3;
		vbuf_[3] = v4;
		_draw_list.vertex.size += 4;
		DrawIndex* ibuf_ = _draw_list.index.data + _draw_list.index.size;
		ibuf_[0] = cmd_.vertex_count;
		ibuf_[1] = cmd_.vertex_count + 1;
		ibuf_[2] = cmd_.vertex_count + 2;
		ibuf_[3] = cmd_.vertex_count;
		ibuf_[4] = cmd_.vertex_count + 2;
		ibuf_[5] = cmd_.vertex_count + 3;
		_draw_list.index.size += 6;
		cmd_.vertex_count += 4;
		cmd_.index_count += 6;
		return true;
	}
	bool Renderer_D3D11::drawQuad(DrawVertex const* pvert)
	{
		return drawQuad(pvert[0], pvert[1], pvert[2], pvert[3]);
	}
	bool Renderer_D3D11::drawRaw(DrawVertex const* pvert, uint16_t nvert, DrawIndex const* pidx, uint16_t nidx)
	{
		if (nvert > _draw_list.vertex.capacity || nidx > _draw_list.index.capacity)
		{
			assert(false); return false;
		}

		if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < nvert || (_draw_list.index.capacity - _draw_list.index.size) < nidx)
		{
			if (!batchFlush()) return false;
		}

		assert(_draw_list.command.size > 0);
		DrawCommand& cmd_ = _draw_list.command.data[_draw_list.command.size - 1];

		DrawVertex* vbuf_ = _draw_list.vertex.data + _draw_list.vertex.size;
		std::memcpy(vbuf_, pvert, nvert * sizeof(DrawVertex));
		_draw_list.vertex.size += nvert;

		DrawIndex* ibuf_ = _draw_list.index.data + _draw_list.index.size;
		for (size_t idx_ = 0; idx_ < nidx; idx_ += 1)
		{
			ibuf_[idx_] = cmd_.vertex_count + pidx[idx_];
		}
		_draw_list.index.size += nidx;

		cmd_.vertex_count += nvert;
		cmd_.index_count += nidx;

		return true;
	}
	bool Renderer_D3D11::drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex** ppvert, DrawIndex** ppidx, uint16_t* idxoffset)
	{
		if (nvert > _draw_list.vertex.capacity || nidx > _draw_list.index.capacity)
		{
			assert(false); return false;
		}

		if ((_draw_list.vertex.capacity - _draw_list.vertex.size) < nvert || (_draw_list.index.capacity - _draw_list.index.size) < nidx)
		{
			if (!batchFlush()) return false;
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

		return true;
	}

	inline ID3D11SamplerState* get_sampler(ISamplerState* p_sampler)
	{
		return static_cast<SamplerState_D3D11*>(p_sampler)->GetState();
	}
	inline ID3D11SamplerState* get_sampler(ScopeObject<ISamplerState>& p_sampler)
	{
		return static_cast<SamplerState_D3D11*>(p_sampler.get())->GetState();
	}

	bool Renderer_D3D11::createPostEffectShader(StringView path, IPostEffectShader** pp_effect)
	{
		try
		{
			*pp_effect = new PostEffectShader_D3D11(m_device.get(), path, true);
			return true;
		}
		catch (...)
		{
			*pp_effect = nullptr;
			return false;
		}
	}
	bool Renderer_D3D11::drawPostEffect(
		IPostEffectShader* p_effect,
		BlendState blend,
		ITexture2D* p_tex, SamplerState rtsv,
		Vector4F const* cv, size_t cv_n,
		ITexture2D* const* p_tex_arr, SamplerState const* sv, size_t tv_sv_n)
	{
		assert(p_effect);
		assert((cv_n == 0) || (cv_n > 0 && cv));
		assert((tv_sv_n == 0) || (tv_sv_n > 0 && p_tex_arr && sv));

		if (!endBatch()) return false;
		
		// PREPARE

		auto* ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> p_d3d11_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> p_d3d11_dsv;

		float sw_ = 0.0f;
		float sh_ = 0.0f;
		/* get current rendertarget size */ {
			ID3D11RenderTargetView* rtv_ = NULL;
			ID3D11DepthStencilView* dsv_ = NULL;
			ctx->OMGetRenderTargets(1, &rtv_, &dsv_);
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
					spdlog::error("[core] ID3D11Resource::QueryInterface -> #ID3D11Texture2D 调用失败");
					return false;
				}
				p_d3d11_rtv = rtv_;
				rtv_->Release();
			}
			if (dsv_)
			{
				p_d3d11_dsv = dsv_;
				dsv_->Release();
			}
		}
		if (sw_ < 1.0f || sh_ < 1.0f)
		{
			spdlog::warn("[core] LuaSTG::Core::Renderer::postEffect 调用提前中止，当前渲染管线未绑定渲染目标");
			return false;
		}

		ctx->ClearState();

		// [Stage IA]

		/* upload vertex data */ {
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			HRESULT hr = gHR = m_device->GetD3D11DeviceContext()->Map(_fx_vbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
			if (FAILED(hr))
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #_fx_vbuffer 调用失败，无法上传顶点");
				return false;
			}
			DrawVertex const vertex_data[4] = {
				DrawVertex(0.f, sh_, 0.0f, 0.0f),
				DrawVertex(sw_, sh_, 1.0f, 0.0f),
				DrawVertex(sw_, 0.f, 1.0f, 1.0f),
				DrawVertex(0.f, 0.f, 0.0f, 1.0f),
			};
			std::memcpy(res_.pData, vertex_data, sizeof(vertex_data));
			m_device->GetD3D11DeviceContext()->Unmap(_fx_vbuffer.Get(), 0);
		}
		
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11Buffer* p_d3d11_vbos[1] = { _fx_vbuffer.Get() };
		UINT const stride = sizeof(DrawVertex);
		UINT const offset = 0;
		ctx->IASetVertexBuffers(0, 1, p_d3d11_vbos, &stride, &offset);
		ctx->IASetIndexBuffer(_fx_ibuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		ctx->IASetInputLayout(_input_layout.Get());

		// [Stage VS]

		/* upload vp matrix */ {
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			HRESULT hr = gHR = ctx->Map(_vp_matrix_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
			if (SUCCEEDED(hr))
			{
				DirectX::XMFLOAT4X4 f4x4;
				DirectX::XMStoreFloat4x4(&f4x4, DirectX::XMMatrixOrthographicOffCenterLH(0.0f, sw_, 0.0f, sh_, 0.0f, 1.0f));
				std::memcpy(res_.pData, &f4x4, sizeof(f4x4));
				ctx->Unmap(_vp_matrix_buffer.Get(), 0);
			}
			else
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #view_projection_matrix_buffer 调用失败，无法上传摄像机变换矩阵");
			}
		}

		ctx->VSSetShader(_vertex_shader[IDX(FogState::Disable)].Get(), NULL, 0);
		ID3D11Buffer* p_mvp[1] = {_vp_matrix_buffer.Get()};
		ctx->VSSetConstantBuffers(0, 1, p_mvp);

		// [Stage RS]

		ctx->RSSetState(_raster_state.Get());
		D3D11_VIEWPORT viewport = {
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width = sw_,
			.Height = sh_,
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		};
		ctx->RSSetViewports(1, &viewport);
		D3D11_RECT scissor = {
			.left = 0,
			.top = 0,
			.right = (LONG)sw_,
			.bottom = (LONG)sh_,
		};
		ctx->RSSetScissorRects(1, &scissor);

		// [Stage PS]

		/* upload built-in value */ if (cv_n > 0) {
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			HRESULT hr = gHR = ctx->Map(_user_float_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
			if (FAILED(hr))
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #user_float_buffer 调用失败，上传数据失败，LuaSTG::Core::Renderer::postEffect 调用提前中止");
				return false;
			}
			std::memcpy(res_.pData, cv, std::min<UINT>((UINT)cv_n, 8) * sizeof(Vector4F));
			ctx->Unmap(_user_float_buffer.Get(), 0);
		}
		/* upload built-in value */ {
			float ps_cbdata[8] = {
				sw_, sh_, 0.0f, 0.0f,
				_state_set.viewport.a.x, _state_set.viewport.a.y, _state_set.viewport.b.x, _state_set.viewport.b.y,
			};
			D3D11_MAPPED_SUBRESOURCE res_ = {};
			HRESULT hr = gHR = ctx->Map(_fog_data_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res_);
			if (FAILED(hr))
			{
				spdlog::error("[core] ID3D11DeviceContext::Map -> #engine_built_in_value_buffer 调用失败，无法上传渲染目标尺寸和视口信息，LuaSTG::Core::Renderer::postEffect 调用提前中止");
				return false;
			}
			std::memcpy(res_.pData, ps_cbdata, sizeof(ps_cbdata));
			ctx->Unmap(_fog_data_buffer.Get(), 0);
		}
		ID3D11Buffer* p_psdata[2] = { _user_float_buffer.Get(), _fog_data_buffer.Get() };
		ctx->PSSetConstantBuffers(0, 2, p_psdata);
		ctx->PSSetShader(static_cast<PostEffectShader_D3D11*>(p_effect)->GetPS(), NULL, 0);

		ID3D11ShaderResourceView* p_srvs[5] = {};
		ID3D11SamplerState* p_samplers[5] = {};
		p_srvs[4] = get_view(p_tex);
		p_samplers[4] = get_sampler(_sampler_state[IDX(rtsv)]);
		for (DWORD stage = 0; stage < std::min<DWORD>((DWORD)tv_sv_n, 4); stage += 1)
		{
			p_srvs[stage] = get_view(p_tex_arr[stage]);
			p_samplers[stage] = get_sampler(_sampler_state[IDX(sv[stage])]);
		}
		ctx->PSSetShaderResources(0, 5, p_srvs);
		ctx->PSSetSamplers(0, 5, p_samplers);

		// [Stage OM]

		ctx->OMSetDepthStencilState(_depth_state[IDX(DepthState::Disable)].Get(), D3D11_DEFAULT_STENCIL_REFERENCE);
		FLOAT blend_factor[4] = {};
		ctx->OMSetBlendState(_blend_state[IDX(blend)].Get(), blend_factor, D3D11_DEFAULT_SAMPLE_MASK);
		ID3D11RenderTargetView* p_d3d11_rtvs[1] = { p_d3d11_rtv.Get() };
		ctx->OMSetRenderTargets(1, p_d3d11_rtvs, p_d3d11_dsv.Get());

		// DRAW

		ctx->DrawIndexed(6, 0, 0);
		
		// CLEAR

		ctx->ClearState();
		ctx->OMSetRenderTargets(1, p_d3d11_rtvs, p_d3d11_dsv.Get());

		return beginBatch();
	}

	bool Renderer_D3D11::createModel(StringView path, IModel** pp_model)
	{
		if (!m_model_shared)
		{
			spdlog::info("[core] 创建模型渲染器共享组件");
			try
			{
				*(~m_model_shared) = new ModelSharedComponent_D3D11(m_device.get());
				spdlog::info("[luastg] 已创建模型渲染器共享组件");
			}
			catch (...)
			{
				spdlog::error("[core] 无法创建模型渲染器共享组件");
				return false;
			}
		}

		try
		{
			*pp_model = new Model_D3D11(m_device.get(), m_model_shared.get(), path);
			return true;
		}
		catch (const std::exception&)
		{
			*pp_model = nullptr;
			spdlog::error("[luastg] LuaSTG::Core::Renderer::createModel 失败");
			return false;
		}
	}
	bool Renderer_D3D11::drawModel(IModel* p_model)
	{
		if (!p_model)
		{
			assert(false);
			return false;
		}

		if (!endBatch())
		{
			return false;
		}

		static_cast<Model_D3D11*>(p_model)->draw(_state_set.fog_state);

		if (!beginBatch())
		{
			return false;
		}

		return true;
	}

	ISamplerState* Renderer_D3D11::getKnownSamplerState(SamplerState state)
	{
		return _sampler_state[IDX(state)].get();
	}

	Renderer_D3D11::Renderer_D3D11(Device_D3D11* p_device)
		: m_device(p_device)
	{
		if (!createResources())
			throw std::runtime_error("Renderer_D3D11::Renderer_D3D11");
		m_device->addEventListener(this);
	}
	Renderer_D3D11::~Renderer_D3D11()
	{
		m_device->removeEventListener(this);
	}

	bool Renderer_D3D11::create(Device_D3D11* p_device, Renderer_D3D11** pp_renderer)
	{
		try
		{
			*pp_renderer = new Renderer_D3D11(p_device);
			return true;
		}
		catch (...)
		{
			*pp_renderer = nullptr;
			return false;
		}
	}

	bool IRenderer::create(IDevice* p_device, IRenderer** pp_renderer)
	{
		try
		{
			*pp_renderer = new Renderer_D3D11(static_cast<Device_D3D11*>(p_device));
			return true;
		}
		catch (...)
		{
			*pp_renderer = nullptr;
			return false;
		}
	}
}
