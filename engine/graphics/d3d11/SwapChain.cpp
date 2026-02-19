#include "d3d11/SwapChain.hpp"
#include "windows/Window.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include "core/Application.hpp"
#include "d3d11/GraphicsDeviceManager.hpp"
#include "d3d11/DisplayModeHelper.hpp"
#include "d3d11/ToStringHelper.hpp"
#include "windows/WindowsVersion.hpp"
#include "windows/DesktopWindowManager.hpp"
#include "windows/Direct3D11.hpp"
#include "windows/DXGI.hpp"
#include "windows/RuntimeLoader/D3DKMT.hpp"
#include "utf8.hpp"

#include "ScreenGrab11.h"

//#define LOG_INFO(x) core::Logger::info(x)
#define LOG_INFO(x)

#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define HRCheckCallReport(x) if (FAILED(hr)) { core::Logger::error("Windows API failed: " x); }
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { core::Logger::error("Windows API failed: " x); assert(false); return false; }

#define NTNew NTSTATUS nt{};
#define NTGet nt
#define NTCheckCallReport(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); }
#define NTCheckCallReportRuntime(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: {}", x); }
#define NTCheckCallReturnBool(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); assert(false); return false; }
#define NTCheckCallNoAssertReturnBool(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); return false; }

namespace {
    using std::string_view_literals::operator ""sv;
    using std::string_literals::operator ""s;

    constexpr DXGI_FORMAT const COLOR_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;
    constexpr DXGI_FORMAT const DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool makeLetterboxing(const core::Vector2U rect, const core::Vector2U inner_rect, D2D1_MATRIX_3X2_F& mat)
    {
        if (rect == inner_rect)
        {
            return false; // 不需要
        }
        else
        {
            double const hscale = (double)rect.x / (double)inner_rect.x;
            double const vscale = (double)rect.y / (double)inner_rect.y;
            double const scale = std::min(hscale, vscale);
            double const width = scale * (double)inner_rect.x;
            double const height = scale * (double)inner_rect.y;
            double const x = ((double)rect.x - width) * 0.5;
            double const y = ((double)rect.y - height) * 0.5;
            mat = D2D1_MATRIX_3X2_F{
                FLOAT(scale), 0.0f,
                0.0f, FLOAT(scale),
                FLOAT(x), FLOAT(y),
            };
            return true;
        }
    }

    bool checkModernSwapChainModelAvailable(ID3D11Device* const device)
    {
        // 是否需要统一开启现代交换链模型
        // 我们划定一个红线，红线以下永远不开启，红线以上永远开启
        // 这样可以简化逻辑的处理

        assert(device);
        HRNew;
        NTNew;

        // 预检系统版本 Windows 10 1809
        // * DXGI_SWAP_EFFECT_FLIP_DISCARD 从 Windows 10 开始支持
        // * 在 Windows 10 1709 (16299) Fall Creators Update 中修复了
        //   Frame Latency Waitable Object 和 SetMaximumFrameLatency 实际上至少有 2 帧的问题
        // * Windows 10 1809 (17763) 是目前微软还在支持的长期支持版本
        //   https://learn.microsoft.com/en-us/windows/release-health/release-information
        // * DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT 从 Windows 8.1 开始支持，我们跳过它

        if (!Platform::WindowsVersion::Is10Build17763()) {
            return false;
        }

        // 检查 PresentAllowTearing
        // * DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING 从 Windows 10 开始支持
        // * DXGI_PRESENT_ALLOW_TEARING 从 Windows 10 开始支持
        // * 此外还有别的要求，比如WDDM支持、MPO支持、显卡驱动支持等
        // * 注意，就算报告支持，实际运行的时候可能仍然不允许撕裂
        // * 系统安装了 KB3156421 更新，也就是 Windows 10 在 2016 年 5 月 10 日的更新
        //   https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        HRGet = Platform::Direct3D11::GetDeviceFactory(device, dxgi_factory.put());
        HRCheckCallReturnBool("Platform::Direct3D11::GetDeviceFactory");
        BOOL present_allow_tearing = Platform::DXGI::CheckFeatureSupportPresentAllowTearing(dxgi_factory.get());

        // 检查 DirectFlip

        win32::com_ptr<IDXGIAdapter1> dxgi_adapter;
        HRGet = Platform::Direct3D11::GetDeviceAdater(device, dxgi_adapter.put());
        HRCheckCallReturnBool("Platform::Direct3D11::GetDeviceAdater");
        DXGI_ADAPTER_DESC1 dxgi_adapter_info{};
        HRGet = dxgi_adapter->GetDesc1(&dxgi_adapter_info);
        HRCheckCallReturnBool("IDXGIAdapter1::GetDesc1");

        Platform::RuntimeLoader::D3DKMT d3dkmt;

        D3DKMT_OPENADAPTERFROMLUID open_adapter_from_luid{};
        open_adapter_from_luid.AdapterLuid = dxgi_adapter_info.AdapterLuid;
        NTGet = d3dkmt.OpenAdapterFromLuid(&open_adapter_from_luid);
        NTCheckCallReturnBool("D3DKMTOpenAdapterFromLuid");

        auto auto_close_adapter = wil::scope_exit([&open_adapter_from_luid, &d3dkmt]
            {
                D3DKMT_CLOSEADAPTER close_adapter{};
                close_adapter.hAdapter = open_adapter_from_luid.hAdapter;
                NTNew;
                NTGet = d3dkmt.CloseAdapter(&close_adapter);
                NTCheckCallReport("D3DKMTCloseAdapter");
            });
        
        D3DKMT_CREATEDEVICE create_device{};
        create_device.hAdapter = open_adapter_from_luid.hAdapter;
        NTGet = d3dkmt.CreateDevice(&create_device);
        NTCheckCallReturnBool("D3DKMTCreateDevice");
        
        auto auto_close_device = wil::scope_exit([&create_device, &d3dkmt]
            {
                D3DKMT_DESTROYDEVICE destroy_device{};
                destroy_device.hDevice = create_device.hDevice;
                NTNew;
                NTGet = d3dkmt.DestroyDevice(&destroy_device);
                NTCheckCallReport("D3DKMTDestroyDevice");
            });

        auto query_adapter_info = [&open_adapter_from_luid, &d3dkmt]<typename T>(KMTQUERYADAPTERINFOTYPE type, T, std::string_view type_name) -> T {
            T data{};
            D3DKMT_QUERYADAPTERINFO query{};
            query.hAdapter = open_adapter_from_luid.hAdapter;
            query.Type = type;
            query.pPrivateDriverData = &data;
            query.PrivateDriverDataSize = sizeof(data);
            NTNew;
            NTGet = d3dkmt.QueryAdapterInfo(&query);
            NTCheckCallReportRuntime(std::string("D3DKMTQueryAdapterInfo -> ").append(type_name));
            return data;
        };

        // wddm 1.2
        auto const direct_flip_caps = query_adapter_info(KMTQAITYPE_DIRECTFLIP_SUPPORT, D3DKMT_DIRECTFLIP_SUPPORT{}, "D3DKMT_DIRECTFLIP_SUPPORT");
        // wddm 2.0
        auto const independent_flip_support = query_adapter_info(KMTQAITYPE_INDEPENDENTFLIP_SUPPORT, D3DKMT_INDEPENDENTFLIP_SUPPORT{}, "D3DKMT_INDEPENDENTFLIP_SUPPORT");

        // 打印信息

        auto bool_to_string = [](bool v) { return v ? "true" : "false"; };

        core::Logger::info(
            "[core] Graphics Device: {}\n"
            "    Direct Flip Support: {}\n"
            "    Independent Flip Support: {}\n"
            "    Present Allow Tearing: {}"
            , utf8::to_string(dxgi_adapter_info.Description)
            , bool_to_string(direct_flip_caps.Supported)
            , bool_to_string(independent_flip_support.Supported)
            , bool_to_string(present_allow_tearing)
        );

        return present_allow_tearing && direct_flip_caps.Supported && independent_flip_support.Supported;
    }
    bool checkMultiPlaneOverlaySupport(ID3D11Device* const device)
    {
        // 是否有多平面叠加支持，如果有，就可以提供更好的性能

        assert(device);
        HRNew;
        NTNew;

        // 用户禁用了 MPO 则跳过

        if (Platform::DesktopWindowManager::IsOverlayTestModeExists()) {
            core::Logger::warn("[core] Multi Plane Overlay is disabled by user");
            return false;
        }

        // 检查各个显示输出的支持情况

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        HRGet = Platform::Direct3D11::GetDeviceFactory(device, dxgi_factory.put());
        HRCheckCallReturnBool("Platform::Direct3D11::GetDeviceFactory");

        Platform::RuntimeLoader::D3DKMT d3dkmt;

        win32::com_ptr<IDXGIAdapter1> dxgi_adapter;
        for (UINT adapter_index = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(adapter_index, dxgi_adapter.put())); adapter_index += 1) {
            DXGI_ADAPTER_DESC1 dxgi_adapter_info{};
            HRGet = dxgi_adapter->GetDesc1(&dxgi_adapter_info);
            HRCheckCallReturnBool("IDXGIAdapter1::GetDesc1");

            win32::com_ptr<IDXGIOutput> dxgi_output;
            for (UINT output_index = 0; SUCCEEDED(dxgi_adapter->EnumOutputs(output_index, dxgi_output.put())); output_index += 1) {
                DXGI_OUTPUT_DESC dxgi_output_info{};
                HRGet =  dxgi_output->GetDesc(&dxgi_output_info);
                HRCheckCallReturnBool("IDXGIOutput::GetDesc");

                D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME open_adapter_from_gdi{};
                std::memcpy(open_adapter_from_gdi.DeviceName, dxgi_output_info.DeviceName, sizeof(dxgi_output_info.DeviceName));
                NTGet = d3dkmt.OpenAdapterFromGdiDisplayName(&open_adapter_from_gdi);
                NTCheckCallReturnBool("D3DKMTOpenAdapterFromGdiDisplayName");

                auto auto_close_adapter = wil::scope_exit([&open_adapter_from_gdi, &d3dkmt]
                    {
                        D3DKMT_CLOSEADAPTER close_adapter{};
                        close_adapter.hAdapter = open_adapter_from_gdi.hAdapter;
                        NTNew;
                        NTGet = d3dkmt.CloseAdapter(&close_adapter);
                        NTCheckCallReport("D3DKMTCloseAdapter");
                    });

                D3DKMT_CREATEDEVICE create_device{};
                create_device.hAdapter = open_adapter_from_gdi.hAdapter;
                NTGet = d3dkmt.CreateDevice(&create_device);
                NTCheckCallReturnBool("D3DKMTCreateDevice");

                auto auto_close_device = wil::scope_exit([&create_device, &d3dkmt]
                    {
                        D3DKMT_DESTROYDEVICE destroy_device{};
                        destroy_device.hDevice = create_device.hDevice;
                        NTNew;
                        NTGet = d3dkmt.DestroyDevice(&destroy_device);
                        NTCheckCallReport("D3DKMTDestroyDevice");
                    });

                auto query_adapter_info = [&open_adapter_from_gdi, &d3dkmt]<typename T>(KMTQUERYADAPTERINFOTYPE type, T, std::string_view type_name) -> T {
                    T data{};
                    D3DKMT_QUERYADAPTERINFO query{};
                    query.hAdapter = open_adapter_from_gdi.hAdapter;
                    query.Type = type;
                    query.pPrivateDriverData = &data;
                    query.PrivateDriverDataSize = sizeof(data);
                    NTNew;
                    NTGet = d3dkmt.QueryAdapterInfo(&query);
                    NTCheckCallReportRuntime(std::string("D3DKMTQueryAdapterInfo -> ").append(type_name));
                    return data;
                };

                // 检查多平面叠加支持

                auto mpo_support = query_adapter_info(KMTQAITYPE_MULTIPLANEOVERLAY_SUPPORT, D3DKMT_MULTIPLANEOVERLAY_SUPPORT{}, "D3DKMT_MULTIPLANEOVERLAY_SUPPORT");

                // 进一步检查多平面叠加功能支持

                D3DKMT_GET_MULTIPLANE_OVERLAY_CAPS get_mpo_caps{};
                get_mpo_caps.hAdapter = open_adapter_from_gdi.hAdapter;
                get_mpo_caps.VidPnSourceId = open_adapter_from_gdi.VidPnSourceId;
                NTGet = d3dkmt.GetMultiPlaneOverlayCaps(&get_mpo_caps);
                NTCheckCallReturnBool("D3DKMTGetMultiPlaneOverlayCaps");

                // 检查额外的功能
                
                BOOL overlays = FALSE; // 我们只打印信息，但不使用这个值，因为它代表的含义并不清晰
                win32::com_ptr<IDXGIOutput2> dxgi_output2;
                HRGet = dxgi_output->QueryInterface(dxgi_output2.put());
                HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput2");
                if (dxgi_output2)
                {
                    overlays = dxgi_output2->SupportsOverlays();
                }

                UINT hardware_composition_support{};
                win32::com_ptr<IDXGIOutput6> dxgi_output6;
                HRGet = dxgi_output->QueryInterface(dxgi_output6.put());
                HRCheckCallReport("IDXGIOutput::QueryInterface -> IDXGIOutput6");
                if (dxgi_output6)
                {
                    HRGet = dxgi_output6->CheckHardwareCompositionSupport(&hardware_composition_support);
                    HRCheckCallReport("IDXGIOutput6::CheckHardwareCompositionSupport");
                }

                // 打印信息

                auto bool_to_string = [](bool v) { return v ? "true" : "false"; };

                core::Logger::info(
                    "[core] Display Output: {} -> {}\n"
                    "    Overlays Support: {}\n"
                    "    Multi Plane Overlay:\n"
                    "        Support: {}\n"
                    "        Max Planes: {}\n"
                    "        Max RGB Planes: {}\n"
                    "        Max YUV Planes: {}\n"
                    "        Overlay Capabilities:\n"
                    "            Rotation: {}\n"
                    "            Rotation Without Independent Flip: {}\n"
                    "            Vertical Flip: {}\n"
                    "            Horizontal Flip: {}\n"
                    "            Stretch RGB: {}\n"
                    "            Stretch YUV: {}\n"
                    "            Bilinear Filter: {}\n"
                    "            High Filter: {}\n"
                    "            Shared: {}\n"
                    "            Immediate: {}\n"
                    "            Plane 0 For Virtual Mode Only: {}\n"
                    "            Version 3 DDI Support: {}\n"
                    "        Max Stretch Factor: {:.2f}x\n"
                    "        Max Shrink Factor: {:.2f}x\n"
                    "    Hardware Composition:\n"
                    "        Fullscreen: {}\n"
                    "        Windowed: {}\n"
                    "        Cursor Stretched: {}"
                    , utf8::to_string(dxgi_adapter_info.Description)
                    , utf8::to_string(dxgi_output_info.DeviceName)
                    , bool_to_string(overlays)
                    , bool_to_string(mpo_support.Supported)
                    , get_mpo_caps.MaxPlanes
                    , get_mpo_caps.MaxRGBPlanes
                    , get_mpo_caps.MaxYUVPlanes
                    , bool_to_string(get_mpo_caps.OverlayCaps.Rotation)
                    , bool_to_string(get_mpo_caps.OverlayCaps.RotationWithoutIndependentFlip)
                    , bool_to_string(get_mpo_caps.OverlayCaps.VerticalFlip)
                    , bool_to_string(get_mpo_caps.OverlayCaps.HorizontalFlip)
                    , bool_to_string(get_mpo_caps.OverlayCaps.StretchRGB)
                    , bool_to_string(get_mpo_caps.OverlayCaps.StretchYUV)
                    , bool_to_string(get_mpo_caps.OverlayCaps.BilinearFilter)
                    , bool_to_string(get_mpo_caps.OverlayCaps.HighFilter)
                    , bool_to_string(get_mpo_caps.OverlayCaps.Shared)
                    , bool_to_string(get_mpo_caps.OverlayCaps.Immediate)
                    , bool_to_string(get_mpo_caps.OverlayCaps.Plane0ForVirtualModeOnly)
                    , bool_to_string(get_mpo_caps.OverlayCaps.Version3DDISupport)
                    , get_mpo_caps.MaxStretchFactor
                    , get_mpo_caps.MaxShrinkFactor
                    , bool_to_string(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)
                    , bool_to_string(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)
                    , bool_to_string(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED)
                );

                // 多平面叠加不支持

                if (!mpo_support.Supported) {
                    return false;
                }

                // 平面数量少于 2（特别是 RGB 平面），那肯定是不支持了

                if (get_mpo_caps.MaxPlanes < 2 || get_mpo_caps.MaxRGBPlanes < 2) {
                    return false;
                }

                // 看起来似乎不支持缩放

                if (!get_mpo_caps.OverlayCaps.StretchRGB || std::abs(get_mpo_caps.MaxStretchFactor - 1.0f) < 0.1f || std::abs(get_mpo_caps.MaxShrinkFactor - 1.0f) < 0.1f) {
                    return false;
                }

                // TODO: 检测 get_mpo_caps.OverlayCaps.Immediate 的作用

                // 全屏模式的合成不支持

                if (!(hardware_composition_support & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)) {
                    return false;
                }
            }
        }

        // 大概是支持的吧……

        return true;
    }
    bool isModernSwapChainModel(const DXGI_SWAP_CHAIN_DESC1& info)
    {
        return info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            || info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD
            ;
    }
    bool isModernSwapChainModel(IDXGISwapChain1* dxgi_swapchain)
    {
        HRNew;

        DXGI_SWAP_CHAIN_DESC1 info = {};
        HRGet = dxgi_swapchain->GetDesc1(&info);
        HRCheckCallReturnBool("IDXGISwapChain1::GetDesc1");

        return isModernSwapChainModel(info);
    }

    bool setFullscreenState(IDXGISwapChain* const swap_chain, const BOOL fullscreen) {
        BOOL current{};
        if (!win32::check_hresult_as_boolean(
            swap_chain->GetFullscreenState(&current, nullptr),
            "IDXGISwapChain::GetFullscreenState"sv
        )) {
            return false;
        }
        if (current == fullscreen) {
            return true;
        }
        const auto old_update = core::ApplicationManager::isUpdateEnabled();
        const auto old_delegate_update = core::ApplicationManager::isDelegateUpdateEnabled();
        core::ApplicationManager::setUpdateEnabled(false);
        core::ApplicationManager::setDelegateUpdateEnabled(false);
        const HRESULT hr = swap_chain->SetFullscreenState(fullscreen, nullptr);
        core::ApplicationManager::setUpdateEnabled(old_update);
        core::ApplicationManager::setDelegateUpdateEnabled(old_delegate_update);
        return win32::check_hresult_as_boolean(hr, fullscreen
            ? "IDXGISwapChain::SetFullscreenState (TRUE)"sv
            : "IDXGISwapChain::SetFullscreenState (FALSE)"sv
        );
    }
}

namespace core {
    // ISwapChain

    void SwapChain::addEventListener(ISwapChainEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
        m_event_listeners.emplace_back(listener);
    }
    void SwapChain::removeEventListener(ISwapChainEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
    }

    bool SwapChain::setWindowMode(const Vector2U size) {
        if (size.x == 0 || size.y == 0) {
            Logger::error("[core] [SwapChain] size cannot be ({}x{})", size.x, size.y);
            assert(false); return false;
        }

        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        if (m_force_composition || (m_allow_composition && m_allow_modern_swap_chain && m_modern_swap_chain_available && checkMultiPlaneOverlaySupport(device))) {
            m_model = SwapChainModel::composition;
        }
        else if (m_allow_modern_swap_chain && m_modern_swap_chain_available) {
            m_model = SwapChainModel::modern;
        }
        else {
            m_model = SwapChainModel::legacy;
        }

        dispatchEvent(Event::destroy);

        destroySwapChain();
        destroyCanvas();
        m_canvas_size = size;
        if (!createCanvas()) {
            return false;
        }
        if (!createSwapChain()) {
            return false;
        }

        dispatchEvent(Event::create);
        return true;
    }
    bool SwapChain::setCanvasSize(const Vector2U size) {
        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }

        dispatchEvent(Event::destroy);

        destroyCanvas();
        m_canvas_size = size;
        if (!createCanvas()) {
            return false;
        }

        if (!updateLetterBoxingTransform()) {
            return false;
        }
        if (m_model == SwapChainModel::composition) {
            if (!resizeSwapChain(size)) {
                return false;
            }
            if (!updateCompositionTransform()) {
                return false;
            }
        }

        dispatchEvent(Event::create);

        // TODO: LuaSTG 那边会先调用 setCanvasSize 再调用 setWindowMode 触发两次交换链创建
        // update exclusive fullsceen display mode
        if (m_exclusive_fullscreen) {
            leaveExclusiveFullscreen();
            enterExclusiveFullscreen();
        }

        return true;
    }
    Vector2U SwapChain::getCanvasSize() {
        return m_canvas_size;
    }

    void SwapChain::setScalingMode(const SwapChainScalingMode mode) {
        m_scaling_mode = mode;
        updateLetterBoxingTransform();
        updateCompositionTransform();
    }
    SwapChainScalingMode SwapChain::getScalingMode() {
        return m_scaling_mode;
    }

    void SwapChain::clearRenderAttachment() {
        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return;
        }

        ID3D11RenderTargetView* const rtv = isRenderTargetAndCanvasSizeEquals() ? m_swap_chain_rtv.get() : m_canvas_rtv.get();
        if (rtv != nullptr) {
            constexpr FLOAT clear_color[4]{ 0.0f, 0.0f, 0.0f, 1.0f }; // solid black
            ctx->ClearRenderTargetView(rtv, clear_color);
        }

        if (m_canvas_dsv) {
            ctx->ClearDepthStencilView(m_canvas_dsv.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        }
    }
    void SwapChain::applyRenderAttachment() {
        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return;
        }

        ID3D11RenderTargetView* const rtv = isRenderTargetAndCanvasSizeEquals() ? m_swap_chain_rtv.get() : m_canvas_rtv.get();
        ctx->OMSetRenderTargets(1, &rtv, m_canvas_dsv.get());
    }
    void SwapChain::waitFrameLatency() {
        if ((m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) && m_frame_latency_event) {
            const auto result = WaitForSingleObject(m_frame_latency_event.get(), 100);
            if (result != WAIT_OBJECT_0 && result != WAIT_TIMEOUT) {
                win32::check_hresult(HRESULT_FROM_WIN32(GetLastError()), "WaitForSingleObject (m_frame_latency_event, 100)"sv);
            }
        }
    }
    void SwapChain::setVSync(const bool enable) {
        m_vsync = enable;
    }
    bool SwapChain::getVSync() {
        return m_vsync;
    }
    bool SwapChain::present() {
        LOG_INFO("present");

        if (m_resize_to_window) {
            RECT rect{};
            if (!GetClientRect(static_cast<HWND>(m_window->getNativeHandle()), &rect)) {
                win32::check_hresult(HRESULT_FROM_WIN32(GetLastError()), "GetClientRect"sv);
                assert(false); return false;
            }
            if ((rect.right - rect.left) <= 0 || (rect.bottom - rect.top) <= 0) {
                rect.right = std::max(rect.left + 1, rect.right);
                rect.bottom = std::max(rect.top + 1, rect.bottom);
            }
            if (!resizeSwapChain(Vector2U(static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.bottom - rect.top)))) {
                return false;
            }
            m_resize_to_window = false;
            return true;
        }
        if (m_resize_to_display_mode) {
            if (!resizeSwapChain(Vector2U(m_swap_chain_fullscreen_display_mode.Width, m_swap_chain_fullscreen_display_mode.Height))) {
                return false;
            }
            m_resize_to_display_mode = false;
            return true;
        }

        // scaling

        if (!isRenderTargetAndCanvasSizeEquals() && (m_model == SwapChainModel::legacy || m_model == SwapChainModel::modern)) {
            if (!m_scaling_renderer.Draw(
                m_canvas_srv.get(),
                m_swap_chain_rtv.get(),
                true
            )) {
                return false;
            }
        }

        // title bar

        if (!presentTitleBar()) {
            return false;
        }

        // present

        UINT interval{};
        UINT flags{};
        if (isModernSwapChainModel(m_swap_chain_info)) {
            // For DXGI_SWAP_EFFECT_FLIP_DISCARD, the vertical sync interval is always 0.
            flags |= DXGI_PRESENT_DO_NOT_WAIT;
            if (!m_vsync && (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)) {
                // If DXGI_FEATURE_PRESENT_ALLOW_TEARING is supported, variable refresh rate will be enabled.
                flags |= DXGI_PRESENT_ALLOW_TEARING;
            }
        }
        else {
            interval = m_vsync ? 1 : 0;
        }
        const auto present_result = m_swap_chain->Present(interval, flags);

        // clear state

        if (const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle()); ctx != nullptr) {
            ctx->ClearState();
            // TODO: DiscardView
        }

        // check result

        if (present_result == DXGI_ERROR_DEVICE_REMOVED || present_result == DXGI_ERROR_DEVICE_RESET) {
            return static_cast<GraphicsDevice*>(m_device.get())->handleDeviceLost();
        }
        else if (present_result != DXGI_ERROR_WAS_STILL_DRAWING && FAILED(present_result)) {
            win32::check_hresult(present_result, "IDXGISwapChain::Present"sv);
            return false;
        }

        return true;
    }

    bool SwapChain::saveSnapshotToFile(const StringView path) {
        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return false;
        }

        ID3D11RenderTargetView* const rtv = isRenderTargetAndCanvasSizeEquals() ? m_swap_chain_rtv.get() : m_canvas_rtv.get();
        if (rtv == nullptr) {
            assert(false); return false;
        }

        win32::com_ptr<ID3D11Resource> resource;
        rtv->GetResource(resource.put());

        const std::wstring path_wide(utf8::to_wstring(path));
        if (!win32::check_hresult_as_boolean(
            DirectX::SaveWICTextureToFile(
                ctx,
                resource.get(),
                GUID_ContainerFormatJpeg,
                path_wide.c_str(),
                &GUID_WICPixelFormat24bppBGR
            ),
            "DirectX::SaveWICTextureToFile"sv
        )) {
            return false;
        }

        return true;
    }

    // IWindowEventListener

    void SwapChain::onWindowActive() {
        if (!m_swap_chain) {
            return;
        }
        enterExclusiveFullscreenTemporarily();
    }
    void SwapChain::onWindowInactive() {
        if (!m_swap_chain) {
            return;
        }
        leaveExclusiveFullscreenTemporarily();
    }
    void SwapChain::onWindowSize(const Vector2U size) {
        if (size.x == 0 || size.y == 0) {
            return;
        }
        if (!m_swap_chain) {
            return;
        }
        if (m_model == SwapChainModel::legacy || m_model == SwapChainModel::modern) {
            if (resizeSwapChain(size)) {
                m_resize_to_window = false;
                m_resize_to_display_mode = false;
            }
        }
        else if (m_model == SwapChainModel::composition) {
            updateCompositionTransform();
        }
        else {
            assert(false);
        }
    }
    void SwapChain::onWindowFullscreenStateChange(const bool state) {
        if (!m_swap_chain) {
            return;
        }
        if (state) {
            enterExclusiveFullscreen();
        }
        else {
            leaveExclusiveFullscreen();
        }
    }

    // IGraphicsDeviceEventListener

    void SwapChain::onGraphicsDeviceCreate() {
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return;
        }
        m_modern_swap_chain_available = checkModernSwapChainModelAvailable(device);
        if (!m_scaling_renderer.AttachDevice(device)) {
            return;
        }
    #ifdef LUASTG_ENABLE_DIRECT2D
        if (!static_cast<Window*>(m_window.get())->getTitleBarController().createResources(
            static_cast<HWND>(m_window->getNativeHandle()),
            static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle())
        )) {
            return;
        }
    #endif
        if (!setWindowMode(m_canvas_size)) {
            return;
        }
        if (m_exclusive_fullscreen) {
            enterExclusiveFullscreen();
        }
    }
    void SwapChain::onGraphicsDeviceDestroy() {
        destroySwapChain();
        destroyCanvas();
        m_scaling_renderer.DetachDevice();
    #ifdef LUASTG_ENABLE_DIRECT2D
        static_cast<Window*>(m_window.get())->getTitleBarController().destroyResources();
    #endif
    }

    // SwapChain

    SwapChain::SwapChain() = default;
    SwapChain::~SwapChain() {
        if (m_initialized) {
            m_window->removeEventListener(this);
            m_device->removeEventListener(this);
        }
        destroySwapChain();
        destroyCanvas();
        m_scaling_renderer.DetachDevice();
        assert(m_event_listeners.empty());
    }

    bool SwapChain::initialize(IWindow* const window, IGraphicsDevice* const device) {
        if (window == nullptr) {
            assert(false); return false;
        }
        if (device == nullptr) {
            assert(false); return false;
        }
        const auto d3d11_device = static_cast<ID3D11Device*>(device->getNativeHandle());
        if (d3d11_device == nullptr) {
            assert(false); return false;
        }

        m_window = window;
        m_device = device;

        const auto& config = core::ConfigurationLoader::getInstance().getGraphicsSystem();
        m_modern_swap_chain_available = checkModernSwapChainModelAvailable(d3d11_device);
        m_allow_exclusive_fullscreen = config.isAllowExclusiveFullscreen();
        m_allow_modern_swap_chain = config.isAllowModernSwapChain();
        m_allow_composition = config.isAllowDirectComposition();

        const auto size{ Vector2U(config.getWidth(), config.getHeight()) };
        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }

        if (!m_scaling_renderer.AttachDevice(d3d11_device)) {
            return false;
        }
    #ifdef LUASTG_ENABLE_DIRECT2D
        if (!static_cast<Window*>(m_window.get())->getTitleBarController().createResources(
            static_cast<HWND>(m_window->getNativeHandle()),
            static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle())
        )) {
            return false;
        }
    #endif
        if (!setWindowMode(size)) {
            return false;
        }

        m_window->addEventListener(this);
        m_device->addEventListener(this);
        m_initialized = true;
        return true;
    }

    // event dispatcher

    void SwapChain::dispatchEvent(const Event e) {
        assert(!m_is_dispatching_event);
        m_is_dispatching_event = true;

        switch (e) {
        case Event::create:
            for (const auto listener : m_event_listeners) {
                listener->onSwapChainCreate();
            }
            break;
        case Event::destroy:
            for (const auto listener : m_event_listeners) {
                listener->onSwapChainDestroy();
            }
            break;
        }

        m_is_dispatching_event = false;
    }

    // basic

    bool SwapChain::createSwapChain() {
        LOG_INFO("createSwapChain");

        // check

        Logger::info("[core] [SwapChain] creating...");

        RECT rect{};
        if (!GetClientRect(static_cast<HWND>(m_window->getNativeHandle()), &rect)) {
            win32::check_hresult(HRESULT_FROM_WIN32(GetLastError()), "GetClientRect"sv);
            assert(false); return false;
        }
        if ((rect.right - rect.left) <= 0 || (rect.bottom - rect.top) <= 0) {
            rect.right = std::max(rect.left + 1, rect.right);
            rect.bottom = std::max(rect.top + 1, rect.bottom);
        }

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        if (!core::GraphicsDeviceManagerDXGI::refreshAndGetFactory(dxgi_factory.put())) {
            Logger::error("[core] [SwapChain] createSwapChain failed: DXGI not available");
            assert(false); return false;
        }

        // fill swap chain info

        // m_swap_chain_info.Width
        // m_swap_chain_info.Height
        m_swap_chain_info.Format = COLOR_BUFFER_FORMAT;
        m_swap_chain_info.Stereo = FALSE;
        m_swap_chain_info.SampleDesc.Count = 1;
        m_swap_chain_info.SampleDesc.Quality = 0;
        m_swap_chain_info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        // m_swap_chain_info.BufferCount
        m_swap_chain_info.Scaling = DXGI_SCALING_STRETCH;
        // m_swap_chain_info.SwapEffect
        m_swap_chain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        // m_swap_chain_info.Flags

        m_swap_chain_fullscreen_info.RefreshRate = m_swap_chain_fullscreen_display_mode.RefreshRate;
        m_swap_chain_fullscreen_info.ScanlineOrdering = m_swap_chain_fullscreen_display_mode.ScanlineOrdering;
        m_swap_chain_fullscreen_info.Scaling = m_swap_chain_fullscreen_display_mode.Scaling;
        m_swap_chain_fullscreen_info.Windowed = TRUE;

        switch (m_model) {
        case SwapChainModel::legacy:
            if (m_exclusive_fullscreen) {
                m_swap_chain_info.Width = m_swap_chain_fullscreen_display_mode.Width;
                m_swap_chain_info.Height = m_swap_chain_fullscreen_display_mode.Height;
            }
            else {
                m_swap_chain_info.Width = static_cast<UINT>(rect.right - rect.left);
                m_swap_chain_info.Height = static_cast<UINT>(rect.bottom - rect.top);
            }
            m_swap_chain_info.BufferCount = 1;
            m_swap_chain_info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            m_swap_chain_info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            break;
        case SwapChainModel::modern:
            if (m_exclusive_fullscreen) {
                m_swap_chain_info.Width = m_swap_chain_fullscreen_display_mode.Width;
                m_swap_chain_info.Height = m_swap_chain_fullscreen_display_mode.Height;
                m_swap_chain_info.BufferCount = 2;
            }
            else {
                m_swap_chain_info.Width = static_cast<UINT>(rect.right - rect.left);
                m_swap_chain_info.Height = static_cast<UINT>(rect.bottom - rect.top);
                m_swap_chain_info.BufferCount = 3;
            }
            m_swap_chain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            if (m_exclusive_fullscreen) {
                m_swap_chain_info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            }
            else {
                m_swap_chain_info.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            }
            break;
        case SwapChainModel::composition:
            m_swap_chain_info.Width = m_canvas_size.x;
            m_swap_chain_info.Height = m_canvas_size.y;
            m_swap_chain_info.BufferCount = 3;
            m_swap_chain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            m_swap_chain_info.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            break;
        default:
            assert(false); return false;
        }

        // create swap chain

        if (m_model == SwapChainModel::legacy || m_model == SwapChainModel::modern) {
            if (!win32::check_hresult_as_boolean(
                dxgi_factory->CreateSwapChainForHwnd(
                    static_cast<ID3D11Device*>(m_device->getNativeHandle()),
                    static_cast<HWND>(m_window->getNativeHandle()),
                    &m_swap_chain_info,
                    m_exclusive_fullscreen ? &m_swap_chain_fullscreen_info : nullptr,
                    nullptr,
                    m_swap_chain.put()
                ),
                "IDXGIFactory2::CreateSwapChainForHwnd"sv
            )) {
                return false;
            }

            win32::check_hresult(
                Platform::DXGI::MakeSwapChainWindowAssociation(m_swap_chain.get(), DXGI_MWA_NO_ALT_ENTER),
                "IDXGIFactory::MakeWindowAssociation (DXGI_MWA_NO_ALT_ENTER)"sv
            );
        }
        else if (m_model == SwapChainModel::composition) {
            assert(!m_exclusive_fullscreen);

            if (!win32::check_hresult_as_boolean(
                dxgi_factory->CreateSwapChainForComposition(
                    static_cast<ID3D11Device*>(m_device->getNativeHandle()),
                    &m_swap_chain_info,
                    nullptr,
                    m_swap_chain.put()
                ),
                "IDXGIFactory2::CreateSwapChainForComposition"sv
            )) {
                return false;
            }

            if (!createComposition()) {
                return false;
            }
        }
        else {
            assert(false); return false;
        }

        // setup frame latency

        win32::check_hresult(
            Platform::DXGI::SetDeviceMaximumFrameLatency(m_swap_chain.get(), 1),
            "IDXGIDevice1::SetMaximumFrameLatency (1)"sv
        );

        if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) {
            HANDLE event{};
            if (!win32::check_hresult_as_boolean(
                Platform::DXGI::SetSwapChainMaximumFrameLatency(m_swap_chain.get(), 1, &event),
                "IDXGISwapChain2::SetMaximumFrameLatency (1)"sv
            )) {
                return false;
            }
            m_frame_latency_event.reset(event);
        }

        // create render target

        if (!createRenderTarget()) {
            return false;
        }

        // transform

        if (!updateLetterBoxingTransform()) {
            return false;
        }

        // log

        if (m_exclusive_fullscreen) {
            const auto refresh_rate = m_swap_chain_fullscreen_info.RefreshRate;
            const auto refresh_rate_value = static_cast<double>(refresh_rate.Numerator) / static_cast<double>(refresh_rate.Denominator);
            Logger::info(
                "[core] [SwapChain] created (exclusive fullscreen):\n"
                "    Display Mode: {}x{}@{}\n"
                "    Buffer Count: {}\n"
                "    Swap Effect: {}"
                , m_swap_chain_info.Width, m_swap_chain_info.Height, refresh_rate_value
                , m_swap_chain_info.BufferCount
                , toStringView(m_swap_chain_info.SwapEffect)
            );
        }
        else {
            Logger::info(
                "[core] [SwapChain] created:\n"
                "    Display Mode: {}x{}\n"
                "    Buffer Count: {}\n"
                "    Swap Effect: {}\n"
                "    Present Allow Tearing: {}\n"
                "    Frame Latency Waitable Object: {}"
                , m_swap_chain_info.Width, m_swap_chain_info.Height
                , m_swap_chain_info.BufferCount
                , toStringView(m_swap_chain_info.SwapEffect)
                , toStringView((m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) == DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
                , toStringView((m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) == DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
            );
        }

        return true;
    }
    void SwapChain::destroySwapChain() {
        LOG_INFO("destroySwapChain");
        destroyRenderTarget();
        destroyComposition();
        if (m_swap_chain) {
            setFullscreenState(m_swap_chain.get(), FALSE);
        }
        m_swap_chain.reset();
        m_frame_latency_event.reset();
    }
    bool SwapChain::resizeSwapChain(const Vector2U size) {
        LOG_INFO("resizeSwapChain");

        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }
        if (!m_swap_chain) {
            assert(false); return false;
        }

        destroyRenderTarget();

        m_swap_chain_info.Width = size.x;
        m_swap_chain_info.Height = size.y;

        const auto& info = m_swap_chain_info;
        if (!win32::check_hresult_as_boolean(
            m_swap_chain->ResizeBuffers(info.BufferCount, info.Width, info.Height, info.Format, info.Flags),
            "IDXGISwapChain::ResizeBuffers"sv
        )) {
            return false;
        }

        if (!createRenderTarget()) {
            return false;
        }

        if (!updateLetterBoxingTransform()) {
            return false;
        }

        return true;
    }
    bool SwapChain::createRenderTarget() {
        LOG_INFO("createRenderTarget");

        if (!m_swap_chain) {
            assert(false); return false;
        }
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        win32::com_ptr<ID3D11Texture2D> texture;
        if (!win32::check_hresult_as_boolean(
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(texture.put())),
            "IDXGISwapChain::GetBuffer (0, ID3D11Texture2D)"sv
        )) {
            return false;
        }

        // TODO: linear color space
        if (!win32::check_hresult_as_boolean(
            device->CreateRenderTargetView(texture.get(), nullptr, m_swap_chain_rtv.put()),
            "ID3D11Device::CreateRenderTargetView"sv
        )) {
            return false;
        }

#ifdef LUASTG_ENABLE_DIRECT2D
        const auto renderer = static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle());
        if (renderer == nullptr) {
            assert(false); return false;
        }

        win32::com_ptr<IDXGISurface> surface;
        if (!win32::check_hresult_as_boolean(
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(surface.put())),
            "IDXGISwapChain::GetBuffer (0, IDXGISurface)"sv
        )) {
            return false;
        }

        // TODO: linear color space
        D2D1_BITMAP_PROPERTIES1 bitmap_info{};
        bitmap_info.pixelFormat.format = COLOR_BUFFER_FORMAT;
        bitmap_info.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        bitmap_info.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

        if (!win32::check_hresult_as_boolean(
            renderer->CreateBitmapFromDxgiSurface(surface.get(), &bitmap_info, m_swap_chain_bitmap.put()),
            "ID2D1DeviceContext::CreateBitmapFromDxgiSurface"sv
        )) {
            return false;
        }
#endif

        return true;
    }
    void SwapChain::destroyRenderTarget() {
        LOG_INFO("destroyRenderTarget");
    #ifdef LUASTG_ENABLE_DIRECT2D
        if (const auto ctx = static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle()); ctx != nullptr) {
            ctx->SetTarget(nullptr);
        }
        m_swap_chain_bitmap.reset();
    #endif
        if (const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle()); ctx != nullptr) {
            ctx->ClearState();
            ctx->Flush();
        }
        m_swap_chain_rtv.reset();
    }

    // exclusive fullscreen

    bool SwapChain::enterExclusiveFullscreenTemporarily() {
        if (!m_exclusive_fullscreen) {
            return true;
        }
        if (!m_swap_chain) {
            assert(false); return false;
        }
        Logger::info("[core] [SwapChain] enter exclusive fullscreen (temporarily)");
        if (!setFullscreenState(m_swap_chain.get(), TRUE)) {
            return false;
        }
        m_resize_to_window = false;
        m_resize_to_display_mode = true;
        return true;
    }
    bool SwapChain::leaveExclusiveFullscreenTemporarily() {
        if (!m_exclusive_fullscreen) {
            return true;
        }
        if (!m_swap_chain) {
            assert(false); return false;
        }
        Logger::info("[core] [SwapChain] leave exclusive fullscreen (temporarily)");
        const auto result = setFullscreenState(m_swap_chain.get(), FALSE);
        m_window->setLayer(WindowLayer::Normal); // 强制取消窗口置顶
        m_resize_to_window = true;
        m_resize_to_display_mode = false;
        return result;
    }
    bool SwapChain::enterExclusiveFullscreen() {
        LOG_INFO("enterExclusiveFullscreen...");

        if (!m_allow_exclusive_fullscreen) {
            return false;
        }
        if (!m_swap_chain) {
            assert(false); return false;
        }
        if (m_model != SwapChainModel::legacy && m_model != SwapChainModel::modern) {
            return false;
        }

        DXGI_MODE_DESC1 display_mode{};
        if (!d3d11::findBestDisplayMode(m_swap_chain.get(), static_cast<HWND>(m_window->getNativeHandle()), m_canvas_size.x, m_canvas_size.y, display_mode)) {
            return false;
        }

        LOG_INFO("enterExclusiveFullscreen (display mode found)");

        dispatchEvent(Event::destroy);
        destroySwapChain();

        m_window->setSize({ display_mode.Width, display_mode.Height });
        m_swap_chain_fullscreen_display_mode = display_mode;
        m_exclusive_fullscreen = true;

        if (!createSwapChain()) {
            return false;
        }

        LOG_INFO("enterExclusiveFullscreen (enter...)");

        if (!setFullscreenState(m_swap_chain.get(), TRUE)) {
            return false;
        }
        m_resize_to_window = false;
        m_resize_to_display_mode = true;

        dispatchEvent(Event::create);

        return true;
    }
    bool SwapChain::leaveExclusiveFullscreen() {
        LOG_INFO("leaveExclusiveFullscreen...");

        if (!m_swap_chain) {
            assert(false); return false;
        }

        setFullscreenState(m_swap_chain.get(), FALSE);

        LOG_INFO("leaveExclusiveFullscreen");

        if (m_exclusive_fullscreen) {
            dispatchEvent(Event::destroy);
            destroySwapChain();
            m_exclusive_fullscreen = false;
            if (!createSwapChain()) {
                return false;
            }
            dispatchEvent(Event::create);
        }
        return true;
    }

    // canvas

    bool SwapChain::createCanvas() {
        if (m_canvas_size.x == 0 || m_canvas_size.y == 0) {
            assert(false); return false;
        }
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        // color buffer

        D3D11_TEXTURE2D_DESC texture_info{};
        texture_info.Width = m_canvas_size.x;
        texture_info.Height = m_canvas_size.y;
        texture_info.MipLevels = 1;
        texture_info.ArraySize = 1;
        texture_info.Format = COLOR_BUFFER_FORMAT;
        texture_info.SampleDesc.Count = 1;
        texture_info.Usage = D3D11_USAGE_DEFAULT;
        texture_info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        win32::com_ptr<ID3D11Texture2D> texture;
        if (!win32::check_hresult_as_boolean(
            device->CreateTexture2D(&texture_info, NULL, texture.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        // shader resource view

        // TODO: linear color space
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_info{};
        srv_info.Format = texture_info.Format;
        srv_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_info.Texture2D.MipLevels = texture_info.MipLevels;

        if (!win32::check_hresult_as_boolean(
            device->CreateShaderResourceView(texture.get(), &srv_info, m_canvas_srv.put()),
            "ID3D11Device::CreateShaderResourceView"sv
        )) {
            return false;
        }

        // render target view

        // TODO: linear color space
        D3D11_RENDER_TARGET_VIEW_DESC rtv_info{};
        rtv_info.Format = texture_info.Format;
        rtv_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        if (!win32::check_hresult_as_boolean(
            device->CreateRenderTargetView(texture.get(), &rtv_info, m_canvas_rtv.put()),
            "ID3D11Device::CreateRenderTargetView"sv
        )) {
            return false;
        }

        // depth stencil buffer

        D3D11_TEXTURE2D_DESC buffer_info{};
        buffer_info.Width = m_canvas_size.x;
        buffer_info.Height = m_canvas_size.y;
        buffer_info.MipLevels = 1;
        buffer_info.ArraySize = 1;
        buffer_info.Format = DEPTH_BUFFER_FORMAT;
        buffer_info.SampleDesc.Count = 1;
        buffer_info.Usage = D3D11_USAGE_DEFAULT;
        buffer_info.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        win32::com_ptr<ID3D11Texture2D> buffer;
        if (!win32::check_hresult_as_boolean(
            device->CreateTexture2D(&buffer_info, NULL, buffer.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        // depth stencil view

        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_info{};
        dsv_info.Format = buffer_info.Format;
        dsv_info.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        if (!win32::check_hresult_as_boolean(
            device->CreateDepthStencilView(buffer.get(), &dsv_info, m_canvas_dsv.put()),
            "ID3D11Device::CreateDepthStencilView"sv
        )) {
            return false;
        }

        return true;
    }
    void SwapChain::destroyCanvas() {
        if (const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle()); ctx != nullptr) {
            ctx->ClearState();
            ctx->Flush();
        }
        m_canvas_srv.reset();
        m_canvas_rtv.reset();
        m_canvas_dsv.reset();
    }
    bool SwapChain::isRenderTargetAndCanvasSizeEquals() const noexcept {
        return m_swap_chain_info.Width == m_canvas_size.x && m_swap_chain_info.Height == m_canvas_size.y;
    }
    bool SwapChain::updateLetterBoxingTransform() {
        if (!m_scaling_renderer.UpdateTransform(
            m_canvas_srv.get(),
            m_swap_chain_rtv.get(),
            m_scaling_mode == SwapChainScalingMode::stretch
        )) {
            return false;
        }

        return true;
    }

    // DirectComposition

    bool SwapChain::createComposition() {
        // check

        const auto window = static_cast<HWND>(m_window->getNativeHandle());
        if (window == nullptr) {
            Logger::error("[core] [SwapChain] createComposition failed: window not available");
            assert(false); return false;
        }

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        if (!core::GraphicsDeviceManagerDXGI::refreshAndGetFactory(dxgi_factory.put())) {
            Logger::error("[core] [SwapChain] createComposition failed: DXGI not available");
            assert(false); return false;
        }

        // create device

        static Platform::RuntimeLoader::DirectComposition loader;
        if (!win32::check_hresult_as_boolean(
            loader.CreateDevice(nullptr, IID_PPV_ARGS(m_composition_device.put())),
            "DCompositionCreateDevice2"sv
        )) {
            return false;
        }

    #ifndef NDEBUG
        win32::com_ptr<IDCompositionDeviceDebug> device_debug;
        if (SUCCEEDED(m_composition_device->QueryInterface(device_debug.put()))) {
            win32::check_hresult(
                device_debug->EnableDebugCounters(),
                "IDCompositionDeviceDebug::EnableDebugCounters"sv
            );
        }
    #endif

        // create resources

        if (!win32::check_hresult_as_boolean(
            m_composition_device->CreateTargetForHwnd(window, TRUE, m_composition_target.put()),
            "IDCompositionDesktopDevice::CreateTargetForHwnd"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_device->CreateVisual(m_composition_visual_root.put()),
            "IDCompositionDevice2::CreateVisual"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_device->CreateVisual(m_composition_visual_swap_chain.put()),
            "IDCompositionDevice2::CreateVisual"sv
        )) {
            return false;
        }

    #ifdef LUASTG_ENABLE_DIRECT2D
        if (!win32::check_hresult_as_boolean(
            m_composition_device->CreateVisual(m_composition_visual_title_bar.put()),
            "IDCompositionDevice2::CreateVisual"sv
        )) {
            return false;
        }
    #endif

        // set content

        if (m_swap_chain) {
            if (!win32::check_hresult_as_boolean(
                m_composition_visual_swap_chain->SetContent(m_swap_chain.get()),
                "IDCompositionVisual::SetContent"sv
            )) {
                return false;
            }
        }

    #ifdef LUASTG_ENABLE_DIRECT2D
        if (!m_swap_chain_title_bar.create(
            dxgi_factory.get(),
            static_cast<ID3D11Device*>(m_device->getNativeHandle()),
            static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle()),
            m_window->_getCurrentSize()
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_visual_title_bar->SetContent(m_swap_chain_title_bar.getSwapChain1()),
            "IDCompositionVisual::SetContent"sv
        )) {
            return false;
        }

        // NOTE: Only add it to the visual tree when necessary.
    #endif

        // build visual tree

        if (!win32::check_hresult_as_boolean(
            m_composition_target->SetRoot(m_composition_visual_root.get()),
            "IDCompositionTarget::SetRoot"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_visual_root->AddVisual(m_composition_visual_swap_chain.get(), TRUE, nullptr),
            "IDCompositionVisual::AddVisual"sv
        )) {
            return false;
        }

        return updateCompositionTransform();
    }
    void SwapChain::destroyComposition() {
        // destroy visual tree

        if (m_composition_target) {
            win32::check_hresult(
                m_composition_target->SetRoot(nullptr),
                "IDCompositionTarget::SetRoot (null)"sv
            );
        }
        if (m_composition_visual_root) {
            win32::check_hresult(
                m_composition_visual_root->RemoveAllVisuals(),
                "IDCompositionVisual::RemoveAllVisuals"sv
            );
            win32::check_hresult(
                m_composition_visual_root->SetContent(nullptr),
                "IDCompositionVisual::SetContent (null)"sv
            );
        }
    #ifdef LUASTG_ENABLE_DIRECT2D
        if (m_composition_visual_swap_chain) {
            win32::check_hresult(
                m_composition_visual_swap_chain->SetContent(nullptr),
                "IDCompositionVisual::SetContent (null)"sv
            );
        }
        if (m_composition_visual_title_bar) {
            win32::check_hresult(
                m_composition_visual_title_bar->SetContent(nullptr),
                "IDCompositionVisual::SetContent (null)"sv
            );
        }
        m_swap_chain_title_bar.destroy();
    #endif

        // destroy resources

        m_composition_target.reset();
        m_composition_visual_root.reset();
    #ifdef LUASTG_ENABLE_DIRECT2D
        m_composition_visual_swap_chain.reset();
        m_composition_visual_title_bar.reset();
        m_is_title_bar_attached = false;
    #endif

        // commit

        if (m_composition_device) {
            commitComposition();
        }

        // destroy device

        m_composition_device.reset();
    }
    bool SwapChain::commitComposition() {
        if (!win32::check_hresult_as_boolean(
            m_composition_device->Commit(),
            "IDCompositionDevice2::Commit"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_device->WaitForCommitCompletion(),
            "IDCompositionDevice2::WaitForCommitCompletion"sv
        )) {
            return false;
        }

        return true;
    }
    bool SwapChain::updateCompositionTransform() {
        LOG_INFO("updateCompositionTransform");

        if (m_window->getNativeHandle() == nullptr) {
            assert(false); return false;
        }
        if (!m_swap_chain) {
            return true;
        }

        DXGI_SWAP_CHAIN_DESC1 info{};
        if (!win32::check_hresult_as_boolean(
            m_swap_chain->GetDesc1(&info),
            "IDXGISwapChain1::GetDesc1"sv
        )) {
            return false;
        }

        RECT rect{};
        if (!GetClientRect(static_cast<HWND>(m_window->getNativeHandle()), &rect)) {
            win32::check_hresult(HRESULT_FROM_WIN32(GetLastError()), "GetClientRect"sv);
            return false;
        }
        const auto window_size{ Vector2U(static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.bottom - rect.top)) };

        D2D1::Matrix3x2F matrix{ D2D1::Matrix3x2F::Identity() };
        if (m_scaling_mode == SwapChainScalingMode::stretch) {
            matrix = D2D1::Matrix3x2F::Scale(
                static_cast<float>(rect.right - rect.left) / static_cast<float>(info.Width),
                static_cast<float>(rect.bottom - rect.top) / static_cast<float>(info.Height)
            );
        }
        else {
            makeLetterboxing(window_size, Vector2U(info.Width, info.Height), matrix);
        }

        if (!win32::check_hresult_as_boolean(
            m_composition_visual_swap_chain->SetTransform(matrix),
            "IDCompositionVisual::SetTransform"sv
        )) {
            return false;
        }

    #ifdef LUASTG_ENABLE_DIRECT2D
        if (!m_swap_chain_title_bar.setSize(Vector2U(window_size.x, m_swap_chain_title_bar.getSize().y))) {
            return false;
        }
    #endif

        return commitComposition();
    }

    // custom title bar

    bool SwapChain::presentTitleBar() {
    #ifdef LUASTG_ENABLE_DIRECT2D
        auto& title_bar_controller = static_cast<Window*>(m_window.get())->getTitleBarController();

        if (m_model == SwapChainModel::legacy || m_model == SwapChainModel::modern) {
            // render directly to the swap chain
            title_bar_controller.draw(m_swap_chain_bitmap.get());
        }
        else if (m_model == SwapChainModel::composition) {
            // render to the secondary swap chain
            if (title_bar_controller.isVisible()) {
                if (!m_is_title_bar_attached) {
                    if (!win32::check_hresult_as_boolean(
                        m_composition_visual_root->AddVisual(m_composition_visual_title_bar.get(), TRUE, m_composition_visual_swap_chain.get()),
                        "IDCompositionVisual::AddVisual (m_composition_visual_title_bar, TRUE, m_composition_visual_swap_chain)"sv
                    )) {
                        return false;
                    }

                    if (!commitComposition()) {
                        return false;
                    }

                    m_is_title_bar_attached = true;
                }

                const auto swap_chain_size = m_swap_chain_title_bar.getSize();
                const auto title_bar_height = title_bar_controller.getHeight();
                if (title_bar_height != swap_chain_size.y) {
                    if (!m_swap_chain_title_bar.setSize(Vector2U(swap_chain_size.x, title_bar_height))) {
                        return false;
                    }
                }

                m_swap_chain_title_bar.clearRenderTarget();
                title_bar_controller.draw(m_swap_chain_title_bar.getBitmap1());
                if (!m_swap_chain_title_bar.present()) {
                    return false;
                }
            }
            else if (m_is_title_bar_attached) {
                if (!win32::check_hresult_as_boolean(
                    m_composition_visual_root->RemoveVisual(m_composition_visual_title_bar.get()),
                    "IDCompositionVisual::RemoveVisual (m_composition_visual_title_bar)"sv
                )) {
                    return false;
                }

                if (!commitComposition()) {
                    return false;
                }

                m_is_title_bar_attached = false;
            }
        }
        else {
            assert(false); return false;
        }

    #endif
        return true;
    }
}

namespace core {
    bool ISwapChain::create(IWindow* const window, IGraphicsDevice* const device, ISwapChain** const out_swap_chain) {
        if (window == nullptr) {
            assert(false); return false;
        }
        if (device == nullptr) {
            assert(false); return false;
        }
        if (out_swap_chain == nullptr) {
            assert(false); return false;
        }
        SmartReference<SwapChain> swap_chain;
        swap_chain.attach(new SwapChain());
        if (!swap_chain->initialize(window, device)) {
            return false;
        }
        *out_swap_chain = swap_chain.detach();
        return true;
    }
}
