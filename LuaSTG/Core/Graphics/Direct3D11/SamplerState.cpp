#include "Core/Graphics/Direct3D11/SamplerState.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/i18n.hpp"

// SamplerState
namespace Core::Graphics::Direct3D11 {
	void SamplerState::onDeviceCreate() {
		if (m_initialized) {
			createResource();
		}
	}
	void SamplerState::onDeviceDestroy() {
		m_sampler.Reset();
	}

	SamplerState::SamplerState() = default;
	SamplerState::~SamplerState() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	bool SamplerState::initialize(Device* const device, Core::Graphics::SamplerState const& info) {
		assert(device);
		m_device = device;
		m_info = info;
		if (!createResource()) {
			return false;
		}
		m_initialized = true;
		m_device->addEventListener(this);
		return true;
	}
	bool SamplerState::createResource() {
		HRESULT hr = S_OK;

		D3D11_SAMPLER_DESC desc = {};

		switch (m_info.filer) {
		default: assert(false); return false;
		case Filter::Point: desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case Filter::PointMinLinear: desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
		case Filter::PointMagLinear: desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
		case Filter::PointMipLinear: desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; break;
		case Filter::LinearMinPoint: desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; break;
		case Filter::LinearMagPoint: desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
		case Filter::LinearMipPoint: desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
		case Filter::Linear: desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case Filter::Anisotropic: desc.Filter = D3D11_FILTER_ANISOTROPIC; break;
		}

		auto mapAddressMode_ = [](TextureAddressMode mode) -> D3D11_TEXTURE_ADDRESS_MODE {
			switch (mode) {
			default: return (D3D11_TEXTURE_ADDRESS_MODE)0;
			case TextureAddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
			case TextureAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			case TextureAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
			case TextureAddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
			case TextureAddressMode::MirrorOnce: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			}
		};

	#define mapAddressMode(_X, _Y) \
		desc._Y = mapAddressMode_(m_info._X);\
		if (desc._Y == (D3D11_TEXTURE_ADDRESS_MODE)0) { assert(false); return false; }

		mapAddressMode(address_u, AddressU);
		mapAddressMode(address_v, AddressV);
		mapAddressMode(address_w, AddressW);

	#undef mapAddressMode

		desc.MipLODBias = m_info.mip_lod_bias;

		desc.MaxAnisotropy = m_info.max_anisotropy;

		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		desc.MinLOD = m_info.min_lod;
		desc.MaxLOD = m_info.max_lod;

	#define makeColor(r, g, b, a) \
		desc.BorderColor[0] = r;\
		desc.BorderColor[1] = g;\
		desc.BorderColor[2] = b;\
		desc.BorderColor[3] = a;

		switch (m_info.border_color) {
		default: assert(false); return false;
		case BorderColor::Black:
			makeColor(0.0f, 0.0f, 0.0f, 0.0f);
			break;
		case BorderColor::OpaqueBlack:
			makeColor(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case BorderColor::TransparentWhite:
			makeColor(1.0f, 1.0f, 1.0f, 0.0f);
			break;
		case BorderColor::White:
			makeColor(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		}

	#undef makeColor

		hr = gHR = m_device->GetD3D11Device()->CreateSamplerState(&desc, &m_sampler);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateSamplerState");
			return false;
		}
		M_D3D_SET_DEBUG_NAME(m_sampler.Get(), "SamplerState_D3D11::d3d11_sampler");

		return true;
	}
}
namespace Core::Graphics::Direct3D11 {
	bool Device::createSamplerState(Core::Graphics::SamplerState const& info, ISamplerState** pp_sampler) {
		*pp_sampler = nullptr;
		ScopeObject<SamplerState> buffer;
		buffer.attach(new SamplerState);
		if (!buffer->initialize(this, info)) {
			return false;
		}
		*pp_sampler = buffer.detach();
		return true;
	}
}