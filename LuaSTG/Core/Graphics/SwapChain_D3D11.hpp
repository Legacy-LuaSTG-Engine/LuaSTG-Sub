#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"

namespace LuaSTG::Core::Graphics
{
	class SwapChain_D3D11
		: public Object<ISwapChain>
		, public IWindowEventListener
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Window_Win32> m_window;
		ScopeObject<Device_D3D11> m_device;
		
		Microsoft::WRL::Wrappers::Event dxgi_swapchain_event;
		Microsoft::WRL::ComPtr<IDXGISwapChain> dxgi_swapchain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11_dsv;

		std::vector<DisplayMode> m_displaymode;

		DXGI_FORMAT m_swapchain_format{ DXGI_FORMAT_B8G8R8A8_UNORM };
		DXGI_FORMAT m_ds_format{ DXGI_FORMAT_D24_UNORM_S8_UINT };
		UINT m_swapchain_buffer_count{ 0 };
		UINT m_swapchain_flags{ 0 };
		BOOL m_swapchain_vsync{ FALSE };

		DisplayMode m_swapchain_last_mode{ 640, 480 };
		BOOL m_swapchain_last_windowed{ TRUE };
		BOOL m_swapchain_last_flip{ FALSE };

		BOOL m_init{ FALSE };
		std::atomic_int m_window_active_changed{ 0 };

	private:
		void onDeviceCreate();
		void onDeviceDestroy();

		void onWindowCreate();
		void onWindowDestroy();

		void onWindowActive();
		void onWindowInactive();

	public:
		ID3D11RenderTargetView* GetRTV() { return d3d11_rtv.Get(); }
		ID3D11DepthStencilView* GetDSV() { return d3d11_dsv.Get(); }

	private:
		void destroySwapChain();
		bool createSwapChain(bool windowed, bool flip, DisplayMode const& mode, bool no_attachment);
		void destroyRenderAttachment();
		bool createRenderAttachment();

	private:
		enum class EventType
		{
			SwapChainCreate,
			SwapChainDestroy,
		};
		bool m_is_dispatch_event{ false };
		std::vector<ISwapChainEventListener*> m_eventobj;
		std::vector<ISwapChainEventListener*> m_eventobj_late;
		void dispatchEvent(EventType t);
	public:
		void addEventListener(ISwapChainEventListener* e);
		void removeEventListener(ISwapChainEventListener* e);
		
		bool refreshDisplayMode();
		uint32_t getDisplayModeCount();
		DisplayMode getDisplayMode(uint32_t index);
		bool findBestMatchDisplayMode(DisplayMode& mode);

		bool setWindowMode(uint32_t width, uint32_t height, bool flip_model);
		bool setSize(uint32_t width, uint32_t height);
		bool setExclusiveFullscreenMode(DisplayMode const& mode);
		bool isWindowMode() { return m_swapchain_last_windowed; }
		uint32_t getWidth() { return m_swapchain_last_mode.width; }
		uint32_t getHeight() { return m_swapchain_last_mode.height; }

		void clearRenderAttachment();
		void applyRenderAttachment();
		void syncWindowActive();
		void waitFrameLatency();
		void setVSync(bool enable);
		bool present();

		bool saveSnapshotToFile(StringView path);

	public:
		SwapChain_D3D11(Window_Win32* p_window, Device_D3D11* p_device);
		~SwapChain_D3D11();
	public:
		static bool create(Window_Win32* p_window, Device_D3D11* p_device, SwapChain_D3D11** pp_swapchain);
	};
}
