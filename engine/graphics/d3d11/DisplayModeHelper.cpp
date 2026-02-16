#include "d3d11/DisplayModeHelper.hpp"
#include "core/Logger.hpp"

namespace {
    using std::string_view_literals::operator ""sv;
}

namespace d3d11 {
    bool getSwapChainNearestOutputFromWindow(IDXGISwapChain* const swap_chain, const HWND window, IDXGIOutput** const out_output) {
        if (swap_chain == nullptr) {
            assert(false); return false;
        }
        if (window == nullptr) {
            assert(false); return false;
        }
        if (out_output == nullptr) {
            assert(false); return false;
        }
        
        const auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        if (monitor == nullptr) {
            core::Logger::error("[core] MonitorFromWindow (MONITOR_DEFAULT_TO_NEAREST) failed"); // unlikely
            assert(false); return false;
        }

        MONITORINFOEXW monitor_info{};
        monitor_info.cbSize = sizeof(MONITORINFOEXW);
        if (!GetMonitorInfoW(monitor, &monitor_info)) {
            core::Logger::error("[core] GetMonitorInfoW failed"); // unlikely
            assert(false); return false;
        }

        win32::com_ptr<IDXGIFactory1> factory;
        if (!win32::check_hresult_as_boolean(
            swap_chain->GetParent(IID_PPV_ARGS(factory.put())),
            "IDXGISwapChain::GetParent (IDXGIFactory1)"sv
        )) {
            assert(false); return false; // unlikely
        }

        win32::com_ptr<IDXGIAdapter1> adapter;
        win32::com_ptr<IDXGIOutput> output;

        for (UINT adapter_index = 0; SUCCEEDED(factory->EnumAdapters1(adapter_index, adapter.put())); adapter_index += 1) {
            for (UINT output_index = 0; SUCCEEDED(adapter->EnumOutputs(output_index, output.put())); output_index += 1) {
                DXGI_OUTPUT_DESC output_info{};
                if (!win32::check_hresult_as_boolean(
                    output->GetDesc(&output_info),
                    "IDXGIOutput::GetDesc"sv
                )) {
                    continue;
                }

                MONITORINFOEXW info{};
                info.cbSize = sizeof(MONITORINFOEXW);
                if (!GetMonitorInfoW(output_info.Monitor, &info)) {
                    core::Logger::error("[core] GetMonitorInfoW failed"); // unlikely
                    assert(false); continue;
                }

                if (std::wstring_view(monitor_info.szDevice) == std::wstring_view(info.szDevice)) {
                    *out_output = output.detach();
                    return true;
                }
            }
        }

        return false;
    }
}
