#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Device.hpp"

// RenderTarget
namespace core::Graphics::Direct3D11 {
	class Device;
	class Texture2D;

	class RenderTarget final
		: public Object<IRenderTarget>
		, public IDeviceEventListener {
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IRenderTarget

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.Get(); }
		[[nodiscard]] void* getNativeBitmapHandle() const noexcept override { return m_bitmap.Get(); }
		bool setSize(Vector2U size) override;
		[[nodiscard]] ITexture2D* getTexture() const noexcept override;

		// RenderTarget

		RenderTarget();
		RenderTarget(RenderTarget const&) = delete;
		RenderTarget(RenderTarget&&) = delete;
		RenderTarget& operator=(RenderTarget const&) = delete;
		RenderTarget& operator=(RenderTarget&&) = delete;
		~RenderTarget();

		[[nodiscard]] ID3D11RenderTargetView* GetView() const noexcept { return m_view.Get(); }

		bool initialize(Device* device, Vector2U size);
		bool createResource();

	private:
		ScopeObject<Device> m_device;
		ScopeObject<Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_view;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_bitmap;
		bool m_initialized{ false };
	};
}
