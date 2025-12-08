#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Device.hpp"
#include "Platform/RuntimeLoader/DXGI.hpp"
#include "Platform/RuntimeLoader/Direct3D11.hpp"
#include "Platform/RuntimeLoader/Direct2D1.hpp"
#include "Platform/RuntimeLoader/DirectWrite.hpp"

// Device
namespace core::Graphics::Direct3D11 {
	class Device final
		: public implement::ReferenceCounted<IDevice> {
	private:
		// DXGI

		Platform::RuntimeLoader::DXGI dxgi_loader;
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;

		std::string preferred_adapter_name;

		std::string dxgi_adapter_name;
		std::vector<std::string> dxgi_adapter_name_list;

		BOOL dxgi_support_tearing{ FALSE };

		// Direct3D

		D3D_FEATURE_LEVEL d3d_feature_level{ D3D_FEATURE_LEVEL_10_0 };

		// Direct3D 11

		Platform::RuntimeLoader::Direct3D11 d3d11_loader;
		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID3D11Device1> d3d11_device1;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3d11_devctx1;

		// Window Image Component

		Microsoft::WRL::ComPtr<IWICImagingFactory> wic_factory;
		Microsoft::WRL::ComPtr<IWICImagingFactory2> wic_factory2;

		// Direct2D 1

#ifdef LUASTG_ENABLE_DIRECT2D
		Platform::RuntimeLoader::Direct2D1 d2d1_loader;
		Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1_factory;
		Microsoft::WRL::ComPtr<ID2D1Device> d2d1_device;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_devctx;
#endif

		// DirectWrite

		Platform::RuntimeLoader::DirectWrite dwrite_loader;
		Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory;

		// Debug

		tracy_d3d11_context_t tracy_context{};

	public:
		// Get API

		inline IDXGIFactory2* GetDXGIFactory2() const noexcept { return dxgi_factory.Get(); }
		inline IDXGIAdapter1* GetDXGIAdapter1() const noexcept { return dxgi_adapter.Get(); }

		inline std::string_view GetAdapterName() const noexcept { return dxgi_adapter_name; }
		inline std::vector<std::string>& GetAdapterNameArray() { return dxgi_adapter_name_list; }

		inline D3D_FEATURE_LEVEL GetD3DFeatureLevel() const noexcept { return d3d_feature_level; }

		inline ID3D11Device* GetD3D11Device() const noexcept { return d3d11_device.Get(); }
		inline ID3D11Device1* GetD3D11Device1() const noexcept { return d3d11_device1.Get(); }
		inline ID3D11DeviceContext* GetD3D11DeviceContext() const noexcept { return d3d11_devctx.Get(); }
		inline ID3D11DeviceContext1* GetD3D11DeviceContext1() const noexcept { return d3d11_devctx1.Get(); }

#ifdef LUASTG_ENABLE_DIRECT2D
		inline ID2D1Device* GetD2D1Device() const noexcept { return d2d1_device.Get(); }
		inline ID2D1DeviceContext* GetD2D1DeviceContext() const noexcept { return d2d1_devctx.Get(); }
#endif

		inline IWICImagingFactory* GetWICImagingFactory() const noexcept { return wic_factory.Get(); }

		inline BOOL IsTearingSupport() const noexcept { return dxgi_support_tearing; }

		inline tracy_d3d11_context_t GetTracyContext() const noexcept { return tracy_context; }

	private:
		bool createDXGIFactory();
		void destroyDXGIFactory();
		bool selectAdapter();
		bool createDXGI();
		void destroyDXGI();
		bool createD3D11();
		void destroyD3D11();
		bool createWIC();
		void destroyWIC();
#ifdef LUASTG_ENABLE_DIRECT2D
		bool createD2D1();
		void destroyD2D1();
#endif
		bool createDWrite();
		void destroyDWrite();
		bool doDestroyAndCreate();
		bool testAdapterPolicy();
		bool testMultiPlaneOverlay();

	public:
		bool handleDeviceLost();
		bool validateDXGIFactory();

	private:
		enum class EventType {
			DeviceCreate,
			DeviceDestroy,
		};
		bool m_is_dispatch_event{ false };
		std::vector<IDeviceEventListener*> m_eventobj;
		std::vector<IDeviceEventListener*> m_eventobj_late;
	private:
		void dispatchEvent(EventType t);
	public:
		void addEventListener(IDeviceEventListener* e);
		void removeEventListener(IDeviceEventListener* e);

		DeviceMemoryUsageStatistics getMemoryUsageStatistics();

		bool recreate();
		void setPreferenceGpu(StringView preferred_gpu) { preferred_adapter_name = preferred_gpu; }
		uint32_t getGpuCount() { return static_cast<uint32_t>(dxgi_adapter_name_list.size()); }
		StringView getGpuName(uint32_t index) { return dxgi_adapter_name_list[index]; }
		StringView getCurrentGpuName() const noexcept { return dxgi_adapter_name; }

		void* getNativeHandle() { return d3d11_device.Get(); }
#ifdef LUASTG_ENABLE_DIRECT2D
		void* getNativeRendererHandle() { return d2d1_devctx.Get(); }
#else
		void* getNativeRendererHandle() { return nullptr; }
#endif

		bool createVertexBuffer(uint32_t size_in_bytes, IBuffer** output) override;
		bool createIndexBuffer(uint32_t size_in_bytes, IBuffer** output) override;
		bool createConstantBuffer(uint32_t size_in_bytes, IBuffer** output) override;

		bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texture);
		bool createTexture(Vector2U size, ITexture2D** pp_texture);
		bool createTextureFromImage(IImage* image, bool mipmap, ITexture2D** pp_texture);
		bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt);
		bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds);

		bool createSamplerState(core::Graphics::SamplerState const& info, ISamplerState** pp_sampler);

	public:
		Device(std::string_view const& preferred_gpu = "");
		~Device();

	public:
		static bool create(StringView preferred_gpu, Device** p_device);
	};
}
