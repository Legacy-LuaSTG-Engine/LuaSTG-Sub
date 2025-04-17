#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Device.hpp"

// DepthStencilBuffer
namespace core::Graphics::Direct3D11 {
	class Device;

	class DepthStencilBuffer final
		: public Object<IDepthStencilBuffer>
		, public IDeviceEventListener {
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IDepthStencilBuffer

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.Get(); }
		bool setSize(Vector2U size) override;
		[[nodiscard]] Vector2U getSize() const noexcept override { return m_size; }

		// DepthStencilBuffer

		DepthStencilBuffer();
		DepthStencilBuffer(DepthStencilBuffer const&) = delete;
		DepthStencilBuffer(DepthStencilBuffer&&) = delete;
		DepthStencilBuffer& operator=(DepthStencilBuffer const&) = delete;
		DepthStencilBuffer& operator=(DepthStencilBuffer&&) = delete;
		~DepthStencilBuffer();

		[[nodiscard]] ID3D11Texture2D* GetResource() const noexcept { return m_texture.Get(); }
		[[nodiscard]] ID3D11DepthStencilView* GetView() const noexcept { return m_view.Get(); }

		bool initialize(Device* device, Vector2U size);
		bool createResource();

	private:
		ScopeObject<Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_view;
		Vector2U m_size{};
		bool m_initialized{ false };
	};
}
