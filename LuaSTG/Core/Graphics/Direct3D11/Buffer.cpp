#include "Core/Graphics/Direct3D11/Buffer.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/i18n.hpp"

#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define HRCheckCallReport(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); }
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); assert(false); return false; }
#define HRCheckCallNoAssertReturnBool(x) if (FAILED(hr)) { i18n_core_system_call_report_error(x); return false; }

// Buffer
namespace Core::Graphics::Direct3D11 {
	void Buffer::onDeviceCreate() {
		if (m_initialized) {
			createResources();
		}
	}
	void Buffer::onDeviceDestroy() {
		m_buffer.reset();
	}

	bool Buffer::map(uint32_t const size_in_bytes, bool const discard, void** const out_pointer) {
		if (size_in_bytes > m_size_in_bytes) {
			assert(false); return false;
		}
		HRNew;
		auto const ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		assert(m_buffer);
		D3D11_MAPPED_SUBRESOURCE mapped{};
		HRGet = ctx->Map(m_buffer.get(), 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
		HRCheckCallReturnBool("ID3D11DeviceContext::Map");
		*out_pointer = mapped.pData;
		return true;
	}
	bool Buffer::unmap() {
		auto const ctx = m_device->GetD3D11DeviceContext();
		assert(ctx);
		assert(m_buffer);
		ctx->Unmap(m_buffer.get(), 0);
		return true;
	}

	Buffer::Buffer() = default;
	Buffer::~Buffer() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	bool Buffer::initialize(Device* const device, uint8_t const type, uint32_t const size_in_bytes) {
		assert(device);
		assert(type == 1 || type == 2);
		assert(size_in_bytes > 0);
		m_device = device;
		m_size_in_bytes = size_in_bytes;
		m_type = type;
		if (!createResources()) {
			return false;
		}
		m_initialized = true;
		m_device->addEventListener(this);
		return true;
	}
	bool Buffer::createResources() {
		HRNew;
		auto const device = m_device->GetD3D11Device();
		assert(device);
		D3D11_BUFFER_DESC info{};
		info.ByteWidth = m_size_in_bytes;
		info.Usage = D3D11_USAGE_DYNAMIC;
		info.BindFlags = m_type == 1 ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER;
		info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		HRGet = device->CreateBuffer(&info, nullptr, m_buffer.put());
		HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
		return true;
	}
}
namespace Core::Graphics::Direct3D11 {
	bool Device::createVertexBuffer(uint32_t const size_in_bytes, IBuffer** const output) {
		*output = nullptr;
		ScopeObject<Buffer> buffer;
		buffer.attach(new Buffer);
		if (!buffer->initialize(this, 1, size_in_bytes)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
	bool Device::createIndexBuffer(uint32_t const size_in_bytes, IBuffer** const output) {
		*output = nullptr;
		ScopeObject<Buffer> buffer;
		buffer.attach(new Buffer);
		if (!buffer->initialize(this, 2, size_in_bytes)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
}
