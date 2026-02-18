#pragma once
#include "core/SwapChain.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/GraphicsDevice.hpp"
#include "d3d11/LetterBoxingRenderer.hpp"
#include "d3d11/SecondarySwapChain.hpp"
#include "windows/RuntimeLoader/DirectComposition.hpp"
#include <wil/resource.h>

namespace core {
	struct SwapChainSharedResources {
		// dependencies

		SmartReference<IWindow> window;
		SmartReference<GraphicsDevice> device;

		// basic

		DXGI_SWAP_CHAIN_DESC1 swap_chain_info{};
		win32::com_ptr<IDXGISwapChain1> swap_chain;
		win32::com_ptr<ID3D11RenderTargetView> swap_chain_rtv;
		bool vsync{};
		bool initialized{};

		// exclusive fullscreen

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_chain_fullscreen_info{};

		// frame latency waitable object

		wil::unique_event_nothrow frame_latency_event;

		// canvas

		Vector2U canvas_size{ 640, 480 };
		win32::com_ptr<ID3D11ShaderResourceView> canvas_srv;
		win32::com_ptr<ID3D11RenderTargetView> canvas_rtv;
		win32::com_ptr<ID3D11DepthStencilView> canvas_dsv;
		d3d11::LetterBoxingRenderer scaling_renderer;
		SwapChainScalingMode scaling_mode{ SwapChainScalingMode::aspect_ratio };

		// DirectComposition

		win32::com_ptr<IDCompositionDesktopDevice> composition_device;
		win32::com_ptr<IDCompositionTarget> composition_target;
		win32::com_ptr<IDCompositionVisual2> composition_visual_root;
		win32::com_ptr<IDCompositionVisual2> composition_visual_swap_chain;

		// Direct2D

#ifdef LUASTG_ENABLE_DIRECT2D
		win32::com_ptr<ID2D1Bitmap1> swap_chain_bitmap;
#endif

		// custom title bar

#ifdef LUASTG_ENABLE_DIRECT2D
		win32::com_ptr<IDCompositionVisual2> composition_visual_title_bar;
		SecondarySwapChain swap_chain_title_bar;
		bool is_title_bar_attached{};
#endif
	};

	struct SwapChainModelLegacy {
	};

	struct SwapChainModelModern {
	};

	struct SwapChainModelComposition {
	};

	class SwapChain_D3D11
		: public implement::ReferenceCounted<ISwapChain>
		, public IWindowEventListener
		, public IGraphicsDeviceEventListener
	{
	private:
		/* X */ SmartReference<IWindow> m_window;
		/* X */ SmartReference<GraphicsDevice> m_device;
		/* X */ d3d11::LetterBoxingRenderer m_scaling_renderer;

		/* X */ wil::unique_event_nothrow dxgi_swapchain_event;
		/* X */ win32::com_ptr<IDXGISwapChain1> dxgi_swapchain;
		/* X */ DXGI_SWAP_CHAIN_DESC1 m_swap_chain_info{};
		/* X */ DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_swap_chain_fullscreen_info{};
		BOOL m_swap_chain_fullscreen_mode{ FALSE };
		/* X */ BOOL m_swap_chain_vsync{ FALSE };

		BOOL m_swapchain_want_present_reset{ FALSE };

		/* X */ BOOL m_init{ FALSE };

		bool m_modern_swap_chain_available{ false };
		bool m_disable_modern_swap_chain{ false };
		bool m_disable_exclusive_fullscreen{ false };
		bool m_disable_composition{ false };
		bool m_enable_composition{ false };

		/* X */ SwapChainScalingMode m_scaling_mode{ SwapChainScalingMode::aspect_ratio };

	public:
		void onGraphicsDeviceCreate() override;
		void onGraphicsDeviceDestroy() override;
		void onWindowCreate() override;
		void onWindowDestroy() override;
		void onWindowActive() override;
		void onWindowInactive() override;
		void onWindowSize(core::Vector2U size) override;
		void onWindowFullscreenStateChange(bool state) override;

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
		/* X */ win32::com_ptr<IDCompositionDesktopDevice> dcomp_desktop_device;
		/* X */ win32::com_ptr<IDCompositionTarget> dcomp_target;
		/* X */ win32::com_ptr<IDCompositionVisual2> dcomp_visual_root;
		/* X */ win32::com_ptr<IDCompositionVisual2> dcomp_visual_swap_chain;
#ifdef LUASTG_ENABLE_DIRECT2D
		/* X */ win32::com_ptr<IDCompositionVisual2> dcomp_visual_title_bar;
		/* X */ SecondarySwapChain swap_chain_title_bar;
		/* X */ bool m_title_bar_attached{ false };
#endif
	private:
		bool createDirectCompositionResources();
		void destroyDirectCompositionResources();
		bool updateDirectCompositionTransform();
		bool commitDirectComposition();
		bool createCompositionSwapChain(Vector2U size, bool latency_event);

	private:
		/* X */ win32::com_ptr<ID3D11RenderTargetView> m_swap_chain_d3d11_rtv;
#ifdef LUASTG_ENABLE_DIRECT2D
		/* X */ win32::com_ptr<ID2D1Bitmap1> m_swap_chain_d2d1_bitmap;
#endif
		/* X */ Vector2U m_canvas_size{ 640,480 };
		/* X */ win32::com_ptr<ID3D11ShaderResourceView> m_canvas_d3d11_srv;
		/* X */ win32::com_ptr<ID3D11RenderTargetView> m_canvas_d3d11_rtv;
		/* X */ win32::com_ptr<ID3D11DepthStencilView> m_canvas_d3d11_dsv;
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
		bool m_is_dispatching_event{};
		std::vector<ISwapChainEventListener*> m_event_listeners;
		void dispatchEvent(EventType e);
	public:
		void addEventListener(ISwapChainEventListener* listener);
		void removeEventListener(ISwapChainEventListener* listener);

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
		inline bool getVSync() { return m_swap_chain_vsync; }
		bool present();

		bool saveSnapshotToFile(StringView path);

	public:
		SwapChain_D3D11(IWindow* p_window, GraphicsDevice* p_device);
		~SwapChain_D3D11();
	};
}
