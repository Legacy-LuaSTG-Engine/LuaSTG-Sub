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

//#define _log(x) core::Logger::info(x)
#define _log(x)

#define HRNew HRESULT hr = S_OK;
#define HRGet hr = gHR
#define HRCheckCallReport(x) if (FAILED(hr)) { core::Logger::error("Windows API failed: " x); }
#define HRCheckCallReturnBool(x) if (FAILED(hr)) { core::Logger::error("Windows API failed: " x); assert(false); return false; }
#define HRCheckCallNoAssertReturnBool(x) if (FAILED(hr)) { core::Logger::error("Windows API failed: " x); return false; }

#define NTNew NTSTATUS nt{};
#define NTGet nt
#define NTCheckCallReport(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); }
#define NTCheckCallReportRuntime(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: {}", x); }
#define NTCheckCallReturnBool(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); assert(false); return false; }
#define NTCheckCallNoAssertReturnBool(x) if (nt != STATUS_SUCCESS) { core::Logger::error("Windows API failed: " x); return false; }

#define ReportError(x) core::Logger::error("Windows API failed: " x)

namespace {
    using std::string_view_literals::operator ""sv;
    using std::string_literals::operator ""s;

    constexpr DXGI_FORMAT const COLOR_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;
    constexpr DXGI_FORMAT const DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

    bool makeLetterboxing(core::Vector2U rect, core::Vector2U inner_rect, DXGI_MATRIX_3X2_F& mat)
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
            mat = DXGI_MATRIX_3X2_F{
                FLOAT(scale), 0.0f,
                0.0f, FLOAT(scale),
                FLOAT(x), FLOAT(y),
            };
            return true;
        }
    }
    bool makeLetterboxing(core::Vector2U rect, core::Vector2U inner_rect, D2D1_MATRIX_3X2_F& mat)
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
    DXGI_SWAP_CHAIN_DESC1 getDefaultSwapChainInfo7()
    {
        return DXGI_SWAP_CHAIN_DESC1{
            .Width = 0,
            .Height = 0,
            .Format = COLOR_BUFFER_FORMAT,
            .Stereo = FALSE,
            .SampleDesc = DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
            .Flags = 0,
        };
    }
    DXGI_SWAP_CHAIN_DESC1 getDefaultSwapChainInfo10()
    {
        return DXGI_SWAP_CHAIN_DESC1{
            .Width = 0,
            .Height = 0,
            .Format = COLOR_BUFFER_FORMAT,
            .Stereo = FALSE,
            .SampleDesc = DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 3,
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
            .Flags = 0,
        };
    }

    bool checkModernSwapChainModelAvailable(ID3D11Device* device)
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
    bool checkMultiPlaneOverlaySupport(ID3D11Device* device)
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
    bool isModernSwapChainModel(DXGI_SWAP_CHAIN_DESC1 const& info)
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

        if (m_model == SwapChainModel::composition) {
            if (!updateCompositionTransform()) {
                return false;
            }
        }

        dispatchEvent(Event::create);
        return true;
    }
    Vector2U SwapChain::getCanvasSize() {
        return m_canvas_size;
    }

    void SwapChain::setScalingMode(const SwapChainScalingMode mode) {
        m_scaling_mode = mode;
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
        // scaling

        if (!isRenderTargetAndCanvasSizeEquals() && (m_model == SwapChainModel::legacy || m_model == SwapChainModel::modern)) {
            if (!m_scaling_renderer.UpdateTransform(
                m_canvas_srv.get(),
                m_swap_chain_rtv.get(),
                m_scaling_mode == SwapChainScalingMode::stretch
            )) {
                return false;
            }

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
            resizeSwapChain(size);
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
        if (!setWindowMode(m_canvas_size)) {
            return;
        }
        if (!m_scaling_renderer.AttachDevice(device)) {
            return;
        }
        // TODO: enter exclusive fullscreen
    }
    void SwapChain::onGraphicsDeviceDestroy() {
        destroySwapChain();
        destroyCanvas();
        m_scaling_renderer.DetachDevice();
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
        m_allow_composition = false; config.isAllowDirectComposition();

        const auto size{ Vector2U(config.getWidth(), config.getHeight()) };
        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }

        if (!setWindowMode(size)) {
            return false;
        }
        if (!m_scaling_renderer.AttachDevice(d3d11_device)) {
            return false;
        }
    #ifdef LUASTG_ENABLE_DIRECT2D
        // static_cast<Window*>(m_window.get())->getTitleBarController().createResources(win, m_device->GetD2D1DeviceContext());
        // static_cast<Window*>(m_window.get())->getTitleBarController().destroyResources();
    #endif

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

    bool SwapChain::createSwapChain(const bool create_rtv) {
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

        if (create_rtv) {
            if (!createRenderTarget()) {
                return false;
            }
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
        destroyRenderTarget();
        destroyComposition();
        if (m_swap_chain) {
            setFullscreenState(m_swap_chain.get(), FALSE);
        }
        m_swap_chain.reset();
        m_frame_latency_event.reset();
    }
    bool SwapChain::resizeSwapChain(const Vector2U size) {
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

        return createRenderTarget();
    }
    bool SwapChain::createRenderTarget() {
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
        return setFullscreenState(m_swap_chain.get(), TRUE);
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
        return result;
    }
    bool SwapChain::enterExclusiveFullscreen() {
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

        dispatchEvent(Event::destroy);
        destroySwapChain();

        m_window->setSize({ display_mode.Width, display_mode.Height });
        m_swap_chain_fullscreen_display_mode = display_mode;
        m_exclusive_fullscreen = true;

        if (!createSwapChain(false)) {
            return false;
        }

        core::ApplicationManager::setDelegateUpdateEnabled(false);
        if (!setFullscreenState(m_swap_chain.get(), TRUE)) {
            return false;
        }

        if (!resizeSwapChain({ display_mode.Width, display_mode.Height })) {
            return false;
        }

        dispatchEvent(Event::create);

        return true;
    }
    bool SwapChain::leaveExclusiveFullscreen() {
        if (!m_swap_chain) {
            assert(false); return false;
        }
        setFullscreenState(m_swap_chain.get(), FALSE);
        core::ApplicationManager::setDelegateUpdateEnabled(true);
        if (m_exclusive_fullscreen) {
            m_exclusive_fullscreen = false;
            dispatchEvent(Event::destroy);
            destroySwapChain();
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

namespace core {
    void SwapChain_D3D11::dispatchEvent(const EventType e) {
        assert(!m_is_dispatching_event);
        m_is_dispatching_event = true;

        switch (e) {
        case EventType::SwapChainCreate:
            for (const auto listener : m_event_listeners) {
                listener->onSwapChainCreate();
            }
            break;
        case EventType::SwapChainDestroy:
            for (const auto listener : m_event_listeners) {
                listener->onSwapChainDestroy();
            }
            break;
        }

        m_is_dispatching_event = false;
    }
    void SwapChain_D3D11::addEventListener(ISwapChainEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
        m_event_listeners.emplace_back(listener);
    }
    void SwapChain_D3D11::removeEventListener(ISwapChainEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
    }

    void SwapChain_D3D11::onGraphicsDeviceCreate()
    {
        m_modern_swap_chain_available = checkModernSwapChainModelAvailable(m_device->GetD3D11Device());
        m_scaling_renderer.AttachDevice(m_device->GetD3D11Device());
        if (m_init) // 曾经设置过
        {
            setWindowMode(m_canvas_size);
            if (m_swap_chain_fullscreen_mode)
            {
                // 重新进入独占全屏
                onWindowFullscreenStateChange(true);
            }
        }
    }
    void SwapChain_D3D11::onGraphicsDeviceDestroy()
    {
        destroySwapChain();
        m_scaling_renderer.DetachDevice();
        m_modern_swap_chain_available = false;
    }
    void SwapChain_D3D11::onWindowCreate()
    {
        // 目前窗口的重新创建只会被交换链触发，所以这里留空
    }
    void SwapChain_D3D11::onWindowDestroy()
    {
        // 目前窗口的重新创建只会被交换链触发，所以这里留空
    }
    void SwapChain_D3D11::onWindowActive()
    {
        if (!dxgi_swapchain)
            return;
        _log("onWindowActive");
        m_swapchain_want_present_reset = TRUE;
        enterExclusiveFullscreenTemporarily();
    }
    void SwapChain_D3D11::onWindowInactive()
    {
        if (!dxgi_swapchain)
            return;
        _log("onWindowInactive");
        leaveExclusiveFullscreenTemporarily();
    }
    void SwapChain_D3D11::onWindowSize(core::Vector2U size)
    {
        _log("onWindowSize");
        if (size.x == 0 || size.y == 0)
            return; // 忽略窗口最小化
        if (!dxgi_swapchain)
            return; // 此时交换链还未初始化
        if (m_is_composition_mode)
        {
            handleDirectCompositionWindowSize(size);
        }
        else
        {
            handleSwapChainWindowSize(size);
        }
    }
    void SwapChain_D3D11::onWindowFullscreenStateChange(bool state)
    {
        _log("onWindowFullscreenStateChange");
        if (!dxgi_swapchain)
            return;
        if (state)
            enterExclusiveFullscreen();
        else
            leaveExclusiveFullscreen();
    }

    bool SwapChain_D3D11::createSwapChain(bool fullscreen, DXGI_MODE_DESC1 const& mode, bool no_attachment)
    {
        _log("createSwapChain");

        Logger::info("[core] [SwapChain] creating...");

        // 必须成功的操作

        const auto win = static_cast<HWND>(m_window->getNativeHandle());
        if (!win) {
            Logger::error("[core] [SwapChain] create failed: Window not initialized");
            assert(false); return false;
        }
        if (!m_device->getNativeHandle()) {
            Logger::error("[core] [SwapChain] create failed: GraphicsDevice not initialized");
            assert(false); return false;
        }
        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        if (!core::GraphicsDeviceManagerDXGI::refreshAndGetFactory(dxgi_factory.put())) {
            Logger::error("[core] [SwapChain] create failed: DXGI not available");
            assert(false); return false;
        }

        HRNew;

        // 填写交换链描述

        m_swap_chain_info = getDefaultSwapChainInfo7();
        m_swap_chain_fullscreen_info = {};
        
        // 切换为 FLIP 交换链模型，独占全屏也支持
        if (m_modern_swap_chain_available) {
            m_swap_chain_info.BufferCount = 3;
            m_swap_chain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }

        if (!fullscreen)
        {
            // 获取窗口尺寸
            RECT rc = {};
            if (!GetClientRect(win, &rc))
            {
                HRGet = HRESULT_FROM_WIN32(GetLastError());
                HRCheckCallReturnBool("GetClientRect");
            }
            if (rc.right <= rc.left || rc.bottom <= rc.top)
            {
                Logger::error(
                    "[core] [SwapChain] size cannot be ({}x{})",
                    rc.right - rc.left,
                    rc.bottom - rc.top
                );
                assert(false); return false;
            }
            // 使用窗口尺寸
            m_swap_chain_info.Width = static_cast<UINT>(rc.right - rc.left);
            m_swap_chain_info.Height = static_cast<UINT>(rc.bottom - rc.top);
            // 进一步配置 FLIP 交换链模型
            if (m_modern_swap_chain_available)
            {
                m_swap_chain_info.BufferCount = 3; // 三个缓冲区能带来更平稳的性能
                m_swap_chain_info.Scaling = DXGI_SCALING_NONE; // 禁用 DWM 对交换链的缩放
                m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // 低延迟呈现技术
                m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // 立即刷新和可变刷新率
            }
        }
        else
        {
            // 使用显示模式尺寸
            m_swap_chain_info.Width = mode.Width;
            m_swap_chain_info.Height = mode.Height;
            // 允许切换显示模式
            m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            // 配置全屏模式
            m_swap_chain_fullscreen_info.RefreshRate = mode.RefreshRate;
            m_swap_chain_fullscreen_info.ScanlineOrdering = mode.ScanlineOrdering;
            m_swap_chain_fullscreen_info.Scaling = mode.Scaling;
            m_swap_chain_fullscreen_info.Windowed = TRUE; // 稍后再切换到独占全屏
        }

        // 创建交换链

        HRGet = dxgi_factory->CreateSwapChainForHwnd(
            m_device->GetD3D11Device(),
            win,
            &m_swap_chain_info,
            fullscreen ? &m_swap_chain_fullscreen_info : NULL,
            NULL,
            dxgi_swapchain.put());
        HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForHwnd");
        
        m_swap_chain_fullscreen_mode = fullscreen;

        // 禁用默认的 ALT+ENTER 全屏切换
        // 注意这里有坑，新创建的 DXGI 工厂和交换链内部使用的不是同一个实例

        HRGet = Platform::DXGI::MakeSwapChainWindowAssociation(
            dxgi_swapchain.get(), DXGI_MWA_NO_ALT_ENTER);
        HRCheckCallReturnBool("IDXGIFactory1::MakeWindowAssociation -> DXGI_MWA_NO_ALT_ENTER");
        
        // 设置设备最大帧延迟为 1

        HRGet = Platform::DXGI::SetDeviceMaximumFrameLatency(
            dxgi_swapchain.get(), 1);
        HRCheckCallReturnBool("IDXGIDevice1::SetMaximumFrameLatency -> 1");
        
        // 如果启用了低延迟呈现技术，则设置交换链最大帧延迟为 1
        
        if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            HANDLE event_handle{};
            HRGet = Platform::DXGI::SetSwapChainMaximumFrameLatency(
                dxgi_swapchain.get(), 1, &event_handle);
            HRCheckCallReturnBool("IDXGISwapChain2::SetMaximumFrameLatency -> 1");
            dxgi_swapchain_event.reset(event_handle);
        }

        auto refresh_rate_string = std::format("{:.2f}Hz", (double)mode.RefreshRate.Numerator / (double)mode.RefreshRate.Denominator);
        if (!fullscreen) refresh_rate_string = "Desktop RefreshRate";
        std::string_view swapchain_model = "Discard";
        if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) swapchain_model = "FlipSequential";
        if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) swapchain_model = "FlipDiscard";
        auto enable_or_disable = [](bool v) -> std::string_view { return v ? "Enable" : "Disable"; };
        Logger::info(
            "[core] [SwapChain] created:\n"
            "    Display Mode: {}x{}@{}\n"
            "    Exclusive Fullscreen：{}\n"
            "    SwapChain swap effect: {}\n"
            "    Present Allow Tearing: {}\n"
            "    Frame Latency Waitable Object: {}"
            , mode.Width, mode.Height, refresh_rate_string
            , enable_or_disable(fullscreen)
            , swapchain_model
            , enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
            , enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        );

        // 渲染附件

        if (!no_attachment)
        {
            if (!createRenderAttachment()) return false;
        }

        // 标记

        m_swapchain_want_present_reset = TRUE;

#ifdef LUASTG_ENABLE_DIRECT2D
        static_cast<Window*>(m_window.get())->getTitleBarController().createResources(win, m_device->GetD2D1DeviceContext());
#endif

        return true;
    }
    void SwapChain_D3D11::destroySwapChain()
    {
        _log("destroySwapChain");

        static_cast<Window*>(m_window.get())->getTitleBarController().destroyResources();
        destroyDirectCompositionResources();
        destroyRenderAttachment();
        if (dxgi_swapchain)
        {
            // 退出独占全屏
            HRNew;
            BOOL bFullscreen = FALSE;
            win32::com_ptr<IDXGIOutput> dxgi_output;
            HRGet = dxgi_swapchain->GetFullscreenState(&bFullscreen, dxgi_output.put());
            HRCheckCallReport("IDXGISwapChain::GetFullscreenState");
            if (SUCCEEDED(hr) && bFullscreen)
            {
                Logger::info("[core] [SwapChain] leave exclusive fullscreen");
                HRGet = setFullscreenState(dxgi_swapchain.get(), FALSE);
                HRCheckCallReport("IDXGISwapChain::SetFullscreenState -> FALSE");
            }
        }
        dxgi_swapchain_event.reset();
        dxgi_swapchain.reset();
    }
    void SwapChain_D3D11::waitFrameLatency(uint32_t timeout, bool reset)
    {
        //_log("waitFrameLatency");

        if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            if (reset && dxgi_swapchain)
            {
                HRNew;

                // 微软不知道写了什么狗屎bug，有时候dwm临时接管桌面合成后会导致上屏延迟多一倍
                // 重新设置最大帧延迟并创建延迟等待对象似乎能解决该问题
                dxgi_swapchain_event.reset();

                win32::com_ptr<IDXGISwapChain2> dxgi_swap_chain2;
                HRGet = dxgi_swapchain->QueryInterface(dxgi_swap_chain2.put());
                HRCheckCallReport("IDXGISwapChain1 -> IDXGISwapChain２");
                if (FAILED(hr)) return;
                HRGet = dxgi_swap_chain2->SetMaximumFrameLatency(1);
                HRCheckCallReport("IDXGISwapChain２::SetMaximumFrameLatency(1)");
                if (FAILED(hr)) return;

                dxgi_swapchain_event.reset(dxgi_swap_chain2->GetFrameLatencyWaitableObject());
            }
            if (dxgi_swapchain_event)
            {
                DWORD const result = WaitForSingleObject(dxgi_swapchain_event.get(), timeout);
                if (!(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT))
                {
                    win32::check_hresult(HRESULT_FROM_WIN32(GetLastError()));
                    Logger::error("Windows API failed: WaitForSingleObject");
                }
            }
        }
    }
    bool SwapChain_D3D11::enterExclusiveFullscreenTemporarily()
    {
        if (!m_swap_chain_fullscreen_mode)
        {
            return true;
        }
        if (!dxgi_swapchain)
        {
            assert(false); return false;
        }

        HRNew;

        BOOL get_state = FALSE;
        HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
        HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

        if (get_state)
        {
            return true;
        }

        _log("IDXGISwapChain::SetFullscreenState -> TRUE\n");
        Logger::info("[core] [SwapChain] enter exclusive fullscreen");
        HRGet = setFullscreenState(dxgi_swapchain.get(), TRUE);
        HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> TRUE");

        return true;
    }
    bool SwapChain_D3D11::leaveExclusiveFullscreenTemporarily()
    {
        if (!m_swap_chain_fullscreen_mode)
        {
            return true;
        }
        if (!dxgi_swapchain)
        {
            assert(false); return false;
        }

        HRNew;

        BOOL get_state = FALSE;
        HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
        HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

        if (!get_state)
        {
            return true;
        }

        _log("IDXGISwapChain::SetFullscreenState -> FALSE\n");
        Logger::info("[core] [SwapChain] leave exclusive fullscreen");
        HRGet = setFullscreenState(dxgi_swapchain.get(), FALSE);
        HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> FALSE");

        m_window->setLayer(WindowLayer::Normal); // 强制取消窗口置顶

        return true;
    }
    bool SwapChain_D3D11::enterExclusiveFullscreen() {
        _log("enterExclusiveFullscreen");
        if (m_disable_exclusive_fullscreen) {
            return false;
        }
        if (!dxgi_swapchain) {
            assert(false);
            return false;
        }
        if (dcomp_visual_swap_chain) {
            return false;
        }

        DXGI_MODE_DESC1 display_mode{};
        if (!d3d11::findBestDisplayMode(dxgi_swapchain.get(), static_cast<HWND>(m_window->getNativeHandle()), m_canvas_size.x, m_canvas_size.y, display_mode)) {
            return false;
        }

        dispatchEvent(EventType::SwapChainDestroy);
        destroySwapChain();

        m_window->setSize({ display_mode.Width, display_mode.Height });

        if (!createSwapChain(true, display_mode, true /* 稍后创建渲染附件 */)) {
            return false;
        }

        HRNew;

        // 进入全屏
        Logger::info("[core] [SwapChain] enter exclusive fullscreen");
        HRGet = setFullscreenState(dxgi_swapchain.get(), TRUE);
        HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> TRUE");

        // 需要重设交换链大小（特别是 Flip 交换链模型）
        HRGet = dxgi_swapchain->ResizeBuffers(0, display_mode.Width, display_mode.Height, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
        HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

        // 创建渲染附件
        if (!createRenderAttachment()) {
            return false;
        }
        if (!updateLetterBoxingRendererTransform()) {
            return false;
        }

        // 记录状态
        m_init = TRUE;

        // 广播
        dispatchEvent(EventType::SwapChainCreate);
        
        return true;
    }
    bool SwapChain_D3D11::leaveExclusiveFullscreen()
    {
        _log("leaveExclusiveFullscreen");
        HRNew;

        BOOL get_state = FALSE;
        HRGet = dxgi_swapchain->GetFullscreenState(&get_state, NULL);
        HRCheckCallReturnBool("IDXGISwapChain::GetFullscreenState");

        if (get_state)
        {
            Logger::info("[core] [SwapChain] leave exclusive fullscreen");
            HRGet = setFullscreenState(dxgi_swapchain.get(), FALSE);
            HRCheckCallReturnBool("IDXGISwapChain::SetFullscreenState -> FALSE");
        }
        
        if (m_swap_chain_fullscreen_mode)
        {
            m_swap_chain_fullscreen_mode = FALSE; // 手动离开全屏模式

            dispatchEvent(EventType::SwapChainDestroy);
            destroySwapChain();

            if (!createSwapChain(false, {}, false))
            {
                return false;
            }

            if (!updateLetterBoxingRendererTransform()) return false;

            // 记录状态
            m_init = TRUE;

            // 广播
            dispatchEvent(EventType::SwapChainCreate);

            return true;
        }
        
        return true;
    }

    bool SwapChain_D3D11::createDirectCompositionResources()
    {
        assert(m_modern_swap_chain_available);
        _log("createDirectCompositionResources");

        // 我们限制 DirectComposition 仅在 Windows 10+ 使用

        HRNew;

        // 必须成功的操作

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        if (!core::GraphicsDeviceManagerDXGI::refreshAndGetFactory(dxgi_factory.put())) {
            Logger::error("[core] [SwapChain] create failed: DXGI not available");
            assert(false); return false;
        }

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<IDXGIDevice> dxgi_device;
        HRGet = m_device->GetD3D11Device()->QueryInterface(dxgi_device.put());
        HRCheckCallReturnBool("ID3D11Device::QueryInterface -> IDXGIDevice");
        
        // 创建基本组件

        HRGet = dcomp_loader.CreateDevice(dxgi_device.get(), IID_PPV_ARGS(dcomp_desktop_device.put()));
        HRCheckCallReturnBool("DCompositionCreateDevice");
#else
        HRGet = dcomp_loader.CreateDevice(nullptr, IID_PPV_ARGS(dcomp_desktop_device.put()));
        HRCheckCallReturnBool("DCompositionCreateDevice");
#endif
        
    #ifdef _DEBUG
        win32::com_ptr<IDCompositionDeviceDebug> dcomp_device_debug;
        HRGet = dcomp_desktop_device->QueryInterface(dcomp_device_debug.put());
        HRCheckCallReport("IDCompositionDesktopDevice::QueryInterface -> IDCompositionDeviceDebug");
        if (SUCCEEDED(hr))
        {
            HRGet = dcomp_device_debug->EnableDebugCounters();
            HRCheckCallReport("IDCompositionDeviceDebug::EnableDebugCounters");
        }
    #endif

        HRGet = dcomp_desktop_device->CreateTargetForHwnd(static_cast<HWND>(m_window->getNativeHandle()), TRUE, dcomp_target.put());
        HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateTargetForHwnd");
        
        
        HRGet = dcomp_desktop_device->CreateVisual(dcomp_visual_swap_chain.put());
        HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");
        
        // 桌面合成引擎模式，创建背景平面+交换链平面

        if (m_is_composition_mode)
        {
            HRGet = dcomp_desktop_device->CreateVisual(dcomp_visual_root.put());
            HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

#ifdef LUASTG_ENABLE_DIRECT2D
            HRGet = dcomp_desktop_device->CreateVisual(dcomp_visual_title_bar.put());
            HRCheckCallReturnBool("IDCompositionDesktopDevice::CreateVisual");

            if (!swap_chain_title_bar.create(
                dxgi_factory.get(),
                m_device->GetD3D11Device(),
                m_device->GetD2D1DeviceContext(),
                m_window->_getCurrentSize()
            )) {
                return false;
            }
            // 标题栏交换链需要的时候再使用

            HRGet = dcomp_visual_title_bar->SetContent(swap_chain_title_bar.getSwapChain1());
            HRCheckCallReturnBool("IDCompositionVisual2::SetContent");
#endif
        }
        
        // 把交换链塞进可视物

        HRGet = dcomp_visual_swap_chain->SetContent(dxgi_swapchain.get());
        HRCheckCallReturnBool("IDCompositionVisual2::SetContent");
        
        HRGet = dcomp_visual_swap_chain->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR); // TODO: 支持改为临近缩放
        HRCheckCallReturnBool("IDCompositionVisual2::SetBitmapInterpolationMode -> DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR");

        // 构建视觉树

        if (m_is_composition_mode)
        {
            HRGet = dcomp_visual_root->AddVisual(dcomp_visual_swap_chain.get(), TRUE, nullptr);
            HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");

            HRGet = dcomp_target->SetRoot(dcomp_visual_root.get());
            HRCheckCallReturnBool("IDCompositionTarget::SetRoot");

            // 设置变换并提交

            if (!updateDirectCompositionTransform()) return false;
        }
        else
        {
            HRGet = dcomp_target->SetRoot(dcomp_visual_swap_chain.get());
            HRCheckCallReturnBool("IDCompositionTarget::SetRoot");

            // 直接提交
            if (!commitDirectComposition()) return false;
        }
        
        return true;
    }
    void SwapChain_D3D11::destroyDirectCompositionResources()
    {
        _log("destroyDirectCompositionResources");

        // 我也不知道为什么要这样清理，但是不这么做的话会出现内存不足，后续创建设备和资源的操作会失败

        HRNew;

        if (dcomp_target)
        {
            HRGet = dcomp_target->SetRoot(NULL);
            HRCheckCallReport("IDCompositionTarget::SetRoot -> NULL");
        }
        if (dcomp_visual_root)
        {
            HRGet = dcomp_visual_root->RemoveAllVisuals();
            HRCheckCallReport("IDCompositionVisual2::RemoveAllVisuals");
        }
        if (dcomp_visual_swap_chain)
        {
            HRGet = dcomp_visual_swap_chain->SetContent(NULL);
            HRCheckCallReport("IDCompositionVisual2::SetContent -> NULL");
        }
#ifdef LUASTG_ENABLE_DIRECT2D
        if (dcomp_visual_title_bar)
        {
            HRGet = dcomp_visual_title_bar->SetContent(NULL);
            HRCheckCallReport("IDCompositionVisual2::SetContent -> NULL");
        }
#endif

        dcomp_target.reset();
        dcomp_visual_root.reset();
        dcomp_visual_swap_chain.reset();
#ifdef LUASTG_ENABLE_DIRECT2D
        dcomp_visual_title_bar.reset();
        swap_chain_title_bar.destroy();
        m_title_bar_attached = false;
#endif

        if (dcomp_desktop_device)
        {
            HRGet = dcomp_desktop_device->Commit();
            HRCheckCallReport("IDCompositionDesktopDevice::Commit");
            HRGet = dcomp_desktop_device->WaitForCommitCompletion();
            HRCheckCallReport("IDCompositionDesktopDevice::WaitForCommitCompletion");
        }
        
        dcomp_desktop_device.reset();
    }
    bool SwapChain_D3D11::updateDirectCompositionTransform()
    {
        assert(m_is_composition_mode);
        _log("updateDirectCompositionTransform");

        HRNew;

        // 必须成功的操作

        DXGI_SWAP_CHAIN_DESC1 desc1 = {};
        HRGet = dxgi_swapchain->GetDesc1(&desc1);
        HRCheckCallReturnBool("IDXGISwapChain1::GetDesc1");
        
        RECT rc = {};
        if (!GetClientRect(static_cast<HWND>(m_window->getNativeHandle()), &rc))
        {
            HRGet = HRESULT_FROM_WIN32(GetLastError());
            HRCheckCallReturnBool("GetClientRect");
        }
        auto const window_size_u = Vector2U(
            (uint32_t)(rc.right - rc.left),
            (uint32_t)(rc.bottom - rc.top));

        // 让背景铺满整个画面（由 Window Class 的背景来处理）


        // 设置交换链内容内接放大

        if (m_scaling_mode == SwapChainScalingMode::stretch)
        {
            auto const mat_d2d = D2D1::Matrix3x2F::Scale(
                (float)window_size_u.x / (float)desc1.Width,
                (float)window_size_u.y / (float)desc1.Height);
            HRGet = dcomp_visual_swap_chain->SetTransform(mat_d2d);
            HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
        }
        else
        {
            DXGI_MATRIX_3X2_F mat{};
            if (makeLetterboxing(window_size_u, Vector2U(desc1.Width, desc1.Height), mat))
            {
                D2D_MATRIX_3X2_F const mat_d2d = {
                    mat._11, mat._12,
                    mat._21, mat._22,
                    mat._31, mat._32,
                };
                HRGet = dcomp_visual_swap_chain->SetTransform(mat_d2d);
                HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
            }
            else
            {
                auto const mat_d2d = D2D1::Matrix3x2F::Identity();
                HRGet = dcomp_visual_swap_chain->SetTransform(mat_d2d);
                HRCheckCallReturnBool("IDCompositionVisual2::SetTransform");
            }
        }

        // 同步窗口宽度

#ifdef LUASTG_ENABLE_DIRECT2D
        if (!swap_chain_title_bar.setSize(Vector2U(window_size_u.x, swap_chain_title_bar.getSize().y))) {
            return false;
        }
#endif
        
        // 提交

        if (!commitDirectComposition()) return false;

        return true;
    }
    bool SwapChain_D3D11::commitDirectComposition()
    {
        _log("commitDirectComposition");

        HRNew;

        HRGet = dcomp_desktop_device->Commit();
        HRCheckCallReturnBool("IDCompositionDesktopDevice::Commit");
        
        HRGet = dcomp_desktop_device->WaitForCommitCompletion();
        HRCheckCallReturnBool("IDCompositionDesktopDevice::WaitForCommitCompletion");
        
        return true;
    }
    bool SwapChain_D3D11::createCompositionSwapChain(Vector2U size, bool latency_event)
    {
        _log("createCompositionSwapChain");

        // 我们限制 DirectComposition 仅在 Windows 10+ 使用

        HRNew;

        Logger::info("[core] [SwapChain] creating... (DirectComposition)");

        // 检查组件

        if (!m_window->getNativeHandle()) {
            Logger::error("[core] [SwapChain] create failed: Window not initialized");
            assert(false); return false;
        }
        if (!m_device->getNativeHandle()) {
            Logger::error("[core] [SwapChain] create failed: GraphicsDevice not initialized");
            assert(false); return false;
        }
        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        if (!core::GraphicsDeviceManagerDXGI::refreshAndGetFactory(dxgi_factory.put())) {
            Logger::error("[core] [SwapChain] create failed: DXGI not available");
            assert(false); return false;
        }

        // 填充交换链描述

        m_swap_chain_info = getDefaultSwapChainInfo10();
        m_swap_chain_info.Width = size.x;
        m_swap_chain_info.Height = size.y;
        m_swap_chain_info.Scaling = DXGI_SCALING_STRETCH; // DirectComposition 要求这个缩放模式

        m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // 必须支持
        if (latency_event)
        {
            m_swap_chain_info.BufferCount = 3; // 三重缓冲
            m_swap_chain_info.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // 低延迟渲染
        }

        // 创建交换链

        HRGet = dxgi_factory->CreateSwapChainForComposition(
            m_device->GetD3D11Device(),
            &m_swap_chain_info, NULL,
            dxgi_swapchain.put());
        HRCheckCallReturnBool("IDXGIFactory2::CreateSwapChainForComposition");
        
        m_swap_chain_fullscreen_mode = FALSE;

        // 设置最大帧延迟为 1

        HRGet = Platform::DXGI::SetDeviceMaximumFrameLatency(dxgi_swapchain.get(), 1);
        HRCheckCallReturnBool("IDXGIDevice1::SetMaximumFrameLatency -> 1");
        
        if (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            HANDLE event_handle{};
            HRGet = Platform::DXGI::SetSwapChainMaximumFrameLatency(
                dxgi_swapchain.get(), 1, &event_handle);
            HRCheckCallReturnBool("IDXGISwapChain2::SetMaximumFrameLatency -> 1");
            dxgi_swapchain_event.reset(event_handle);
        }

        // 打印信息

        auto refresh_rate_string = "Desktop RefreshRate";
        std::string_view swapchain_model = "FlipSequential";
        if (m_swap_chain_info.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) swapchain_model = "FlipDiscard";
        auto enable_or_disable = [](bool v) -> std::string_view { return v ? "Enable" : "Disable"; };
        Logger::info(
            "[core] [SwapChain] created:\n"
            "    Display Mode: {}x{}@{}\n"
            "    Exclusive Fullscreen：{}\n"
            "    SwapChain swap effect: {}\n"
            "    Present Allow Tearing: {}\n"
            "    Frame Latency Waitable Object: {}"
            , size.x, size.y, refresh_rate_string
            , "Disable" // 没有独占全屏
            , swapchain_model
            , enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
            , enable_or_disable(m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        );

        // 渲染附件

        if (!createRenderAttachment()) return false;

        // 创建合成器

        if (!createDirectCompositionResources()) return false;

        // 标记

        m_swapchain_want_present_reset = TRUE;

#ifdef LUASTG_ENABLE_DIRECT2D
        static_cast<Window*>(m_window.get())->getTitleBarController().createResources(static_cast<HWND>(m_window->getNativeHandle()), m_device->GetD2D1DeviceContext());
#endif

        return true;
    }

    bool SwapChain_D3D11::createSwapChainRenderTarget()
    {
        _log("createSwapChainRenderTarget");

        if (!dxgi_swapchain)
        {
            assert(false); return false;
        }

        if (!m_device->GetD3D11Device())
        {
            assert(false); return false;
        }

        HRNew;

        win32::com_ptr<ID3D11Texture2D> d3d11_texture2d;
        HRGet = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(d3d11_texture2d.put()));
        HRCheckCallReturnBool("IDXGISwapChain::GetBuffer -> 0");
        
        // TODO: 线性颜色空间
        HRGet = m_device->GetD3D11Device()->CreateRenderTargetView(d3d11_texture2d.get(), NULL, m_swap_chain_d3d11_rtv.put());
        HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<IDXGISurface> dxgi_surface;
        HRGet = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(dxgi_surface.put()));
        HRCheckCallReturnBool("IDXGISwapChain::GetBuffer -> 0");

        // TODO: 线性颜色空间
        D2D1_BITMAP_PROPERTIES1 d2d1_bitmap_info{};
        d2d1_bitmap_info.pixelFormat.format = COLOR_BUFFER_FORMAT;
        d2d1_bitmap_info.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        d2d1_bitmap_info.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        HRGet = m_device->GetD2D1DeviceContext()->CreateBitmapFromDxgiSurface(dxgi_surface.get(), &d2d1_bitmap_info, m_swap_chain_d2d1_bitmap.put());
        HRCheckCallReturnBool("ID2D1DeviceContext::CreateBitmapFromDxgiSurface");
#endif

        return true;
    }
    void SwapChain_D3D11::destroySwapChainRenderTarget()
    {
        _log("destroySwapChainRenderTarget");

        if (m_device->GetD3D11DeviceContext())
        {
            m_device->GetD3D11DeviceContext()->ClearState();
            m_device->GetD3D11DeviceContext()->Flush();
        }
        m_swap_chain_d3d11_rtv.reset();
#ifdef LUASTG_ENABLE_DIRECT2D
        m_swap_chain_d2d1_bitmap.reset();
#endif
    }
    bool SwapChain_D3D11::createCanvasColorBuffer()
    {
        _log("createCanvasColorBuffer");

        if (m_canvas_size.x == 0 || m_canvas_size.y == 0)
        {
            assert(false); return false;
        }

        if (!m_device->GetD3D11Device())
        {
            assert(false); return false;
        }

        HRNew;

        // Color Buffer

        D3D11_TEXTURE2D_DESC cb_info = {};
        cb_info.Width = m_canvas_size.x;
        cb_info.Height = m_canvas_size.y;
        cb_info.MipLevels = 1;
        cb_info.ArraySize = 1;
        cb_info.Format = COLOR_BUFFER_FORMAT;
        cb_info.SampleDesc.Count = 1;
        cb_info.Usage = D3D11_USAGE_DEFAULT;
        cb_info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        win32::com_ptr<ID3D11Texture2D> cb_texture;
        HRGet = m_device->GetD3D11Device()->CreateTexture2D(&cb_info, NULL, cb_texture.put());
        HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

        // Shader Resource

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_info = {};
        srv_info.Format = cb_info.Format; // TODO: 线性颜色空间
        srv_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_info.Texture2D.MipLevels = cb_info.MipLevels;

        HRGet = m_device->GetD3D11Device()->CreateShaderResourceView(cb_texture.get(), &srv_info, m_canvas_d3d11_srv.put());
        HRCheckCallReturnBool("ID3D11Device::CreateShaderResourceView");

        // Render Target

        D3D11_RENDER_TARGET_VIEW_DESC rtv_info = {};
        rtv_info.Format = cb_info.Format; // TODO: 线性颜色空间
        rtv_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        HRGet = m_device->GetD3D11Device()->CreateRenderTargetView(cb_texture.get(), &rtv_info, m_canvas_d3d11_rtv.put());
        HRCheckCallReturnBool("ID3D11Device::CreateRenderTargetView");

        return true;
    }
    void SwapChain_D3D11::destroyCanvasColorBuffer()
    {
        _log("destroyCanvasColorBuffer");

        m_canvas_d3d11_srv.reset();
        m_canvas_d3d11_rtv.reset();
    }
    bool SwapChain_D3D11::createCanvasDepthStencilBuffer()
    {
        _log("createCanvasDepthStencilBuffer");

        if (m_canvas_size.x == 0 || m_canvas_size.y == 0)
        {
            assert(false); return false;
        }

        if (!m_device->GetD3D11Device())
        {
            assert(false); return false;
        }

        HRNew;

        // Depth Stencil Buffer

        D3D11_TEXTURE2D_DESC ds_info = {};
        ds_info.Width = m_canvas_size.x;
        ds_info.Height = m_canvas_size.y;
        ds_info.MipLevels = 1;
        ds_info.ArraySize = 1;
        ds_info.Format = DEPTH_BUFFER_FORMAT;
        ds_info.SampleDesc.Count = 1;
        ds_info.Usage = D3D11_USAGE_DEFAULT;
        ds_info.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        win32::com_ptr<ID3D11Texture2D> ds_texture;
        HRGet = m_device->GetD3D11Device()->CreateTexture2D(&ds_info, NULL, ds_texture.put());
        HRCheckCallReturnBool("ID3D11Device::CreateTexture2D");

        // Depth Stencil View

        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_info = {};
        dsv_info.Format = ds_info.Format;
        dsv_info.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        HRGet = m_device->GetD3D11Device()->CreateDepthStencilView(ds_texture.get(), &dsv_info, m_canvas_d3d11_dsv.put());
        HRCheckCallReturnBool("ID3D11Device::CreateDepthStencilView");

        return true;
    }
    void SwapChain_D3D11::destroyCanvasDepthStencilBuffer()
    {
        _log("destroyCanvasDepthStencilBuffer");

        m_canvas_d3d11_dsv.reset();
    }
    bool SwapChain_D3D11::createRenderAttachment()
    {
        _log("createRenderAttachment");

        if (!createSwapChainRenderTarget()) return false;
        if (m_is_composition_mode)
        {
            // 此时画布颜色缓冲区就是交换链的后台缓冲区
            m_canvas_d3d11_srv.reset(); // 不使用
            m_canvas_d3d11_rtv = m_swap_chain_d3d11_rtv;
        }
        else
        {
            if (!createCanvasColorBuffer()) return false;
        }
        if (!createCanvasDepthStencilBuffer()) return false;
        return true;
    }
    void SwapChain_D3D11::destroyRenderAttachment()
    {
        _log("destroyRenderAttachment");

        destroyCanvasDepthStencilBuffer();
        destroyCanvasColorBuffer();
        destroySwapChainRenderTarget();
    }
    void SwapChain_D3D11::applyRenderAttachment()
    {
        //_log("applyRenderAttachment");

        if (auto* ctx = m_device->GetD3D11DeviceContext())
        {
            ID3D11RenderTargetView* rtvs[1] = { m_canvas_d3d11_rtv.get() };
            ctx->OMSetRenderTargets(1, rtvs, m_canvas_d3d11_dsv.get());
        }
    }
    void SwapChain_D3D11::clearRenderAttachment()
    {
        //_log("clearRenderAttachment");

        if (auto* ctx = m_device->GetD3D11DeviceContext())
        {
            if (m_canvas_d3d11_rtv)
            {
                FLOAT const clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                ctx->ClearRenderTargetView(m_canvas_d3d11_rtv.get(), clear_color);
            }
            if (m_canvas_d3d11_dsv)
            {
                ctx->ClearDepthStencilView(m_canvas_d3d11_dsv.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
            }
            ctx->Flush(); // 让命令立即提交到 GPU
        }
    }

    bool SwapChain_D3D11::updateLetterBoxingRendererTransform()
    {
        _log("updateLetterBoxingRendererTransform");
        assert(m_canvas_d3d11_srv);
        assert(m_swap_chain_d3d11_rtv);

        return m_scaling_renderer.UpdateTransform(
            m_canvas_d3d11_srv.get(),
            m_swap_chain_d3d11_rtv.get(),
            m_scaling_mode == SwapChainScalingMode::stretch
        );
    }
    bool SwapChain_D3D11::presentLetterBoxingRenderer()
    {
        _log("presentLetterBoxingRenderer");
        assert(m_canvas_d3d11_srv);
        assert(m_swap_chain_d3d11_rtv);

        return m_scaling_renderer.Draw(
            m_canvas_d3d11_srv.get(),
            m_swap_chain_d3d11_rtv.get(),
            true
        );
    }

    bool SwapChain_D3D11::handleDirectCompositionWindowSize(Vector2U size)
    {
        _log("handleDirectCompositionWindowSize");

        if (size.x == 0 || size.y == 0)
        {
            assert(false); return false;
        }

        if (!dxgi_swapchain)
        {
            assert(false); return false;
        }
        
        // 此时交换链和画布一致，不应该修改交换链本身，而是修改合成变换

        if (!updateDirectCompositionTransform()) return false;

        return true;
    }
    bool SwapChain_D3D11::handleSwapChainWindowSize(Vector2U size)
    {
        _log("handleSwapChainWindowSize");

        if (size.x == 0 || size.y == 0)
        {
            assert(false); return false;
        }

        if (!dxgi_swapchain)
        {
            assert(false); return false;
        }

        // 此时交换链和画布分离，应该重新调整交换链尺寸

        HRNew;

        destroySwapChainRenderTarget();

        HRGet = dxgi_swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
        HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

        if (!createSwapChainRenderTarget()) return false;

        if (!updateLetterBoxingRendererTransform()) return false;

        // TODO: 对于现代交换链模型，由于使用了 DirectComposition，还需要重新设置一次 Content

        return true;
    }

    bool SwapChain_D3D11::setWindowMode(Vector2U size)
    {
        _log("setWindowMode");

        if (size.x < 1 || size.y < 1)
        {
            Logger::error(
                "[core] [SwapChain] size cannot be ({}x{})",
                size.x, size.y
            );
            assert(false); return false;
        }

        if (m_enable_composition || (!m_disable_composition && m_modern_swap_chain_available && checkMultiPlaneOverlaySupport(m_device->GetD3D11Device())))
        {
            m_is_composition_mode = true;
            return setCompositionWindowMode(size);
        }
        else
        {
            m_is_composition_mode = false;
        }

        dispatchEvent(EventType::SwapChainDestroy);
        destroySwapChain();

        if (!m_disable_modern_swap_chain && m_modern_swap_chain_available)
        {
            // TODO: 这样就没法独占全屏了，因为拿不到包含的Output
            m_canvas_size = size;
            if (!createCompositionSwapChain(size, /* latency event */ true)) // 让它创建渲染附件
            {
                return false;
            }
        }
        else
        {
            m_canvas_size = size;
            if (!createSwapChain(false, {}, false)) // 让它创建渲染附件
            {
                return false;
            }
        }
        
        // 更新数据

        m_init = TRUE;
        if (!updateLetterBoxingRendererTransform()) return false;

        // 通知各个组件交换链已重新创建

        dispatchEvent(EventType::SwapChainCreate);

        return true;
    }
    bool SwapChain_D3D11::setCompositionWindowMode(Vector2U size)
    {
        _log("setCompositionWindowMode");

        // 销毁旧交换链

        dispatchEvent(EventType::SwapChainDestroy);
        destroySwapChain();

        // 创建交换链

        bool latency_event = true;
        m_canvas_size = size;
        if (!createCompositionSwapChain(size, latency_event))
        {
            return false;
        }

        // 更新数据

        m_init = TRUE;

        // 通知各个组件交换链已重新创建

        dispatchEvent(EventType::SwapChainCreate);

        return true;
    }

    bool SwapChain_D3D11::setCanvasSize(Vector2U size)
    {
        _log("setCanvasSize");

        if (size.x == 0 || size.y == 0)
        {
            Logger::error(
                "[core] [SwapChain] size cannot be ({}x{})",
                size.x, size.y
            );
            assert(false); return false;
        }

        m_canvas_size = size;

        // TODO: 如果尺寸没变，是不是可以直接返回？

        if (!dxgi_swapchain)
        {
            return true; // 当交换链还未初始化时，仅保存画布尺寸
        }

        dispatchEvent(EventType::SwapChainDestroy);

        if (m_is_composition_mode)
        {
            // 对于合成交换链，由于交换链和画布是一致的，所以要调整交换链尺寸

            destroyRenderAttachment();

            HRNew;
            HRGet = dxgi_swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, m_swap_chain_info.Flags);
            HRCheckCallReturnBool("IDXGISwapChain::ResizeBuffers");

            if (!createRenderAttachment()) return false;

            if (!updateDirectCompositionTransform()) return false;
        }
        else
        {
            // 对于普通交换链，由于画布是独立的，只需重新创建画布

            destroyCanvasDepthStencilBuffer();
            destroyCanvasColorBuffer();

            if (!createCanvasColorBuffer()) return false;
            if (!createCanvasDepthStencilBuffer()) return false;

            if (!updateLetterBoxingRendererTransform()) return false;
        }

        dispatchEvent(EventType::SwapChainCreate);

        if (!m_is_composition_mode && m_swap_chain_fullscreen_mode)
        {
            // TODO: LuaSTG 那边会先调用 setCanvasSize 再调用 setWindowMode 触发两次交换链创建
            // 重新选择合适的独占全屏模式
            leaveExclusiveFullscreen();
            enterExclusiveFullscreen();
        }

        return true;
    }

    void SwapChain_D3D11::setScalingMode(SwapChainScalingMode mode)
    {
        m_scaling_mode = mode;
        if (m_is_composition_mode)
            updateDirectCompositionTransform();
        else
            updateLetterBoxingRendererTransform();
    }

    void SwapChain_D3D11::waitFrameLatency()
    {
        if (m_swapchain_want_present_reset)
        {
            m_swapchain_want_present_reset = FALSE;
            waitFrameLatency(100, true);
        }
        else
        {
            waitFrameLatency(100, false);
        }
    }
    void SwapChain_D3D11::setVSync(bool enable)
    {
        m_swap_chain_vsync = enable;
    }
    bool SwapChain_D3D11::present()
    {
        _log("present");

        HRESULT hr = S_OK;

        // 手动合成画面的情况下，通过内接缩放渲染器来缩放显示

        if (!m_is_composition_mode) {
            tracy_d3d11_context_zone(m_device->GetTracyContext(), "PreScaling");
            if (!presentLetterBoxingRenderer()) {
                return false;
            }
            m_device->GetD3D11DeviceContext()->Flush(); // 立即提交命令到 GPU
        }

        // 绘制标题栏

#ifdef LUASTG_ENABLE_DIRECT2D
        auto& title_bar_controller = static_cast<Window*>(m_window.get())->getTitleBarController();
        if (m_is_composition_mode) {
            tracy_d3d11_context_zone(m_device->GetTracyContext(), "TitleBarComposition");
            // 绘制标题栏到单独的表面
            if (title_bar_controller.isVisible()) {
                if (!m_title_bar_attached) {
                    HRGet = dcomp_visual_root->AddVisual(dcomp_visual_title_bar.get(), TRUE, dcomp_visual_swap_chain.get());
                    HRCheckCallReturnBool("IDCompositionVisual2::AddVisual");
                    if (!commitDirectComposition()) return false;
                    m_title_bar_attached = true;
                }
                auto const swap_chain_size = swap_chain_title_bar.getSize();
                auto const title_bar_height = title_bar_controller.getHeight();
                if (title_bar_height != swap_chain_size.y) {
                    if (!swap_chain_title_bar.setSize(Vector2U(swap_chain_size.x, title_bar_height))) {
                        return false;
                    }
                }
                swap_chain_title_bar.clearRenderTarget();
                title_bar_controller.draw(swap_chain_title_bar.getBitmap1());
                if (!swap_chain_title_bar.present()) {
                    return false;
                }
            }
            else if (m_title_bar_attached) {
                HRGet = dcomp_visual_root->RemoveVisual(dcomp_visual_title_bar.get());
                HRCheckCallReturnBool("IDCompositionVisual2::RemoveVisual");
                if (!commitDirectComposition()) return false;
                m_title_bar_attached = false;
            }
        }
        else {
            tracy_d3d11_context_zone(m_device->GetTracyContext(), "DrawTitleBar");
            // 绘制标题栏到交换链上，而不是画布上
            title_bar_controller.draw(m_swap_chain_d2d1_bitmap.get());
        }
#endif
        
        // 呈现

        UINT interval = 0;
        UINT flags = 0;
        if (isModernSwapChainModel(m_swap_chain_info)) {
            // 现代交换链模型
            if (!m_swap_chain_vsync && (m_swap_chain_info.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)) {
                // 如果禁用了垂直同步，且支持 DXGI_FEATURE_PRESENT_ALLOW_TEARING 功能，使用该模式获得最低延迟
                // 如果系统、硬件（特别是显示器）、驱动支持可变刷新率，那么将不会造成画面撕裂
                flags |= DXGI_PRESENT_ALLOW_TEARING;
            }
            else {
                // 异步三重缓冲模式
                flags |= DXGI_PRESENT_DO_NOT_WAIT;
            }
        }
        else {
            // 传统交换链模型，通过指定同步间隔开/关垂直同步
            // 注意：从 Windows 8 开始才支持 DXGI_PRESENT_DO_NOT_WAIT
            interval = m_swap_chain_vsync ? 1 : 0;
        }
        hr = gHR = dxgi_swapchain->Present(interval, flags);

        // 清空渲染状态并丢弃内容

        m_device->GetD3D11DeviceContext()->ClearState();
        if (const auto ctx1 = m_device->GetD3D11DeviceContext1(); ctx1 != nullptr) {
            ctx1->DiscardView(m_swap_chain_d3d11_rtv.get());
        }

        // 检查结果

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            // 设备丢失
            return m_device->handleDeviceLost();
        }
        else if (hr != DXGI_ERROR_WAS_STILL_DRAWING && FAILED(hr)) {
            Logger::error("Windows API failed: IDXGISwapChain::Present");
            return false;
        }

        return true;
    }

    bool SwapChain_D3D11::saveSnapshotToFile(StringView path)
    {
        std::wstring wpath(utf8::to_wstring(path));

        HRESULT hr = S_OK;

        win32::com_ptr<ID3D11Resource> d3d11_resource;
        m_canvas_d3d11_rtv->GetResource(d3d11_resource.put());

        hr = gHR = DirectX::SaveWICTextureToFile(
            m_device->GetD3D11DeviceContext(),
            d3d11_resource.get(),
            GUID_ContainerFormatJpeg,
            wpath.c_str(),
            &GUID_WICPixelFormat24bppBGR);
        if (FAILED(hr))
        {
            Logger::error("Windows API failed: DirectX::SaveWICTextureToFile");
            return false;
        }

        return true;
    }

    SwapChain_D3D11::SwapChain_D3D11(IWindow* p_window, GraphicsDevice* p_device)
        : m_window(p_window)
        , m_device(p_device)
    {
        assert(p_window);
        assert(p_device);
        auto const& gs = core::ConfigurationLoader::getInstance().getGraphicsSystem();
        m_modern_swap_chain_available = checkModernSwapChainModelAvailable(m_device->GetD3D11Device());
        m_disable_exclusive_fullscreen = !gs.isAllowExclusiveFullscreen();
        m_disable_modern_swap_chain = !gs.isAllowModernSwapChain();
        m_enable_composition = false; // TODO
        m_disable_composition = !gs.isAllowDirectComposition();
        m_scaling_renderer.AttachDevice(m_device->GetD3D11Device());
        m_window->addEventListener(this);
        m_device->addEventListener(this);
    }
    SwapChain_D3D11::~SwapChain_D3D11() {
        m_window->removeEventListener(this);
        m_device->removeEventListener(this);
        destroySwapChain();
        m_scaling_renderer.DetachDevice();
        m_modern_swap_chain_available = false;
        assert(m_event_listeners.empty());
    }
}
