#include "Core/Graphics/Direct3D11/Buffer.hpp"
#include "core/Logger.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"

#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define HRCheckCallReport(x) if (FAILED(hr)) { Logger::error("Windows API failed: " x); }
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { Logger::error("Windows API failed: " x); assert(false); return false; }
#define HRCheckCallNoAssertReturnBool(x) if (FAILED(hr)) { Logger::error("Windows API failed: " x); return false; }

namespace {
	uint32_t alignTo16 (uint32_t const size) {
		return ((size + 15u) / 16u) * 16;
	}
}

// Buffer
namespace core::Graphics::Direct3D11 {
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
	bool Buffer::update(const void* const data, const size_t size_in_bytes) {
		void* ptr{};
		if (!map(size_in_bytes, true, &ptr)) {
			return false;
		}
		std::memcpy(ptr, data, size_in_bytes);
		return unmap();
	}

	Buffer::Buffer() = default;
	Buffer::~Buffer() {
		if (m_initialized && m_device) {
			m_device->removeEventListener(this);
		}
	}

	bool Buffer::initialize(Device* const device, uint8_t const type, uint32_t const size_in_bytes) {
		assert(device);
		assert(type == type_vertex_buffer || type == type_index_buffer || type == type_constant_buffer);
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
		switch (m_type) {
		case type_vertex_buffer:
			info.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case type_index_buffer:
			info.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case type_constant_buffer:
			// For a constant buffer (BindFlags of D3D11_BUFFER_DESC set to D3D11_BIND_CONSTANT_BUFFER),
			// you must set the ByteWidth value of D3D11_BUFFER_DESC in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
			// https://learn.microsoft.com/en-us/windows/win32/api/D3D11/nf-d3d11-id3d11device-createbuffer
			info.ByteWidth = alignTo16(info.ByteWidth);
			info.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			break;
		default:
			assert(false); return false;
		}
		info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		HRGet = device->CreateBuffer(&info, nullptr, m_buffer.put());
		HRCheckCallReturnBool("ID3D11Device::CreateBuffer");
		return true;
	}
}
namespace core::Graphics::Direct3D11 {
	bool Device::createVertexBuffer(uint32_t const size_in_bytes, IBuffer** const output) {
		*output = nullptr;
		SmartReference<Buffer> buffer;
		buffer.attach(new Buffer);
		if (!buffer->initialize(this, Buffer::type_vertex_buffer, size_in_bytes)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
	bool Device::createIndexBuffer(uint32_t const size_in_bytes, IBuffer** const output) {
		*output = nullptr;
		SmartReference<Buffer> buffer;
		buffer.attach(new Buffer);
		if (!buffer->initialize(this, Buffer::type_index_buffer, size_in_bytes)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
	bool Device::createConstantBuffer(uint32_t const size_in_bytes, IBuffer** const output) {
		*output = nullptr;
		SmartReference<Buffer> buffer;
		buffer.attach(new Buffer);
		if (!buffer->initialize(this, Buffer::type_constant_buffer, size_in_bytes)) {
			return false;
		}
		*output = buffer.detach();
		return true;
	}
}
