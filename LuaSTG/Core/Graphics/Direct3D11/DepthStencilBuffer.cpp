#include "Core/Graphics/Direct3D11/DepthStencilBuffer.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/i18n.hpp"

// DepthStencilBuffer
namespace core::Graphics::Direct3D11 {
	void DepthStencilBuffer::onDeviceCreate() {
		if (m_initialized) {
			createResource();
		}
	}
	void DepthStencilBuffer::onDeviceDestroy() {
		m_texture.Reset();
		m_view.Reset();
	}

	bool DepthStencilBuffer::setSize(Vector2U const size) {
		m_texture.Reset();
		m_view.Reset();
		m_size = size;
		return createResource();
	}

	DepthStencilBuffer::DepthStencilBuffer() = default;
	DepthStencilBuffer::~DepthStencilBuffer() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	bool DepthStencilBuffer::initialize(Device* const device, Vector2U const size) {
		assert(device);
		assert(size.x > 0 && size.y > 0);
		m_device = device;
		m_size = size;
		if (!createResource()) {
			return false;
		}
		m_initialized = true;
		m_device->addEventListener(this);
		return true;
	}
	bool DepthStencilBuffer::createResource() {
		HRESULT hr = S_OK;

		auto* d3d11_device = m_device->GetD3D11Device();
		auto* d3d11_devctx = m_device->GetD3D11DeviceContext();
		if (!d3d11_device || !d3d11_devctx)
			return false;

		D3D11_TEXTURE2D_DESC tex2ddef = {
			.Width = m_size.x,
			.Height = m_size.y,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0,},
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
		};
		hr = gHR = d3d11_device->CreateTexture2D(&tex2ddef, nullptr, &m_texture);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateTexture2D");
			return false;
		}
		M_D3D_SET_DEBUG_NAME(m_texture.Get(), "DepthStencilBuffer_D3D11::d3d11_texture2d");

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvdef = {
			.Format = tex2ddef.Format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Texture2D = D3D11_TEX2D_DSV{.MipSlice = 0,},
		};
		hr = gHR = d3d11_device->CreateDepthStencilView(m_texture.Get(), &dsvdef, &m_view);
		if (FAILED(hr)) {
			i18n_core_system_call_report_error("ID3D11Device::CreateDepthStencilView");
			return false;
		}
		M_D3D_SET_DEBUG_NAME(m_view.Get(), "DepthStencilBuffer_D3D11::d3d11_dsv");

		return true;
	}
}
namespace core::Graphics::Direct3D11 {
	bool Device::createDepthStencilBuffer(Vector2U const size, IDepthStencilBuffer** const pp_ds) {
		*pp_ds = nullptr;
		SmartReference<DepthStencilBuffer> buffer;
		buffer.attach(new DepthStencilBuffer);
		if (!buffer->initialize(this, size)) {
			return false;
		}
		*pp_ds = buffer.detach();
		return true;
	}
}
