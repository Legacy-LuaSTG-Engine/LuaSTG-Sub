#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/Direct3D11/LetterBoxingRenderer.hpp"
#include "Platform/RuntimeLoader/DirectComposition.hpp"

namespace Core::Graphics
{
	class DisplayModeUpdater
	{
	private:
		DISPLAY_DEVICEW last_device{};
		DEVMODEW last_mode{};
		bool is_scope{ false };
	public:
		// TODO: 改进这里的代码
		void Enter(HWND window, UINT width, UINT height)
		{
			Leave();

			assert(window);
			assert(width > 0 && height > 0);

			HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
			assert(monitor);

			MONITORINFOEXW monitor_info{};
			monitor_info.cbSize = sizeof(monitor_info);
			GetMonitorInfoW(monitor, &monitor_info);

			DISPLAY_DEVICEW temp_device{};
			temp_device.cb = sizeof(temp_device);
			last_device = {};
			for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &temp_device, 0); i += 1) {
				if (std::wstring_view(temp_device.DeviceName) == std::wstring_view(monitor_info.szDevice)) {
					last_device = temp_device;
					break;
				}
				temp_device = {};
				temp_device.cb = sizeof(temp_device);
			}
			assert(last_device.cb > 0);

			last_mode = {};
			last_mode.dmSize = sizeof(last_mode);
			EnumDisplaySettingsExW(last_device.DeviceName, ENUM_CURRENT_SETTINGS, &last_mode, 0);

			std::vector<DEVMODEW> modes;
			DEVMODEW temp_mode{};
			temp_mode.dmSize = sizeof(temp_mode);
			for (DWORD i = 0; EnumDisplaySettingsExW(last_device.DeviceName, i, &temp_mode, 0); i += 1) {
				modes.push_back(temp_mode);
				temp_mode = {};
				temp_mode.dmSize = sizeof(temp_mode);
			}

			auto is_same_aspect_ratio = [this](DEVMODEW const& m) -> bool
			{
				DWORD const width = m.dmPelsWidth * last_mode.dmPelsHeight / m.dmPelsHeight;
				return 2 >= std::abs(static_cast<int>(width) - static_cast<int>(last_mode.dmPelsWidth));
			};

			auto is_size_larger = [&](DEVMODEW const& m) -> bool
			{
				return m.dmPelsWidth >= width && m.dmPelsHeight >= height;
			};

		#define u_s2 .

			auto is_same_mode_basic = [](DEVMODEW const& l, DEVMODEW const& r) -> bool
			{
				return l u_s2 dmPosition.x == r u_s2 dmPosition.x
					&& l u_s2 dmPosition.y == r u_s2 dmPosition.y
					&& l u_s2 dmDisplayOrientation == r u_s2 dmDisplayOrientation
					&& l.dmBitsPerPel == r.dmBitsPerPel
					&& l.dmPelsWidth == r.dmPelsWidth
					&& l.dmPelsHeight == r.dmPelsHeight
					&& l.dmDisplayFrequency == r.dmDisplayFrequency
					;
			};

			auto is_auto_scaling = [](DEVMODEW const& m) -> bool
			{
				return m u_s2 dmDisplayFixedOutput == DMDFO_DEFAULT;
			};

		#undef u_s2

			auto is_high_refresh_rate = [](DEVMODEW const& m) -> bool
			{
				return m.dmDisplayFrequency >= 58;
			};

			for (auto it = modes.begin(); it != modes.end();) {
				if (is_same_aspect_ratio(*it)) {
					it++;
				}
				else {
					it = modes.erase(it);
				}
			}

			for (auto it = modes.begin(); it != modes.end();) {
				if (is_size_larger(*it)) {
					it++;
				}
				else {
					it = modes.erase(it);
				}
			}

			for (auto it = modes.begin(); it != modes.end();) {
				if (is_auto_scaling(*it)) {
					it++;
				}
				else {
					it = modes.erase(it);
				}
			}

			for (auto it = modes.begin(); it != modes.end();) {
				if (it->dmBitsPerPel >= 32) {
					it++;
				}
				else {
					it = modes.erase(it);
				}
			}

			std::ranges::sort(modes, [width, height](DEVMODEW const& l, DEVMODEW const& r) -> bool
				{
					double const s0 = double(width) * double(height);
					double const sl = double(l.dmPelsWidth) * double(l.dmPelsHeight);
					double const sr = double(r.dmPelsWidth) * double(r.dmPelsHeight);
					double const vl = sl / s0;
					double const vr = sr / s0;
					if (vl != vr) {
						return vl < vr;
					}
					else {
						return l.dmDisplayFrequency > r.dmDisplayFrequency;
					}
				});

			if (!modes.empty()) {
				ChangeDisplaySettingsExW(last_device.DeviceName, &modes.at(0), nullptr, 0, nullptr);
				is_scope = true;
			}
		}
		void Leave()
		{
			if (is_scope) {
				is_scope = false;
				ChangeDisplaySettingsExW(last_device.DeviceName, &last_mode, nullptr, 0, nullptr);
			}
		}
	public:
		DisplayModeUpdater() = default;
		~DisplayModeUpdater()
		{
			Leave();
		}
	};

	class SwapChain_D3D11
		: public Object<ISwapChain>
		, public IWindowEventListener
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Window_Win32> m_window;
		ScopeObject<Device_D3D11> m_device;
		Direct3D11::LetterBoxingRenderer m_scaling_renderer;
		DisplayModeUpdater m_display_mode_updater;

		Microsoft::WRL::Wrappers::Event dxgi_swapchain_event;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain;
		DXGI_SWAP_CHAIN_DESC1 m_swap_chain_info{};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_swap_chain_fullscreen_info{};
		BOOL m_swap_chain_fullscreen_mode{ FALSE };
		BOOL m_swap_chain_vsync{ FALSE };

		BOOL m_swapchain_want_present_reset{ FALSE };

		BOOL m_init{ FALSE };

		bool m_modern_swap_chain_available{ false };
		bool m_disable_exclusive_fullscreen{ false };
		bool m_disable_composition{ false };
		bool m_enable_composition{ false };

	private:
		void onDeviceCreate();
		void onDeviceDestroy();
		void onWindowCreate();
		void onWindowDestroy();
		void onWindowActive();
		void onWindowInactive();
		void onWindowSize(Core::Vector2U size);
		void onWindowFullscreenStateChange(bool state);

	private:
		void destroySwapChain();
		bool createSwapChain(bool fullscreen, DXGI_MODE_DESC1 const& mode, bool no_attachment);
		void waitFrameLatency(uint32_t timeout, bool reset);
		bool enterExclusiveFullscreenTemporarily();
		bool leaveExclusiveFullscreenTemporarily();
		bool enterExclusiveFullscreen();
		bool leaveExclusiveFullscreen();

	private:
		bool m_is_composition_mode{ false };
		Platform::RuntimeLoader::DirectComposition dcomp_loader;
		Microsoft::WRL::ComPtr<IDCompositionDesktopDevice> dcomp_desktop_device;
		Microsoft::WRL::ComPtr<IDCompositionTarget> dcomp_target;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dcomp_visual_root;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dcomp_visual_background;
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dcomp_visual_swap_chain;
		Microsoft::WRL::ComPtr<IDCompositionSurface> dcomp_surface_background;
	private:
		bool createDirectCompositionResources();
		void destroyDirectCompositionResources();
		bool updateDirectCompositionTransform();
		bool commitDirectComposition();
		bool createCompositionSwapChain(Vector2U size, bool latency_event);

	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_swap_chain_d3d11_rtv;
		Vector2U m_canvas_size{ 640,480 };
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_canvas_d3d11_srv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_canvas_d3d11_rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_canvas_d3d11_dsv;
	private:
		bool createSwapChainRenderTarget();
		void destroySwapChainRenderTarget();
		bool createCanvasColorBuffer();
		void destroyCanvasColorBuffer();
		bool createCanvasDepthStencilBuffer();
		void destroyCanvasDepthStencilBuffer();
		bool createRenderAttachment();
		void destroyRenderAttachment();

	private:
		bool updateLetterBoxingRendererTransform();
		bool presentLetterBoxingRenderer();

	private:
		bool handleDirectCompositionWindowSize(Vector2U size);
		bool handleSwapChainWindowSize(Vector2U size);

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

		bool setWindowMode(Vector2U size);
		bool setCompositionWindowMode(Vector2U size);

		bool setCanvasSize(Vector2U size);
		Vector2U getCanvasSize() { return m_canvas_size; }

		void clearRenderAttachment();
		void applyRenderAttachment();
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
