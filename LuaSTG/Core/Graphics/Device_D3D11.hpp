#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Device.hpp"
#include "Platform/RuntimeLoader/DXGI.hpp"
#include "Platform/RuntimeLoader/Direct3D11.hpp"
#include "Platform/RuntimeLoader/Direct2D1.hpp"
#include "Platform/RuntimeLoader/DirectWrite.hpp"

namespace Core::Graphics
{
	class Device_D3D11 : public Object<IDevice>
	{
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
		
		Platform::RuntimeLoader::Direct2D1 d2d1_loader;
		Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1_factory;
		Microsoft::WRL::ComPtr<ID2D1Device> d2d1_device;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_devctx;

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

		inline ID2D1Device* GetD2D1Device() const noexcept { return d2d1_device.Get(); }
		inline ID2D1DeviceContext* GetD2D1DeviceContext() const noexcept { return d2d1_devctx.Get(); }

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
		bool createD2D1();
		void destroyD2D1();
		bool createDWrite();
		void destroyDWrite();
		bool doDestroyAndCreate();
		bool testAdapterPolicy();
		bool testMultiPlaneOverlay();

	public:
		bool handleDeviceLost();
		bool validateDXGIFactory();

	private:
		enum class EventType
		{
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
		void* getNativeRendererHandle() { return d2d1_devctx.Get(); }

		bool createVertexBuffer(uint32_t size_in_bytes, IBuffer** output);
		bool createIndexBuffer(uint32_t size_in_bytes, IBuffer** output);

		bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texture);
		//bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texture);
		bool createTexture(Vector2U size, ITexture2D** pp_texture);

		bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt);
		bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds);

		bool createSamplerState(SamplerState const& info, ISamplerState** pp_sampler);

	public:
		Device_D3D11(std::string_view const& preferred_gpu = "");
		~Device_D3D11();

	public:
		static bool create(StringView preferred_gpu, Device_D3D11** p_device);
	};
}

// SamplerState
namespace Core::Graphics::Direct3D11 {
	class SamplerState final
		: public Object<ISamplerState>
		, public IDeviceEventListener {

	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// ISamplerState

		// SamplerState

		SamplerState();
		SamplerState(SamplerState const&) = delete;
		SamplerState(SamplerState&&) = delete;
		SamplerState& operator=(SamplerState const&) = delete;
		SamplerState& operator=(SamplerState&&) = delete;
		~SamplerState();

		[[nodiscard]] ID3D11SamplerState* GetState() const noexcept { return m_sampler.Get(); }

		bool initialize(Device_D3D11* device, Core::Graphics::SamplerState const& info);
		bool createResource();

	private:
		ScopeObject<Device_D3D11> m_device;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
		Core::Graphics::SamplerState m_info{};
		bool m_initialized{ false };
	};
}

// Texture2D
namespace Core::Graphics::Direct3D11 {
	class Texture2D final
		: public Object<ITexture2D>
		, public IDeviceEventListener
	{
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// ITexture2D

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.Get(); }

		[[nodiscard]] bool isDynamic() const noexcept override { return m_dynamic; }
		[[nodiscard]] bool isPremultipliedAlpha() const noexcept override { return m_pre_mul_alpha; }
		void setPremultipliedAlpha(bool const v) override { m_pre_mul_alpha = v; }
		[[nodiscard]] Vector2U getSize() const noexcept override { return m_size; }
		bool setSize(Vector2U size) override;

		bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) override;
		void setPixelData(IData* p_data) override { m_data = p_data; }

		bool saveToFile(StringView path) override;

		void setSamplerState(ISamplerState* p_sampler) override { m_sampler = p_sampler; }
		[[nodiscard]] ISamplerState* getSamplerState() const noexcept override { return m_sampler.get(); }

		// Texture2D

		Texture2D();
		Texture2D(Texture2D const&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(Texture2D const&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;
		~Texture2D();

		[[nodiscard]] ID3D11Texture2D* GetResource() const noexcept { return m_texture.Get(); }
		[[nodiscard]] ID3D11ShaderResourceView* GetView() const noexcept { return m_view.Get(); }

		bool initialize(Device_D3D11* device, StringView path, bool mipmap);
		bool initialize(Device_D3D11* device, Vector2U size, bool is_render_target);
		bool createResource();

	private:
		ScopeObject<Device_D3D11> m_device;
		ScopeObject<ISamplerState> m_sampler;
		ScopeObject<IData> m_data;
		std::string m_source_path;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_view;
		Vector2U m_size{};
		bool m_dynamic{ false };
		bool m_pre_mul_alpha{ false };
		bool m_mipmap{ false };
		bool m_is_render_target{ false };
		bool m_initialized{ false };
	};
}

// RenderTarget
namespace Core::Graphics::Direct3D11 {
	class RenderTarget final
		: public Object<IRenderTarget>
		, public IDeviceEventListener
	{
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// IRenderTarget

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.Get(); }
		[[nodiscard]] void* getNativeBitmapHandle() const noexcept override { return m_bitmap.Get(); }
		bool setSize(Vector2U size) override;
		[[nodiscard]] ITexture2D* getTexture() const noexcept override { return m_texture.get(); }

		// RenderTarget

		RenderTarget();
		RenderTarget(RenderTarget const&) = delete;
		RenderTarget(RenderTarget&&) = delete;
		RenderTarget& operator=(RenderTarget const&) = delete;
		RenderTarget& operator=(RenderTarget&&) = delete;
		~RenderTarget();

		[[nodiscard]] ID3D11RenderTargetView* GetView() const noexcept { return m_view.Get(); }

		bool initialize(Device_D3D11* device, Vector2U size);
		bool createResource();

	private:
		ScopeObject<Device_D3D11> m_device;
		ScopeObject<Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_view;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_bitmap;
		bool m_initialized{ false };
	};
}

// DepthStencilBuffer
namespace Core::Graphics::Direct3D11 {
	class DepthStencilBuffer final
		: public Object<IDepthStencilBuffer>
		, public IDeviceEventListener
	{
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

		bool initialize(Device_D3D11* device, Vector2U size);
		bool createResource();

	private:
		ScopeObject<Device_D3D11> m_device;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_view;
		Vector2U m_size{};
		bool m_initialized{ false };
	};
}

// Buffer
namespace Core::Graphics::Direct3D11 {
	class Buffer final
		: public Object<IBuffer>
		, public IDeviceEventListener
	{
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
	 	virtual ~Buffer();

		bool initialize(Device_D3D11* device, uint8_t type, uint32_t size_in_bytes);
		bool createResources();

	private:
		ScopeObject<Device_D3D11> m_device;
		wil::com_ptr_nothrow<ID3D11Buffer> m_buffer;
		uint32_t m_size_in_bytes{};
		uint8_t m_type{}; // 0-unknown, 1-vertex, 2-index
		bool m_initialized{ false };
	};
}
