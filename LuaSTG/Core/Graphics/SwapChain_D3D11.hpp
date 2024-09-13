#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/Direct3D11/LetterBoxingRenderer.hpp"
#include "Platform/RuntimeLoader/DirectComposition.hpp"

namespace Core::Graphics
{
	class SwapChain_D3D11
		: public Object<ISwapChain>
		, public IWindowEventListener
		, public IDeviceEventListener
	{
	private:
		ScopeObject<Window_Win32> m_window;
		ScopeObject<Device_D3D11> m_device;
		Direct3D11::LetterBoxingRenderer m_scaling_renderer;

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

		SwapChainScalingMode m_scaling_mode{ SwapChainScalingMode::AspectRatio };

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
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_swap_chain_d2d1_bitmap;
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

		void setScalingMode(SwapChainScalingMode mode);
		SwapChainScalingMode getScalingMode() { return m_scaling_mode; }

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
