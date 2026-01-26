#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Device.hpp"

// Buffer
namespace core::Graphics::Direct3D11 {
	class Device;

	class Buffer final
		: public implement::ReferenceCounted<IBuffer>
		, public IDeviceEventListener {
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IBuffer

		bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) override;
		bool unmap() override;

		// Buffer

		Buffer();
		Buffer(Buffer const&) = delete;
		Buffer(Buffer&&) = delete;
		Buffer& operator=(Buffer const&) = delete;
		Buffer& operator=(Buffer&&) = delete;
		~Buffer();

		[[nodiscard]] ID3D11Buffer* getNativeBuffer() const noexcept { return m_buffer.get(); }

		static constexpr uint8_t type_vertex_buffer{ 1 };
		static constexpr uint8_t type_index_buffer{ 2 };
		static constexpr uint8_t type_constant_buffer{ 3 };

		bool initialize(Device* device, uint8_t type, uint32_t size_in_bytes);
		bool createResources();

	private:
		SmartReference<Device> m_device;
		win32::com_ptr<ID3D11Buffer> m_buffer;
		uint32_t m_size_in_bytes{};
		uint8_t m_type{};
		bool m_initialized{ false };
	};
}
