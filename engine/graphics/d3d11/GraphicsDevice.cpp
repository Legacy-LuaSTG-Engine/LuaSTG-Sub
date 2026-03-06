#include "d3d11/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include "d3d11/GraphicsDeviceManager.hpp"
#include "d3d11/ToStringHelper.hpp"
#include "d3d11/DeviceHelper.hpp"
#include "windows/AdapterPolicy.hpp"
#include "windows/Direct3D11.hpp"
#include "utf8.hpp"

namespace {
    using std::string_view_literals::operator ""sv;
}

namespace core {
    // IGraphicsDevice

    void* GraphicsDevice::getNativeDevice() {
        return d3d11_device.get();
    }

    void GraphicsDevice::addEventListener(IGraphicsDeviceEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
        m_event_listeners.emplace_back(listener);
    }
    void GraphicsDevice::removeEventListener(IGraphicsDeviceEventListener* const listener) {
        assert(!m_is_dispatching_event);
        std::erase(m_event_listeners, listener);
    }

    // GraphicsDevice

    GraphicsDevice::GraphicsDevice() = default;
    GraphicsDevice::~GraphicsDevice() {
        destroy();
    }
}

namespace core {
    // IGraphicsDevice

    GraphicsDeviceMemoryStatistics GraphicsDevice::getMemoryStatistics() {
        GraphicsDeviceMemoryStatistics data{};
        win32::com_ptr<IDXGIAdapter3> adapter;
        if (win32::check_hresult_as_boolean(dxgi_adapter->QueryInterface(adapter.put()))) {
            DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
            if (win32::check_hresult_as_boolean(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))) {
                data.local.budget = info.Budget;
                data.local.current_usage = info.CurrentUsage;
                data.local.available_for_reservation = info.AvailableForReservation;
                data.local.current_reservation = info.CurrentReservation;
            }
            if (win32::check_hresult_as_boolean(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info))) {
                data.non_local.budget = info.Budget;
                data.non_local.current_usage = info.CurrentUsage;
                data.non_local.available_for_reservation = info.AvailableForReservation;
                data.non_local.current_reservation = info.CurrentReservation;
            }
        }
        return data;
    }

    bool GraphicsDevice::recreate() {
        return doDestroyAndCreate();
    }

    // GraphicsDevice

    bool GraphicsDevice::create() {
        Logger::info("[core] [GraphicsDevice] initializing...");

        testAdapterPolicy();
        if (!createDXGI())
            return false;
        if (!createD3D11())
            return false;
#ifdef LUASTG_ENABLE_DIRECT2D
        if (!createD2D1())
            return false;
#endif

        Logger::info("[core] [GraphicsDevice] initialization complete");
        return true;
    }
    void GraphicsDevice::destroy() {
        // 清理对象
#ifdef LUASTG_ENABLE_DIRECT2D
        destroyD2D1();
#endif
        destroyD3D11();
        destroyDXGI();
        assert(m_event_listeners.size() == 0);
    }
    bool GraphicsDevice::handleDeviceLost() {
        if (d3d11_device) {
            win32::check_hresult(d3d11_device->GetDeviceRemovedReason());
        }
        return doDestroyAndCreate();
    }

    // GraphicsDevice: private

    void GraphicsDevice::dispatchEvent(const Event e) {
        assert(!m_is_dispatching_event);
        m_is_dispatching_event = true;

        switch (e) {
        case Event::create:
            for (const auto listener : m_event_listeners) {
                listener->onGraphicsDeviceCreate();
            }
            break;
        case Event::destroy:
            for (const auto listener : m_event_listeners) {
                listener->onGraphicsDeviceDestroy();
            }
            break;
        }

        m_is_dispatching_event = false;
    }

    bool GraphicsDevice::testAdapterPolicy() {
        if (preferred_adapter_name.empty()) {
            return true; // default GPU
        }

        auto testStage = [](const std::string& name, const bool nv, const bool amd) -> bool {
            Platform::AdapterPolicy::setNVIDIA(nv);
            Platform::AdapterPolicy::setAMD(amd);
            if (!core::GraphicsDeviceManager::refresh()) {
                return false;
            }
            if (core::GraphicsDeviceManager::getPhysicalDeviceCount() == 0) {
                return false;
            }
            return core::GraphicsDeviceManager::getPhysicalDeviceName(0) == name;
        };

        // Stage 1 - Disable and test
        if (testStage(preferred_adapter_name, false, false)) return true;
        // Stage 2 - Enable and test
        if (testStage(preferred_adapter_name, true, true)) return true;
        // Stage 3 - NVIDIA and test
        if (testStage(preferred_adapter_name, true, false)) return true;
        // Stage 4 - AMD and test
        if (testStage(preferred_adapter_name, false, true)) return true;
        // Stage 5 - Disable and failed
        Platform::AdapterPolicy::setAll(false);

        return false;
    }
    bool GraphicsDevice::selectAdapter() {
        Logger::info("[core] [GraphicsDevice] enumerate physical devices"sv);

        win32::com_ptr<IDXGIAdapter1> adapter;
        std::string adapter_name;
        core::GraphicsDeviceManagerDXGI::refreshAndFindAdapter(preferred_adapter_name, adapter.put(), &adapter_name);
        core::GraphicsDeviceManager::log();

        if (adapter) {
            if (!win32::check_hresult_as_boolean(
                adapter->GetParent(IID_PPV_ARGS(dxgi_factory.put())),
                "IDXGIObject::GetParent (IDXGIFactory2)"sv
            )) {
                return false;
            }
            dxgi_adapter = adapter;
            dxgi_adapter_name = adapter_name;
        }

        dxgi_adapter_name_list.clear();
        for (uint32_t i = 0; i < core::GraphicsDeviceManager::getPhysicalDeviceCount(); i += 1) {
            dxgi_adapter_name_list.emplace_back(core::GraphicsDeviceManager::getPhysicalDeviceName(i));
        }

        if (!dxgi_adapter) {
            Logger::error("[core] [GraphicsDevice] no physical device available");
            return false;
        }

        //if (!link_to_output) {
        //	Logger::warn(
        //		"[core] [GraphicsDevice] {} doesn't appear to connect to any display output, which could result in:\n"
        //		"    Buffer is copied over PCI-E when in exclusive fullscreen\n"
        //		"    Desktop Window Manager takes over desktop composition when in exclusive fullscreen\n"
        //		"    Degraded performance and increased frame latency"
        //	);
        //}

        Logger::info("[core] [GraphicsDevice] current physical device: {}", dxgi_adapter_name);
        return true;
    }
    bool GraphicsDevice::createDXGI() {
        if (!selectAdapter()) {
            if (!core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
                return false;
            }
        }
        return true;
    }
    void GraphicsDevice::destroyDXGI() {
        dxgi_factory.reset();
        dxgi_adapter.reset();

        dxgi_adapter_name.clear();
        dxgi_adapter_name_list.clear();
    }
    bool GraphicsDevice::createD3D11() {
        HRESULT hr = S_OK;

        // 创建

        if (dxgi_adapter) {
            d3d11::createDevice(
                dxgi_adapter.get(),
                d3d11_device.put(), d3d11_devctx.put(), &d3d_feature_level
            );
        }
        else if (core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowSoftwareDevice()) {
            D3D_DRIVER_TYPE driver_type{};
            if (d3d11::createSoftwareDevice(
                d3d11_device.put(), d3d11_devctx.put(), &d3d_feature_level,
                &driver_type
            )) {
                Logger::info("[core] [GraphicsDevice] software device type: {}", toStringView(driver_type));
            }
        }

        if (!d3d11_device) {
            Logger::error("[core] [GraphicsDevice] no device available");
            return false;
        }

        if (!dxgi_adapter) {
            if (!win32::check_hresult_as_boolean(
                Platform::Direct3D11::GetDeviceAdater(d3d11_device.get(), dxgi_adapter.put()),
                "Platform::Direct3D11::GetDeviceAdater"sv
            )) {
                return false;
            }
        }

        d3d11_devctx->QueryInterface(d3d11_devctx1.put()); // error can be safely ignored

        // 特性检查

        d3d11::logDeviceFeatureSupportDetails(d3d11_device.get());
        d3d11::logDeviceFormatSupportDetails(d3d11_device.get());

        UINT bgra32_support{}, bgra32_support2{};
        d3d11::checkFormatSupport(d3d11_device.get(), DXGI_FORMAT_B8G8R8A8_UNORM, &bgra32_support, &bgra32_support2); // error can be safely ignored

        constexpr UINT required_support = D3D11_FORMAT_SUPPORT_TEXTURE2D
            | D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER
            | D3D11_FORMAT_SUPPORT_MIP
            | D3D11_FORMAT_SUPPORT_RENDER_TARGET
            | D3D11_FORMAT_SUPPORT_BLENDABLE
            | D3D11_FORMAT_SUPPORT_DISPLAY;
        if ((bgra32_support & required_support) != required_support) {
            Logger::warn("[core] [GraphicsDevice] device does not fully support the B8G8R8A8 format"sv);
        }

        tracy_context = tracy_d3d11_context_create(d3d11_device.get(), d3d11_devctx.get());

        return true;
    }
    void GraphicsDevice::destroyD3D11() {
        tracy_d3d11_context_destroy(tracy_context);
        tracy_context = nullptr;

        d3d_feature_level = D3D_FEATURE_LEVEL_10_0;

        d3d11_device.reset();
        d3d11_devctx.reset();
        d3d11_devctx1.reset();
    }
#ifdef LUASTG_ENABLE_DIRECT2D
    bool GraphicsDevice::createD2D1() {
        HRESULT hr = S_OK;

        D2D1_FACTORY_OPTIONS d2d1_options{};
    #if (!defined(NDEBUG) && defined(LUASTG_GRAPHICS_DEBUG_LAYER_ENABLE))
        d2d1_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    #endif
        if (!win32::check_hresult_as_boolean(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_MULTI_THREADED,
                __uuidof(ID2D1Factory1),
                &d2d1_options,
                d2d1_factory.put<void>()
            ),
            "D2D1CreateFactory (ID2D1Factory1)"sv
        )) {
            assert(false); return false;
        }

        win32::com_ptr<IDXGIDevice> dxgi_device;
        if (!win32::check_hresult_as_boolean(
            d3d11_device->QueryInterface(dxgi_device.put()),
            "ID3D11Device::QueryInterface (IDXGIDevice)"sv
        )) {
            assert(false); return false;
        }

        if (!win32::check_hresult_as_boolean(
            d2d1_factory->CreateDevice(dxgi_device.get(), d2d1_device.put()),
            "ID2D1Factory1::CreateDevice"sv
        )) {
            assert(false); return false;
        }

        if (!win32::check_hresult_as_boolean(
            d2d1_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d1_devctx.put()),
            "ID2D1Device::CreateDeviceContext"sv
        )) {
            assert(false); return false;
        }

        return true;
    }
    void GraphicsDevice::destroyD2D1() {
        d2d1_factory.reset();
        d2d1_device.reset();
        d2d1_devctx.reset();
    }
#endif
    bool GraphicsDevice::doDestroyAndCreate() {
        dispatchEvent(Event::destroy);

#ifdef LUASTG_ENABLE_DIRECT2D
        destroyD2D1();
#endif
        //destroyWIC(); // 长生存期
        destroyD3D11();
        destroyDXGI();

        testAdapterPolicy();

        if (!createDXGI()) return false;
        if (!createD3D11()) return false;
        //if (!createWIC()) return false; // 长生存期
#ifdef LUASTG_ENABLE_DIRECT2D
        if (!createD2D1()) return false;
#endif

        dispatchEvent(Event::create);
        return true;
    }
}

namespace core {
    bool IGraphicsDevice::create(const StringView preferred_gpu, IGraphicsDevice** const output) {
        if (output == nullptr) {
            assert(false); return false;
        }
        SmartReference<GraphicsDevice> device;
        device.attach(new GraphicsDevice());
        device->setPreferenceGpu(preferred_gpu);
        if (!device->create()) {
            return false;
        }
        *output = device.detach();
        return true;
    }
}
