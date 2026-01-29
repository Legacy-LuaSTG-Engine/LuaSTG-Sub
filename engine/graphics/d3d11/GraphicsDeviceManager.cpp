#include "d3d11/GraphicsDeviceManager.hpp"
#include "utf8.hpp"
#include "core/Logger.hpp"
#include "d3d11/ToStringHelper.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

    struct PhysicalDeviceInfo {
        std::string name;
        win32::com_ptr<IDXGIAdapter1> adapter;
        DXGI_ADAPTER_DESC3 info{};
        int info_level{};
        D3D_FEATURE_LEVEL feature_level{ D3D_FEATURE_LEVEL_9_1 };
    };

    std::vector<PhysicalDeviceInfo> physical_device_info;
    std::vector<PhysicalDeviceInfo> physical_device_info_not_supported;
    bool is_present_allow_tearing_supported{};

    decltype(&CreateDXGIFactory2) getCreateFactory2() {
        if (const auto dxgi = GetModuleHandleW(L"dxgi.dll"); dxgi != nullptr) {
            return reinterpret_cast<decltype(&CreateDXGIFactory2)>(GetProcAddress(dxgi, "CreateDXGIFactory2"));
        }
        return nullptr;
    }

    bool createFactory(win32::com_ptr<IDXGIFactory2>& factory) {
    #ifdef NDEBUG
        constexpr UINT flags{};
    #else
        constexpr UINT flags{ DXGI_CREATE_FACTORY_DEBUG };
    #endif
        if (const auto createFactory2 = getCreateFactory2(); createFactory2 != nullptr) {
            if (!win32::check_hresult_as_boolean(
                createFactory2(flags, IID_PPV_ARGS(factory.put())),
                "CreateDXGIFactory2"sv
            )) {
                return false;
            }
        }
        else {
            if (!win32::check_hresult_as_boolean(
                CreateDXGIFactory1(IID_PPV_ARGS(factory.put())),
                "CreateDXGIFactory1"sv
            )) {
                return false;
            }
        }
        return true;
    }

    bool isSoftwareOrRemoteAdapter(const DXGI_ADAPTER_DESC3& info) {
        // https://walbourn.github.io/anatomy-of-direct3d-11-create-device/
        return ((info.VendorId == 0x1414) && (info.DeviceId == 0x8c))
            || (info.Flags & DXGI_ADAPTER_FLAG3_REMOTE)
            || (info.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE);
    }

    D3D_FEATURE_LEVEL testAdapterFeatureLevel(win32::com_ptr<IDXGIAdapter1>& adapter) {
        constexpr UINT flags{ D3D11_CREATE_DEVICE_BGRA_SUPPORT };
        constexpr D3D_FEATURE_LEVEL feature_levels[] {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
        };
        D3D_FEATURE_LEVEL feature_level{};
        for (UINT offset = 0; offset <= 3; offset += 1) {
            const auto hr = D3D11CreateDevice(
                adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                flags, feature_levels + offset, 9u - offset, D3D11_SDK_VERSION,
                nullptr, &feature_level, nullptr
            );
            if (SUCCEEDED(hr)) {
                break;
            }
        }
        return feature_level;
    }

    bool internalRefresh(win32::com_ptr<IDXGIFactory2>& factory, const bool save_adapter) {
        if (!createFactory(factory)) {
            return false;
        }

        win32::com_ptr<IDXGIFactory5> factory5;
        std::ignore = factory->QueryInterface(factory5.put());
        if (factory5) {
            BOOL supported{};
            if (win32::check_hresult_as_boolean(
                factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported)),
                "IDXGIFactory5::CheckFeatureSupport (DXGI_FEATURE_PRESENT_ALLOW_TEARING)"sv
            )) {
                is_present_allow_tearing_supported = supported == TRUE;
            }
        }

        win32::com_ptr<IDXGIAdapter1> adapter;
        for (UINT i = 0; ; i += 1) {
            const auto hr = factory->EnumAdapters1(i, adapter.put());
            if (hr == DXGI_ERROR_NOT_FOUND) {
                break;
            }
            else if (FAILED(hr)) {
                win32::check_hresult(hr, "IDXGIFactory1::EnumAdapters1");
                break;
            }

            win32::com_ptr<IDXGIAdapter2> adapter2;
            std::ignore = adapter->QueryInterface(adapter2.put());
            win32::com_ptr<IDXGIAdapter4> adapter4;
            std::ignore = adapter->QueryInterface(adapter4.put());

            PhysicalDeviceInfo info;

            if (adapter4 && win32::check_hresult_as_boolean(
                adapter4->GetDesc3(&info.info),
                "IDXGIAdapter4::GetDesc3"sv
            )) {
                info.info_level = 4;
            }

            static_assert(sizeof(DXGI_ADAPTER_DESC2) == sizeof(DXGI_ADAPTER_DESC3), "bad reinterpret_cast");
            if (info.info_level == 0 && adapter2 && win32::check_hresult_as_boolean(
                adapter2->GetDesc2(reinterpret_cast<DXGI_ADAPTER_DESC2*>(&info.info)),
                "IDXGIAdapter2::GetDesc2"sv
            )) {
                info.info_level = 2;
            }

            static_assert(sizeof(DXGI_ADAPTER_DESC1) == sizeof(DXGI_ADAPTER_DESC3) - 8, "bad reinterpret_cast");
            if (info.info_level == 0 && win32::check_hresult_as_boolean(
                adapter->GetDesc1(reinterpret_cast<DXGI_ADAPTER_DESC1*>(&info.info)),
                "IDXGIAdapter1::GetDesc1"sv
            )) {
                info.info_level = 1;
            }

            info.feature_level = testAdapterFeatureLevel(adapter);

            if (info.info_level > 0) {
                info.name = utf8::to_string(info.info.Description);
                if (save_adapter) {
                    info.adapter = adapter;
                }
                if (info.feature_level >= D3D_FEATURE_LEVEL_10_0 && !isSoftwareOrRemoteAdapter(info.info)) {
                    physical_device_info.emplace_back(info);
                }
                else if (info.feature_level >= D3D_FEATURE_LEVEL_9_1) {
                    physical_device_info_not_supported.emplace_back(info);
                }
                // else: compute-only or software device
            }
        }

        return true;
    }

    void logCollection(const std::vector<PhysicalDeviceInfo>& collection, std::string& s) {
        for (const auto& v : collection) {
            s.append(std::format(
                "        {}:\n"sv
                "            Direct3D feature level: {}\n"sv
                "            VendorId: 0x{:08x}\n"sv
                "            DeviceId: 0x{:08x}\n"sv
                "            SubSysId: 0x{:08x}\n"sv
                "            Revision: {}\n"sv
                "            Dedicated video memory: {} ({})\n"sv
                "            Dedicated system memory: {} ({})\n"sv
                "            Shared system memory: {} ({})\n"sv
                "            Adapter LUID: {:08x}-{:08x}\n"sv
                "            Flags: {}\n"sv
                , v.name
                , toStringView(v.feature_level)
                , v.info.VendorId
                , v.info.DeviceId
                , v.info.SubSysId
                , v.info.Revision
                , v.info.DedicatedVideoMemory, toString(v.info.DedicatedVideoMemory, Semantic::data_size)
                , v.info.DedicatedSystemMemory, toString(v.info.DedicatedSystemMemory, Semantic::data_size)
                , v.info.SharedSystemMemory, toString(v.info.SharedSystemMemory, Semantic::data_size)
                , static_cast<DWORD>(v.info.AdapterLuid.HighPart), v.info.AdapterLuid.LowPart
                , toString(v.info.Flags)
            ));
            if (v.info_level < 4) {
                continue;
            }
            s.append(std::format(
                "            Graphics preemption granularity: {}\n"sv
                "            Compute preemption granularity: {}\n"sv
                , toStringView(v.info.GraphicsPreemptionGranularity)
                , toStringView(v.info.ComputePreemptionGranularity)
            ));
        }
    }
}

namespace core {
    bool GraphicsDeviceManager::refresh() {
        physical_device_info.clear();
        physical_device_info_not_supported.clear();
        is_present_allow_tearing_supported = false;

        win32::com_ptr<IDXGIFactory2> factory;
        return internalRefresh(factory, false);
    }
    bool GraphicsDeviceManager::isFeatureSupported(const GraphicsSystemFeature feature) {
        if (feature == GraphicsSystemFeature::windows_dxgi_present_allow_tearing) {
            return is_present_allow_tearing_supported;
        }
        return false;
    }
    uint32_t GraphicsDeviceManager::getPhysicalDeviceCount() {
        return static_cast<uint32_t>(physical_device_info.size());
    }
    StringView GraphicsDeviceManager::getPhysicalDeviceName(const uint32_t index) {
        if (index >= physical_device_info.size()) {
            return ""sv;
        }
        return physical_device_info.at(index).name;
    }
    void GraphicsDeviceManager::log() {
        std::string s;
        s.append(std::format(
            "[core] [GraphicsDeviceManager] Graphics System details:\n"sv
            "    Features:\n"sv
            "        Present allow tearing (Windows DXGI): {}\n"sv
            "    Physical devices:\n"sv
            , toStringView(is_present_allow_tearing_supported, Semantic::support)
        ));
        if (!physical_device_info.empty()) {
            logCollection(physical_device_info, s);
        }
        else {
            s.append("        N/A\n"sv);
        }
        if (!physical_device_info_not_supported.empty()) {
            s.append("    Physical devices (unmet requirements):\n"sv);
            logCollection(physical_device_info_not_supported, s);
        }
        if (s.back() == '\n') {
            s.pop_back();
        }
        Logger::info(s);
    }
}

namespace core {
    bool GraphicsDeviceManagerDXGI::refreshAndFindAdapter(const std::string_view name, IDXGIAdapter1** const adapter, std::string* const adapter_name) {
        physical_device_info.clear();
        physical_device_info_not_supported.clear();
        is_present_allow_tearing_supported = false;

        if (adapter == nullptr) {
            assert(false); return false;
        }

        win32::com_ptr<IDXGIFactory2> factory;
        if (!internalRefresh(factory, true)) {
            return false;
        }

        if (!name.empty()) {
            for (const auto& v : physical_device_info) {
                if (v.name == name) {
                    v.adapter->AddRef();
                    *adapter = v.adapter.get();
                    if (adapter_name != nullptr) {
                        *adapter_name = v.name;
                    }
                    return true;
                }
            }
        }

        if (!physical_device_info.empty()) {
            const auto& v = physical_device_info.front();
            v.adapter->AddRef();
            *adapter = v.adapter.get();
            if (adapter_name != nullptr) {
                *adapter_name = v.name;
            }
            return true;
        }

        return false;
    }
}
