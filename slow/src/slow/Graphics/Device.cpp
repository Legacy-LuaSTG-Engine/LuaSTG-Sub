#include "slow/Graphics/Device.hpp"
#include "pch.hpp"
#include "WICTextureLoader11.h"
#include "ScreenGrab11.h"

namespace slow::Graphics
{
	D3D11_PRIMITIVE_TOPOLOGY _primitive_topology_to_d3d11(const EPrimitiveTopology& t)
	{
		switch (t)
		{
		default:
		case EPrimitiveTopology::Unknown: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case EPrimitiveTopology::PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case EPrimitiveTopology::LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case EPrimitiveTopology::LineStrip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case EPrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case EPrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		}
	}
	
	D3D11_INPUT_CLASSIFICATION _input_slot_type_to_d3d11(const EInputSlotType& t)
	{
		switch (t)
		{
		default:
		case EInputSlotType::PerVertexData: return D3D11_INPUT_PER_VERTEX_DATA;
		case EInputSlotType::PerInstanceData: return D3D11_INPUT_PER_INSTANCE_DATA;
		}
	}
	D3D11_INPUT_ELEMENT_DESC _input_element_to_d3d11(const TInputElement& iv)
	{
		D3D11_INPUT_ELEMENT_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_INPUT_ELEMENT_DESC));
		desc.SemanticName = iv.semantic_name.data;
		desc.SemanticIndex = iv.semantic_index;
		desc.Format = (DXGI_FORMAT)iv.format;
		desc.InputSlot = iv.input_slot;
		desc.AlignedByteOffset = iv.aligned_byte_offset;
		desc.InputSlotClass = _input_slot_type_to_d3d11(iv.input_slot_type);
		desc.InstanceDataStepRate = iv.instance_data_step_rate;
		return desc;
	}
	
	D3D11_FILL_MODE _fill_mode_to_d3d11(const EFillMode& mode)
	{
		switch (mode)
		{
		case EFillMode::WireFrame: return D3D11_FILL_WIREFRAME;
		default:
		case EFillMode::Solid: return D3D11_FILL_SOLID;
		}
	}
	D3D11_CULL_MODE _cull_mode_to_d3d11(const ECullMode& mode)
	{
		switch (mode)
		{
		default:
		case ECullMode::None: return D3D11_CULL_NONE;
		case ECullMode::Front: return D3D11_CULL_FRONT;
		case ECullMode::Back: return D3D11_CULL_BACK;
		}
	}
	D3D11_RASTERIZER_DESC _raster_state_to_d3d11(const TRasterizerState& state)
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
		desc.FillMode = _fill_mode_to_d3d11(state.fill_mode);
		desc.CullMode = _cull_mode_to_d3d11(state.cull_mode);
		desc.FrontCounterClockwise = state.front_counter_clock_wise ? TRUE : FALSE;
		desc.DepthClipEnable = state.depth_clip_enable ? TRUE : FALSE;
		desc.ScissorEnable = state.scissor_enable ? TRUE : FALSE;
		desc.MultisampleEnable = state.multi_sample_enable ? TRUE : FALSE;
		desc.AntialiasedLineEnable = state.antialiased_line_enable ? TRUE : FALSE;
		desc.DepthBias = state.depth_bias;
		desc.DepthBiasClamp = state.depth_bias_clamp;
		desc.SlopeScaledDepthBias = state.slope_scaled_depth_bias;
		return desc;
	}

	D3D11_COMPARISON_FUNC _comparison_func_to_d3d11(const EComparisonFunction& func)
	{
		switch (func)
		{
		default:
		case EComparisonFunction::Never: return D3D11_COMPARISON_NEVER;
		case EComparisonFunction::Less: return D3D11_COMPARISON_LESS;
		case EComparisonFunction::Equal: return D3D11_COMPARISON_EQUAL;
		case EComparisonFunction::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
		case EComparisonFunction::Greater: return D3D11_COMPARISON_GREATER;
		case EComparisonFunction::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
		case EComparisonFunction::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
		case EComparisonFunction::Always: return D3D11_COMPARISON_ALWAYS;
		}
	}
	D3D11_STENCIL_OP _stencil_op_to_d3d11(const EStencilOperation& op)
	{
		switch (op)
		{
		default:
		case EStencilOperation::Keep: return D3D11_STENCIL_OP_KEEP;
		case EStencilOperation::Zero: return D3D11_STENCIL_OP_ZERO;
		case EStencilOperation::Replace: return D3D11_STENCIL_OP_REPLACE;
		case EStencilOperation::Increase: return D3D11_STENCIL_OP_INCR_SAT;
		case EStencilOperation::Decrease: return D3D11_STENCIL_OP_DECR_SAT;
		case EStencilOperation::Invert: return D3D11_STENCIL_OP_INVERT;
		case EStencilOperation::IncreaseWrap: return D3D11_STENCIL_OP_INCR;
		case EStencilOperation::DecreaseWrap: return D3D11_STENCIL_OP_DECR;
		}
	}
	D3D11_DEPTH_STENCIL_DESC _depth_stencil_to_d3d11(const TDepthStencilState& state)
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = state.depth_enable ? TRUE : FALSE;
		desc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)state.depth_write_mask;
		desc.DepthFunc = _comparison_func_to_d3d11(state.depth_function);
		desc.StencilEnable = state.stencil_enable ? TRUE : FALSE;
		desc.StencilReadMask = state.stencil_read_mask;
		desc.StencilWriteMask = state.stencil_write_mask;
		desc.FrontFace.StencilFailOp = _stencil_op_to_d3d11(state.front_face.stencil_failed);
		desc.FrontFace.StencilDepthFailOp = _stencil_op_to_d3d11(state.front_face.stencil_depth_failed);
		desc.FrontFace.StencilPassOp = _stencil_op_to_d3d11(state.front_face.stencil_pass);
		desc.FrontFace.StencilFunc = _comparison_func_to_d3d11(state.front_face.stencil_function);
		desc.BackFace.StencilFailOp = _stencil_op_to_d3d11(state.back_face.stencil_failed);
		desc.BackFace.StencilDepthFailOp = _stencil_op_to_d3d11(state.back_face.stencil_depth_failed);
		desc.BackFace.StencilPassOp = _stencil_op_to_d3d11(state.back_face.stencil_pass);
		desc.BackFace.StencilFunc = _comparison_func_to_d3d11(state.back_face.stencil_function);
		return desc;
	}

	D3D11_BLEND _blend_factor_to_d3d11(const EBlendFactor& blend)
	{
		switch (blend)
		{
		default:
		case EBlendFactor::Zero: return D3D11_BLEND_ZERO;
		case EBlendFactor::One: return D3D11_BLEND_ONE;
		case EBlendFactor::PixelColor: return D3D11_BLEND_SRC_COLOR;
		case EBlendFactor::PixelColorInv: return D3D11_BLEND_INV_SRC_COLOR;
		case EBlendFactor::PixelAlpha: return D3D11_BLEND_SRC_ALPHA;
		case EBlendFactor::PixelAlphaInv: return D3D11_BLEND_INV_SRC_ALPHA;
		case EBlendFactor::BufferAlpha: return D3D11_BLEND_DEST_ALPHA;
		case EBlendFactor::BufferAlphaInv: return D3D11_BLEND_INV_DEST_ALPHA;
		case EBlendFactor::BufferColor: return D3D11_BLEND_DEST_COLOR;
		case EBlendFactor::BufferColorInv: return D3D11_BLEND_INV_DEST_COLOR;
		case EBlendFactor::PixelAlphaSAT: return D3D11_BLEND_SRC_ALPHA_SAT;
		case EBlendFactor::Factor: return D3D11_BLEND_BLEND_FACTOR;
		case EBlendFactor::FactorInv: return D3D11_BLEND_INV_BLEND_FACTOR;
		}
	}
	D3D11_BLEND_OP _blend_operation_to_d3d11(const EBlendOperation& op)
	{
		switch (op)
		{
		default:
		case EBlendOperation::Add: return D3D11_BLEND_OP_ADD;
		case EBlendOperation::Sub: return D3D11_BLEND_OP_SUBTRACT;
		case EBlendOperation::RevSub: return D3D11_BLEND_OP_REV_SUBTRACT;
		case EBlendOperation::Min: return D3D11_BLEND_OP_MIN;
		case EBlendOperation::Max: return D3D11_BLEND_OP_MAX;
		}
	}
	D3D11_BLEND_DESC _blend_state_to_d3d11(const TBlendState& state)
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = state.enable ? TRUE : FALSE;
		desc.RenderTarget[0].SrcBlend = _blend_factor_to_d3d11(state.pixel_color_factor);
		desc.RenderTarget[0].DestBlend = _blend_factor_to_d3d11(state.buffer_color_factor);
		desc.RenderTarget[0].BlendOp = _blend_operation_to_d3d11(state.color_operation);
		desc.RenderTarget[0].SrcBlendAlpha = _blend_factor_to_d3d11(state.pixel_alpha_factor);
		desc.RenderTarget[0].DestBlendAlpha = _blend_factor_to_d3d11(state.buffer_alpha_factor);
		desc.RenderTarget[0].BlendOpAlpha = _blend_operation_to_d3d11(state.alpha_operation);
		desc.RenderTarget[0].RenderTargetWriteMask = static_cast<UINT8>(state.write_mask);
		for (size_t idx = 1; idx < 8; idx += 1)
		{
			desc.RenderTarget[idx] = desc.RenderTarget[0];
		}
		return desc;
	}

	struct PipelineState11 : public Object<IPipelineState>
	{
		Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;
		Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state;
		u32 sample_mask;

		void* getNativePipelineState() const noexcept { return (void*)this; }

		PipelineState11()
			: sample_mask(D3D11_DEFAULT_SAMPLE_MASK)
		{
		}
		virtual ~PipelineState11()
		{
		}
	};
}

namespace slow::Graphics
{
	class Device11;

	class Texture2D11 : public Object<ITexture2D>
	{
	private:
		friend Device11;
		uint32_t _width = 0;
		uint32_t _height = 0;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _devctx;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _res;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
		bool _dynamic = false;
	public:
		EResourceType getType() const noexcept { return EResourceType::Texture2D; }

		void* getNativeResource() const noexcept { return reinterpret_cast<void*>(_res.Get()); }

		u32 getWidth() const noexcept { return _width; }
		u32 getHeight() const noexcept { return _height; }

		void* getNativeShaderResourceView() const noexcept { return reinterpret_cast<void*>(_srv.Get()); }

		b8 isDynamic() const noexcept { return _dynamic; }
		b8 updateRegion(u32x4 rect, u8view data, uint32_t rowPitch, b8 autoOffset)
		{
			if (_dynamic && _devctx && _res)
			{
				D3D11_BOX box_ = {};
				box_.left = rect.left;
				box_.top = rect.top;
				box_.front = 0;
				box_.right = rect.right;
				box_.bottom = rect.bottom;
				box_.back = 1;
				if (!autoOffset)
				{
					_devctx->UpdateSubresource(_res.Get(), 0, &box_, data.data, rowPitch, 0);
				}
				else
				{
					// TODO: known bits per pixel
					uint8_t* ptr_ = (uint8_t*)data.data
						+ rect.top * rowPitch // skip y lines of pixel
						+ rect.left * sizeof(uint32_t); // start from x'th pixel
					_devctx->UpdateSubresource(_res.Get(), 0, &box_, ptr_, rowPitch, 0);
				}
				return true;
			}
			return false;
		}
		b8 saveToFile(c8view path, b8 isJPEG)
		{
			if (_devctx && _res)
			{
				HRESULT hr_ = DirectX::SaveWICTextureToFile(
					_devctx.Get(), _res.Get(),
					isJPEG ? GUID_ContainerFormatJpeg : GUID_ContainerFormatPng, _to_wide(path.data).c_str());
				return SUCCEEDED(hr_);
			}
			return false;
		}
	public:
		Texture2D11() {}
		virtual ~Texture2D11() {}
	};

	class RenderTarget11 : public Object<IRenderTarget>
	{
	private:
		friend Device11;
		uint32_t _width = 0;
		uint32_t _height = 0;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _devctx;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _res;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv;
	public:
		EResourceType getType() const noexcept { return EResourceType::RenderTarget; }

		void* getNativeResource() const noexcept { return reinterpret_cast<void*>(_res.Get()); }

		u32 getWidth() const noexcept { return _width; }
		u32 getHeight() const noexcept { return _height; }

		void* getNativeShaderResourceView() const noexcept { return reinterpret_cast<void*>(_srv.Get()); }
		void* getNativeRenderTargetView() const noexcept { return reinterpret_cast<void*>(_rtv.Get()); }

		b8 fillColor(f32x4 color)
		{
			if (_devctx && _rtv)
			{
				_devctx->ClearRenderTargetView(_rtv.Get(), color.data);
				return true;
			}
			return false;
		}
		b8 saveToFile(c8view path, b8 isJPEG)
		{
			if (_devctx && _res)
			{
				HRESULT hr_ = DirectX::SaveWICTextureToFile(
					_devctx.Get(), _res.Get(),
					isJPEG ? GUID_ContainerFormatJpeg : GUID_ContainerFormatPng, _to_wide(path.data).c_str());
				return SUCCEEDED(hr_);
			}
			return false;
		}
	public:
		RenderTarget11() {}
		virtual ~RenderTarget11() {}
	};

	class DepthStencil11 : public Object<IDepthStencil>
	{
	private:
		friend Device11;
		uint32_t _width = 0;
		uint32_t _height = 0;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _devctx;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _res;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv;
	public:
		EResourceType getType() const noexcept { return EResourceType::DepthStencil; }

		void* getNativeResource() const noexcept { return reinterpret_cast<void*>(_res.Get()); }

		u32 getWidth() const noexcept { return _width; }
		u32 getHeight() const noexcept { return _height; }

		void* getNativeDepthStencilView() const noexcept { return reinterpret_cast<void*>(_dsv.Get()); }

		b8 fillDepth(f32 depth)
		{
			if (_devctx && _dsv)
			{
				_devctx->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_DEPTH, depth, 0);
				return true;
			}
			return false;
		}
		b8 fillStencil(u8 stencil)
		{
			if (_devctx && _dsv)
			{
				_devctx->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_STENCIL, 0.0f, stencil);
				return true;
			}
			return false;
		}
		b8 fillDepthStencil(f32 depth, u8 stencil)
		{
			if (_devctx && _dsv)
			{
				_devctx->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
				return true;
			}
			return false;
		}
	public:
		DepthStencil11() {}
		virtual ~DepthStencil11() {}
	};
}

namespace slow::Graphics
{
	class CommandList11 : public Object<ICommandList>
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _ctx;
		object_ptr<PipelineState11> _state;
		EPrimitiveTopology _primitive_topology;
		f32x4 _viewport;
		f32x4 _scissor_rect;
		u32 _stencil_ref;
		f32x4 _blend_factor;
	public:
		void setPrimitiveTopology(EPrimitiveTopology type)
		{
			_primitive_topology = type;
			assert(_ctx); if (!_ctx) return;
			_ctx->IASetPrimitiveTopology(_primitive_topology_to_d3d11(_primitive_topology));
		}
		void setViewport(f32x4 viewport)
		{
			_viewport = viewport;
			assert(_ctx); if (!_ctx) return;
			const D3D11_VIEWPORT vp_[1] = { {_viewport.left, _viewport.top, _viewport.right - _viewport.left, _viewport.bottom - _viewport.top, 0.0f, 1.0f} };
			_ctx->RSSetViewports(1, vp_);
		}
		void setScissorRect(f32x4 scissor)
		{
			_scissor_rect = scissor;
			assert(_ctx); if (!_ctx) return;
			const D3D11_RECT rc_[1] = { {(LONG)_scissor_rect.left, (LONG)_scissor_rect.top,(LONG)_scissor_rect.right,(LONG)_scissor_rect.bottom} };
			_ctx->RSSetScissorRects(1, rc_);
		}
		void setStencilRef(u32 value)
		{
			_stencil_ref = value;
			if (_state)
			{
				assert(_ctx); if (!_ctx) return;
				_ctx->OMSetDepthStencilState(_state->depth_stencil_state.Get(), _stencil_ref);
			}
		}
		void setBlendFactor(f32x4 factor)
		{
			_blend_factor = factor;
			if (_state)
			{
				assert(_ctx); if (!_ctx) return;
				_ctx->OMSetBlendState(_state->blend_state.Get(), _blend_factor.data, _state->sample_mask);
			}
		}
		void setPipelineState(IPipelineState* state)
		{
			assert(_ctx); if (!_ctx) return;
			assert(state); if (!state) return;

			_state = dynamic_cast<PipelineState11*>(state);

			_ctx->IASetInputLayout(_state->input_layout.Get());

			_ctx->VSSetShader(_state->vertex_shader.Get(), NULL, 0);

			_ctx->RSSetState(_state->rasterizer_state.Get());

			_ctx->PSSetShader(_state->pixel_shader.Get(), NULL, 0);

			_ctx->OMSetDepthStencilState(_state->depth_stencil_state.Get(), _stencil_ref);
			_ctx->OMSetBlendState(_state->blend_state.Get(), _blend_factor.data, _state->sample_mask);
		}
	public:
		CommandList11()
			: _primitive_topology(EPrimitiveTopology::Unknown)
			, _viewport(f32x4(0.0f, 0.0f, 0.0f, 0.0f))
			, _scissor_rect(f32x4(0.0f, 0.0f, 0.0f, 0.0f))
			, _stencil_ref(D3D11_DEFAULT_STENCIL_REFERENCE)
			, _blend_factor(f32x4(0.0f, 0.0f, 0.0f, 0.0f))
		{
		}
		virtual ~CommandList11()
		{
		}
	};
	
	class Device11 : public Object<IDevice>
	{
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory1> _dxgi;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> _adapter;
		Microsoft::WRL::ComPtr<ID3D11Device> _device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _devctx;
		D3D_FEATURE_LEVEL _feature_level = D3D_FEATURE_LEVEL_10_0;
		bool _feature_allow_tearing = false;
		bool _feature_clear_view = false;
	private:
		bool _hr_handle(HRESULT hr, const wchar_t* method, const wchar_t* desc)
		{
			if (FAILED(hr))
			{
				std::wstringstream wss_;

				wchar_t fmt_buf_[64] = {};
				std::swprintf(fmt_buf_, 64, L"0x%08X", static_cast<unsigned long>(hr));
				wss_ << L"[slow::Graphics::Device11::" << method << L"] " << desc << L" failed (HRESULT=" << fmt_buf_ << L")";

				wchar_t msg_buf_[256] = {};
				const DWORD msg_cnt = FormatMessageW(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					hr, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
					msg_buf_, 256, NULL);
				if (msg_cnt > 0) wss_ << L" " << msg_buf_;
				else wss_ << L"\n";

				OutputDebugStringW(wss_.str().c_str());
				return false;
			}
			else
			{
				return true;
			}
		}
		void _error_handle(const wchar_t* method, const wchar_t* desc)
		{
			std::wstringstream wss_;
			wss_ << L"[slow::Graphics::Device11::" << method << L"] " << desc << L"\n";
			OutputDebugStringW(wss_.str().c_str());
		}
	public:
		void* getNativeDevice() const noexcept { return reinterpret_cast<void*>(_device.Get()); }

		b8 createTexture2D(u32 width, u32 height, b8 dynamic, ITexture2D** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createTexture2D", L"the device is not initialized ");
				return false;
			}
			
			object_ptr<Texture2D11> obj_; obj_.setDirect(new Texture2D11());
			HRESULT hr_ = S_OK;

			D3D11_TEXTURE2D_DESC res_def_ = {};
			res_def_.Width = width;
			res_def_.Height = height;
			res_def_.MipLevels = 1;
			res_def_.ArraySize = 1;
			res_def_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			res_def_.SampleDesc.Count = 1;
			res_def_.SampleDesc.Quality = 0;
			res_def_.Usage = D3D11_USAGE_DEFAULT;
			res_def_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			res_def_.CPUAccessFlags = 0;
			res_def_.MiscFlags = 0;
			hr_ = _device->CreateTexture2D(&res_def_, NULL, &obj_->_res);
			if (!_hr_handle(hr_, L"createTexture2D", L"ID3D11Device::CreateTexture2D"))
				return false;

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_def_ = {};
			srv_def_.Format = res_def_.Format;
			srv_def_.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_def_.Texture2D.MostDetailedMip = 0;
			srv_def_.Texture2D.MipLevels = res_def_.MipLevels;
			hr_ = _device->CreateShaderResourceView(obj_->_res.Get(), &srv_def_, &obj_->_srv);
			if (!_hr_handle(hr_, L"createTexture2D", L"ID3D11Device::CreateShaderResourceView"))
				return false;

			obj_->_width = width;
			obj_->_height = height;
			obj_->_devctx = _devctx;
			obj_->_dynamic = dynamic;

			*outObject = obj_.getOwnership();
			return true;
		}
		b8 createTexture2DFromFile(c8view path, b8 mipmap, ITexture2D** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createTexture2DFromFile", L"the device is not initialized ");
				return false;
			}
			
			HRESULT hr_ = S_OK;

			Microsoft::WRL::ComPtr<ID3D11Resource> res_;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
			hr_ = DirectX::CreateWICTextureFromFileEx(
				_device.Get(), mipmap ? _devctx.Get() : nullptr,
				_to_wide(path.data).c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				DirectX::WIC_LOADER_DEFAULT,
				res_.GetAddressOf(), srv_.GetAddressOf());
			if (!_hr_handle(hr_, L"createTexture2DFromFile", L"DirectX::CreateWICTextureFromFileEx"))
				return false;

			object_ptr<Texture2D11> obj_; obj_.setDirect(new Texture2D11());
			hr_ = res_.As(&obj_->_res);
			if (!_hr_handle(hr_, L"createTexture2DFromFile", L"ID3D11Resource::QueryInterface(IID_ID3D11Texture2D, X)"))
				return false;
			obj_->_srv = srv_;

			D3D11_TEXTURE2D_DESC res_desc_ = {};
			obj_->_res->GetDesc(&res_desc_);

			obj_->_width = res_desc_.Width;
			obj_->_height = res_desc_.Height;
			obj_->_devctx = _devctx;
			obj_->_dynamic = false;

			*outObject = obj_.getOwnership();
			return true;
		}
		b8 createTexture2DFromMemory(u8view data, b8 mipmap, ITexture2D** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createTexture2DFromMemory", L"the device is not initialized ");
				return false;
			}
			
			HRESULT hr_ = S_OK;

			Microsoft::WRL::ComPtr<ID3D11Resource> res_;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
			hr_ = DirectX::CreateWICTextureFromMemoryEx(
				_device.Get(), mipmap ? _devctx.Get() : nullptr,
				(uint8_t*)data.data, data.size, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				DirectX::WIC_LOADER_DEFAULT,
				res_.GetAddressOf(), srv_.GetAddressOf());
			if (!_hr_handle(hr_, L"createTexture2DFromMemory", L"DirectX::CreateWICTextureFromMemoryEx"))
				return false;

			object_ptr<Texture2D11> obj_; obj_.setDirect(new Texture2D11());
			hr_ = res_.As(&obj_->_res);
			if (!_hr_handle(hr_, L"createTexture2DFromMemory", L"ID3D11Resource::QueryInterface(IID_ID3D11Texture2D, X)"))
				return false;
			obj_->_srv = srv_;

			D3D11_TEXTURE2D_DESC res_desc_ = {};
			obj_->_res->GetDesc(&res_desc_);

			obj_->_width = res_desc_.Width;
			obj_->_height = res_desc_.Height;
			obj_->_devctx = _devctx;
			obj_->_dynamic = false;

			*outObject = obj_.getOwnership();
			return true;
		}
		b8 createRenderTarget(u32 width, u32 height, IRenderTarget** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createRenderTarget", L"the device is not initialized ");
				return false;
			}
			
			object_ptr<RenderTarget11> obj_; obj_.setDirect(new RenderTarget11());
			HRESULT hr_ = S_OK;

			obj_->_width = width;
			obj_->_height = height;
			obj_->_devctx = _devctx;

			D3D11_TEXTURE2D_DESC res_desc_ = {};
			res_desc_.Width = width;
			res_desc_.Height = height;
			res_desc_.MipLevels = 1;
			res_desc_.ArraySize = 1;
			res_desc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			res_desc_.SampleDesc.Count = 1;
			res_desc_.SampleDesc.Quality = 0;
			res_desc_.Usage = D3D11_USAGE_DEFAULT;
			res_desc_.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			res_desc_.CPUAccessFlags = 0;
			res_desc_.MiscFlags = 0;
			hr_ = _device->CreateTexture2D(&res_desc_, NULL, &obj_->_res);
			if (!_hr_handle(hr_, L"createRenderTarget", L"ID3D11Device::CreateTexture2D"))
				return false;
			
			D3D11_RENDER_TARGET_VIEW_DESC rtv_desc_ = {};
			rtv_desc_.Format = res_desc_.Format;
			rtv_desc_.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtv_desc_.Texture2D.MipSlice = 0;
			hr_ = _device->CreateRenderTargetView(obj_->_res.Get(), &rtv_desc_, &obj_->_rtv);
			if (!_hr_handle(hr_, L"createRenderTarget", L"ID3D11Device::CreateRenderTargetView"))
				return false;

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc_ = {};
			srv_desc_.Format = res_desc_.Format;
			srv_desc_.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc_.Texture2D.MostDetailedMip = 0;
			srv_desc_.Texture2D.MipLevels = res_desc_.MipLevels;
			hr_ = _device->CreateShaderResourceView(obj_->_res.Get(), &srv_desc_, &obj_->_srv);
			if (!_hr_handle(hr_, L"createRenderTarget", L"ID3D11Device::CreateShaderResourceView"))
				return false;

			*outObject = obj_.getOwnership();
			return true;
		}
		b8 createDepthStencil(u32 width, u32 height, IDepthStencil** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createDepthStencil", L"the device is not initialized ");
				return false;
			}
			
			object_ptr<DepthStencil11> obj_; obj_.setDirect(new DepthStencil11());
			HRESULT hr_ = S_OK;

			obj_->_width = width;
			obj_->_height = height;
			obj_->_devctx = _devctx;

			D3D11_TEXTURE2D_DESC res_desc_ = {};
			res_desc_.Width = width;
			res_desc_.Height = height;
			res_desc_.MipLevels = 1;
			res_desc_.ArraySize = 1;
			res_desc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			res_desc_.SampleDesc.Count = 1;
			res_desc_.SampleDesc.Quality = 0;
			res_desc_.Usage = D3D11_USAGE_DEFAULT;
			res_desc_.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			res_desc_.CPUAccessFlags = 0;
			res_desc_.MiscFlags = 0;
			hr_ = _device->CreateTexture2D(&res_desc_, NULL, &obj_->_res);
			if (!_hr_handle(hr_, L"createDepthStencil", L"ID3D11Device::CreateTexture2D"))
				return false;

			D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc_ = {};
			dsv_desc_.Format = res_desc_.Format;
			dsv_desc_.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsv_desc_.Flags = 0;
			dsv_desc_.Texture2D.MipSlice = 0;
			hr_ = _device->CreateDepthStencilView(obj_->_res.Get(), &dsv_desc_, &obj_->_dsv);
			if (!_hr_handle(hr_, L"createDepthStencil", L"ID3D11Device::CreateDepthStencilView"))
				return false;

			*outObject = obj_.getOwnership();
			return true;
		}

		b8 createPipelineState(TPipelineState def, IPipelineState** outObject)
		{
			if (!_device)
			{
				_error_handle(L"createPipelineState", L"the device is not initialized ");
				return false;
			}
			
			object_ptr<PipelineState11> obj_; obj_.setDirect(new PipelineState11());
			HRESULT hr_ = 0;

			hr_ = _device->CreateVertexShader(def.VS.data, def.VS.size, NULL, &obj_->vertex_shader);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreateVertexShader"))
				return false;
			hr_ = _device->CreatePixelShader(def.PS.data, def.PS.size, NULL, &obj_->pixel_shader);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreatePixelShader"))
				return false;

			std::vector<D3D11_INPUT_ELEMENT_DESC> ia_def_(def.input_layout.size);
			for (size_t idx_ = 0; idx_ < def.input_layout.size; idx_ += 1)
				ia_def_[idx_] = _input_element_to_d3d11(def.input_layout.element[idx_]);
			hr_ = _device->CreateInputLayout(ia_def_.data(), ia_def_.size(), def.VS.data, def.VS.size, &obj_->input_layout);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreateInputLayout"))
				return false;

			D3D11_RASTERIZER_DESC rs_def_ = _raster_state_to_d3d11(def.rasterizer_state);
			D3D11_DEPTH_STENCIL_DESC ds_def_ = _depth_stencil_to_d3d11(def.depth_stencil_state);
			D3D11_BLEND_DESC blend_def_ = _blend_state_to_d3d11(def.blend_state);

			hr_ = _device->CreateRasterizerState(&rs_def_, &obj_->rasterizer_state);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreateRasterizerState"))
				return false;
			hr_ = _device->CreateDepthStencilState(&ds_def_, &obj_->depth_stencil_state);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreateDepthStencilState"))
				return false;
			hr_ = _device->CreateBlendState(&blend_def_, &obj_->blend_state);
			if (!_hr_handle(hr_, L"createPipelineState", L"ID3D11Device::CreateBlendState"))
				return false;

			obj_->sample_mask = def.sample_mask;

			*outObject = obj_.getOwnership();
			return true;
		}
	public:
		bool createRenderComponents()
		{
			HRESULT hr_ = S_OK;

			// DXGI factory
			hr_ = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgi));
			if (!_hr_handle(hr_, L"createRenderComponents", L"CreateDXGIFactory1")) return false;
			// feature
			Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi5_;
			if (_hr_succeeded(_dxgi.As(&dxgi5_)))
			{
				BOOL bSupport = FALSE;
				if (_hr_succeeded(dxgi5_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bSupport, sizeof(bSupport))))
				{
					_feature_allow_tearing = bSupport;
				}
			}

			// D3D11 creation flags
			UINT create_device_flags_ = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		#ifdef _DEBUG
			create_device_flags_ |= D3D11_CREATE_DEVICE_DEBUG;
		#endif
			const D3D_FEATURE_LEVEL target_feature_levels_[4] = {
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
			};

			// find adapter
			Microsoft::WRL::ComPtr<IDXGIFactory6> dxgi6_;
			DXGI_ADAPTER_DESC1 adapter_desc_ = {};
			auto adapter_find_ = [&]() -> bool {
				if (_hr_succeeded(_adapter->GetDesc1(&adapter_desc_)))
				{
					const bool is_software_ = ((adapter_desc_.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE);
					const bool is_basic_ = ((adapter_desc_.VendorId == 0x1414) && (adapter_desc_.DeviceId == 0x008C)); // Microsoft Basic Render Driver
					if (!is_software_ && !is_basic_)
					{
						const HRESULT test_11_0_ = D3D11CreateDevice(_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
							create_device_flags_, target_feature_levels_ + 1, 3, D3D11_SDK_VERSION,
							NULL, NULL, NULL);
						const HRESULT test_11_1_ = D3D11CreateDevice(_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
							create_device_flags_, target_feature_levels_, 4, D3D11_SDK_VERSION,
							NULL, NULL, NULL);
						if (_hr_succeeded(test_11_0_) || _hr_succeeded(test_11_1_))
						{
							return true;
						}
					}
				}
				return false;
			};
			if (_hr_succeeded(_dxgi.As(&dxgi6_)))
			{
				for (UINT idx_ = 0; _hr_succeeded(dxgi6_->EnumAdapterByGpuPreference(idx_, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter))); idx_ += 1)
				{
					if (adapter_find_()) break;
				}
			}
			else
			{
				for (UINT idx_ = 0; _hr_succeeded(_dxgi->EnumAdapters1(0, &_adapter)); idx_ += 1)
				{
					if (adapter_find_()) break;
				}
			}
			if (!_adapter) { _error_handle(L"createRenderComponents", L"Direct3D11 graphic card doesn't exist"); return false; }

			// D3D11 device & device context
			hr_ = D3D11CreateDevice(_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
				create_device_flags_, target_feature_levels_, 4, D3D11_SDK_VERSION,
				&_device, &_feature_level, &_devctx);
			if (!_hr_handle(hr_, L"createRenderComponents", L"D3D11CreateDevice")) return false;
			// feature
			D3D11_FEATURE_DATA_D3D11_OPTIONS feature_data_1_ = {};
			if (_hr_succeeded(_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &feature_data_1_, sizeof(feature_data_1_)))) _feature_clear_view = feature_data_1_.ClearView;

			return true;
		}
		void destroyRenderComponents()
		{
			_feature_clear_view = false;
			_feature_level = D3D_FEATURE_LEVEL_10_0;
			_devctx.Reset();
			_device.Reset();

			_feature_allow_tearing = false;
			_adapter.Reset();
			_dxgi.Reset();
		}
	public:
		Device11()
		{
			createRenderComponents();
		}
		virtual ~Device11()
		{
			destroyRenderComponents();
		}
	};

	b8 createDevice(IDevice** outObject)
	{
		object_ptr<Device11> obj_; obj_.setDirect(new Device11());
		*outObject = obj_.getOwnership();
		return true;
	}
}
