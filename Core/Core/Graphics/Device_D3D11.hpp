#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Device.hpp"

namespace Core::Graphics
{
	class Device_D3D11 : public Object<IDevice>
	{
	private:
		// DXGI

		HMODULE dxgi_dll{ NULL };
		decltype(::CreateDXGIFactory1)* dxgi_api_CreateDXGIFactory1{ NULL };
		decltype(::CreateDXGIFactory2)* dxgi_api_CreateDXGIFactory2{ NULL };

		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;

		std::string preferred_adapter_name;

		std::string dxgi_adapter_name;
		std::vector<std::string> dxgi_adapter_names;

		DWORD dwm_acceleration_level{ 0 };
		BOOL dxgi_support_flip_model{ FALSE };
		BOOL dxgi_support_low_latency{ FALSE };
		BOOL dxgi_support_flip_model2{ FALSE };
		BOOL dxgi_support_tearing{ FALSE };

		// Direct3D

		D3D_FEATURE_LEVEL d3d_feature_level{ D3D_FEATURE_LEVEL_10_0 };

		// Direct3D 11

		HMODULE d3d11_dll{ NULL };
		decltype(::D3D11CreateDevice)* d3d11_api_D3D11CreateDevice{ NULL };

		Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
		Microsoft::WRL::ComPtr<ID3D11Device1> d3d11_device1;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3d11_devctx1;

		// Window Image Component

		Microsoft::WRL::ComPtr<IWICImagingFactory> wic_factory;
		Microsoft::WRL::ComPtr<IWICImagingFactory2> wic_factory2;

		// Direct2D 1
		
		HMODULE d2d1_dll{ NULL };
		HRESULT(WINAPI* d2d1_api_D2D1CreateFactory)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS*, void**) { NULL };

		Microsoft::WRL::ComPtr<ID2D1Factory> d2d1_factory;
		Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1_factory1;
		Microsoft::WRL::ComPtr<ID2D1Device> d2d1_device;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_devctx;

		// DirectWrite

		HMODULE dwrite_dll{ NULL };
		decltype(DWriteCreateFactory)* dwrite_api_DWriteCreateFactory{ NULL };

		Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory;

	public:
		// Get API

		IDXGIFactory1* GetDXGIFactory1() const noexcept { return dxgi_factory.Get(); }
		IDXGIFactory2* GetDXGIFactory2() const noexcept { return dxgi_factory2.Get(); }
		IDXGIAdapter1* GetDXGIAdapter1() const noexcept { return dxgi_adapter.Get(); }

		void SetPreferredAdapter(std::string_view name) { preferred_adapter_name = name; }
		std::string_view GetAdapterName() const noexcept { return dxgi_adapter_name; }
		std::vector<std::string>& GetAdapterNameArray() { return dxgi_adapter_names; }

		D3D_FEATURE_LEVEL GetD3DFeatureLevel() const noexcept { return d3d_feature_level; }

		ID3D11Device* GetD3D11Device() const noexcept { return d3d11_device.Get(); }
		ID3D11Device1* GetD3D11Device1() const noexcept { return d3d11_device1.Get(); }
		ID3D11DeviceContext* GetD3D11DeviceContext() const noexcept { return d3d11_devctx.Get(); }
		ID3D11DeviceContext1* GetD3D11DeviceContext1() const noexcept { return d3d11_devctx1.Get(); }

		IWICImagingFactory* GetWICImagingFactory() const noexcept { return wic_factory.Get(); }

		BOOL IsFlipSequentialSupport() const noexcept { return dxgi_support_flip_model; }
		BOOL IsFrameLatencySupport() const noexcept { return dxgi_support_low_latency; }
		BOOL IsFlipDiscardSupport() const noexcept { return dxgi_support_flip_model2; }
		BOOL IsTearingSupport() const noexcept { return dxgi_support_tearing; }

	private:
		bool loadDLL();
		void unloadDLL();
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

		uint32_t getGpuCount() { return (uint32_t)dxgi_adapter_names.size(); }
		StringView getGpuName(uint32_t index) { return dxgi_adapter_names[index]; }

		void* getNativeHandle() { return d3d11_device.Get(); }

		bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texutre);
		//bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texutre);
		bool createTexture(Vector2U size, ITexture2D** pp_texutre);

		bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt);
		bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds);

		bool createSamplerState(SamplerState const& def, ISamplerState** pp_sampler);

	public:
		Device_D3D11(std::string_view const& prefered_gpu = "");
		~Device_D3D11();

	public:
		static bool create(StringView prefered_gpu, Device_D3D11** p_device);
	};

	class SamplerState_D3D11
		: public Object<ISamplerState>
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Device_D3D11> m_device;
		SamplerState m_info;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> d3d11_sampler;

	public:
		void onDeviceCreate();
		void onDeviceDestroy();

		bool createResource();

	public:
		ID3D11SamplerState* GetState() { return d3d11_sampler.Get(); }

	public:
		SamplerState_D3D11(Device_D3D11* p_device, SamplerState const& def);
		~SamplerState_D3D11();
	};

	class Texture2D_D3D11
		: public Object<ITexture2D>
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Device_D3D11> m_device;
		ScopeObject<ISamplerState> m_sampler;
		ScopeObject<IData> m_data;
		std::string source_path;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d11_srv;
		Vector2U m_size{};
		bool m_dynamic{ false };
		bool m_premul{ false };
		bool m_mipmap{ false };
		bool m_isrt{ false };

	public:
		void onDeviceCreate();
		void onDeviceDestroy();

		bool createResource();

	public:
		ID3D11Texture2D* GetResource() { return d3d11_texture2d.Get(); }
		ID3D11ShaderResourceView* GetView() { return d3d11_srv.Get(); }

	public:
		void* getNativeHandle() { return d3d11_srv.Get(); }

		bool isDynamic() { return m_dynamic; }
		bool isPremultipliedAlpha() { return m_premul; }
		void setPremultipliedAlpha(bool v) { m_premul = v; }
		Vector2U getSize() { return m_size; }
		bool setSize(Vector2U size);

		bool uploadPixelData(RectU rc, void const* data, uint32_t pitch);
		void setPixelData(IData* p_data) { m_data = p_data; }

		bool saveToFile(StringView path);

		void setSamplerState(ISamplerState* p_sampler) { m_sampler = p_sampler; }
		ISamplerState* getSamplerState() { return m_sampler.get(); }

	public:
		Texture2D_D3D11(Device_D3D11* device, StringView path, bool mipmap);
		Texture2D_D3D11(Device_D3D11* device, Vector2U size, bool rendertarget); // rendertarget = true 时不注册监听器，交给 RenderTarget_D3D11 控制
		~Texture2D_D3D11();
	};

	class RenderTarget_D3D11
		: public Object<IRenderTarget>
		, public IDeviceEventListener
	{
		ScopeObject<Device_D3D11> m_device;
		ScopeObject<Texture2D_D3D11> m_texture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rtv;

	public:
		void onDeviceCreate();
		void onDeviceDestroy();

		bool createResource();

	public:
		ID3D11RenderTargetView* GetView() { return d3d11_rtv.Get(); }

	public:
		void* getNativeHandle() { return d3d11_rtv.Get(); }

		bool setSize(Vector2U size);
		ITexture2D* getTexture() { return *m_texture; }

	public:
		RenderTarget_D3D11(Device_D3D11* device, Vector2U size);
		~RenderTarget_D3D11();
	};

	class DepthStencilBuffer_D3D11
		: public Object<IDepthStencilBuffer>
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Device_D3D11> m_device;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11_dsv;
		Vector2U m_size{};

	public:
		void onDeviceCreate();
		void onDeviceDestroy();

		bool createResource();

	public:
		ID3D11Texture2D* GetResource() { return d3d11_texture2d.Get(); }
		ID3D11DepthStencilView* GetView() { return d3d11_dsv.Get(); }

	public:
		void* getNativeHandle() { return d3d11_dsv.Get(); }

		Vector2U getSize() { return m_size; }

	public:
		DepthStencilBuffer_D3D11(Device_D3D11* device, Vector2U size);
		~DepthStencilBuffer_D3D11();
	};
}
