#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/Graphics/Direct3D11/LetterBoxingRenderer.hpp"
#include "windows/RuntimeLoader/DirectComposition.hpp"

namespace core::Graphics
{
	class SecondarySwapChain {
	private:
		DXGI_SWAP_CHAIN_DESC1 info{};
		wil::com_ptr_nothrow<IDXGIFactory2> dxgi_factory;
		wil::com_ptr_nothrow<ID3D11Device> d3d11_device;
		wil::com_ptr_nothrow<ID3D11DeviceContext> d3d11_device_context;
#ifdef LUASTG_ENABLE_DIRECT2D
		wil::com_ptr_nothrow<ID2D1DeviceContext> d2d1_device_context;
#endif
		wil::com_ptr_nothrow<IDXGISwapChain1> dxgi_swap_chain;
		wil::com_ptr_nothrow<ID3D11RenderTargetView> d3d11_rtv;
#ifdef LUASTG_ENABLE_DIRECT2D
		wil::com_ptr_nothrow<ID2D1Bitmap1> d2d1_bitmap;
#endif
	private:
		bool createRenderAttachment();
		void destroyRenderAttachment();
	public:
		inline IDXGISwapChain1* GetDXGISwapChain1() { return dxgi_swap_chain.get(); }
#ifdef LUASTG_ENABLE_DIRECT2D
		inline ID2D1Bitmap1* GetD2D1Bitmap1() { return d2d1_bitmap.get(); }
#endif
	public:
		bool create(IDXGIFactory2* factory, ID3D11Device* device, ID2D1DeviceContext* context, Vector2U const& size);
		void destroy();
		bool setSize(Vector2U const& size);
		inline Vector2U getSize() const noexcept { return { info.Width, info.Height }; }
		void clearRenderTarget();
		bool present();
	};

	class SwapChain_D3D11
		: public implement::ReferenceCounted<ISwapChain>
		, public IWindowEventListener
		, public IDeviceEventListener
	{
	private:
		SmartReference<IWindow> m_window;
		SmartReference<Direct3D11::Device> m_device;
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
		bool m_disable_modern_swap_chain{ false };
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
		void onWindowSize(core::Vector2U size);
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
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dcomp_visual_swap_chain;
#ifdef LUASTG_ENABLE_DIRECT2D
		Microsoft::WRL::ComPtr<IDCompositionVisual2> dcomp_visual_title_bar;
		SecondarySwapChain swap_chain_title_bar;
		bool m_title_bar_attached{ false };
#endif
	private:
		bool createDirectCompositionResources();
		void destroyDirectCompositionResources();
		bool updateDirectCompositionTransform();
		bool commitDirectComposition();
		bool createCompositionSwapChain(Vector2U size, bool latency_event);

	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_swap_chain_d3d11_rtv;
#ifdef LUASTG_ENABLE_DIRECT2D
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_swap_chain_d2d1_bitmap;
#endif
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
		inline bool getVSync() { return m_swap_chain_vsync; }
		bool present();

		bool saveSnapshotToFile(StringView path);

	public:
		SwapChain_D3D11(IWindow* p_window, Direct3D11::Device* p_device);
		~SwapChain_D3D11();
	public:
		static bool create(IWindow* p_window, Direct3D11::Device* p_device, SwapChain_D3D11** pp_swapchain);
	};
}
