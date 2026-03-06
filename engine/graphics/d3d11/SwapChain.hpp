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
    enum class SwapChainModel : int8_t {
        legacy,
        modern,
        composition,
    };

    class SwapChain : public implement::ReferenceCounted<ISwapChain> , public IWindowEventListener , public IGraphicsDeviceEventListener {
    public:
        // ISwapChain

        void addEventListener(ISwapChainEventListener* listener) override;
        void removeEventListener(ISwapChainEventListener* listener) override;

        bool setWindowMode(Vector2U size) override;
        bool setCanvasSize(Vector2U size) override;
        Vector2U getCanvasSize() override;

        void setScalingMode(SwapChainScalingMode mode) override;
        SwapChainScalingMode getScalingMode() override;

        void clearRenderAttachment() override;
        void applyRenderAttachment() override;
        void waitFrameLatency() override;
        void setVSync(bool enable) override;
        bool getVSync() override;
        bool present() override;

        bool saveSnapshotToFile(StringView path) override;

        // IWindowEventListener

        void onWindowActive() override;
        void onWindowInactive() override;
        void onWindowSize(Vector2U size) override;
        void onWindowFullscreenStateChange(bool state) override;

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // SwapChain

        SwapChain();
        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) = delete;
        ~SwapChain();

        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain& operator=(SwapChain&&) = delete;

        bool initialize(IWindow* window, IGraphicsDevice* device);

    private:
        // event dispatcher

        enum class Event {
            create,
            destroy,
        };

        void dispatchEvent(Event e);

        // basic

        bool createSwapChain();
        void destroySwapChain();
        bool resizeSwapChain(Vector2U size);
        bool createRenderTarget();
        void destroyRenderTarget();

        // exclusive fullscreen

        bool enterExclusiveFullscreenTemporarily();
        bool leaveExclusiveFullscreenTemporarily();
        bool enterExclusiveFullscreen();
        bool leaveExclusiveFullscreen();

        // canvas

        bool createCanvas();
        void destroyCanvas();
        bool isRenderTargetAndCanvasSizeEquals() const noexcept;
        bool updateLetterBoxingTransform();

        // DirectComposition

        bool createComposition();
        void destroyComposition();
        bool commitComposition();
        bool updateCompositionTransform();

        // custom title bar

        bool presentTitleBar();

        // dependencies

        SmartReference<IWindow> m_window;
        SmartReference<IGraphicsDevice> m_device;

        // event dispatcher

        std::vector<ISwapChainEventListener*> m_event_listeners;
        bool m_is_dispatching_event{};

        // basic

        DXGI_SWAP_CHAIN_DESC1 m_swap_chain_info{};
        win32::com_ptr<IDXGISwapChain1> m_swap_chain;
        win32::com_ptr<ID3D11RenderTargetView> m_swap_chain_rtv;
        bool m_vsync{};
        SwapChainModel m_model{};
        bool m_allow_modern_swap_chain{};
        bool m_modern_swap_chain_available{};
        bool m_initialized{};

        // exclusive fullscreen

        DXGI_MODE_DESC1 m_swap_chain_fullscreen_display_mode{};
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_swap_chain_fullscreen_info{};
        bool m_allow_exclusive_fullscreen{};
        bool m_exclusive_fullscreen{};
        bool m_resize_to_window{};
        bool m_resize_to_display_mode{};

        // frame latency waitable object

        wil::unique_event_nothrow m_frame_latency_event;

        // canvas

        Vector2U m_canvas_size{ 640, 480 };
        win32::com_ptr<ID3D11ShaderResourceView> m_canvas_srv;
        win32::com_ptr<ID3D11RenderTargetView> m_canvas_rtv;
        win32::com_ptr<ID3D11DepthStencilView> m_canvas_dsv;
        d3d11::LetterBoxingRenderer m_scaling_renderer;
        SwapChainScalingMode m_scaling_mode{ SwapChainScalingMode::aspect_ratio };

        // DirectComposition

        win32::com_ptr<IDCompositionDesktopDevice> m_composition_device;
        win32::com_ptr<IDCompositionTarget> m_composition_target;
        win32::com_ptr<IDCompositionVisual2> m_composition_visual_root;
        win32::com_ptr<IDCompositionVisual2> m_composition_visual_swap_chain;
        bool m_force_composition{};
        bool m_allow_composition{};

        // Direct2D

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<ID2D1Bitmap1> m_swap_chain_bitmap;
#endif

        // custom title bar

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<IDCompositionVisual2> m_composition_visual_title_bar;
        SecondarySwapChain m_swap_chain_title_bar;
        bool m_is_title_bar_attached{};
#endif
    };
}
