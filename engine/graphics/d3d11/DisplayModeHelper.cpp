#include "d3d11/DisplayModeHelper.hpp"
#include "core/Logger.hpp"
#include "d3d11/ToStringHelper.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr DXGI_FORMAT COLOR_BUFFER_FORMAT{ DXGI_FORMAT_B8G8R8A8_UNORM };

    bool getDisplayModeList1(IDXGIOutput1* const output, std::vector<DXGI_MODE_DESC1>& mode_list) {
        if (output == nullptr) {
            assert(false); return false;
        }
        UINT mode_count{};
        if (!win32::check_hresult_as_boolean(
            output->GetDisplayModeList1(COLOR_BUFFER_FORMAT, 0, &mode_count, nullptr),
            "IDXGIOutput1::GetDisplayModeList1"sv
        )) {
            return false;
        }
        if (mode_count == 0) {
            mode_list.clear();
            return true;
        }
        mode_list.resize(mode_count);
        if (!win32::check_hresult_as_boolean(
            output->GetDisplayModeList1(COLOR_BUFFER_FORMAT, 0, &mode_count, mode_list.data()),
            "IDXGIOutput1::GetDisplayModeList1"sv
        )) {
            mode_list.clear();
            return false;
        }
        return true;
    }

    int calcIntegerMultipleOf60Hz(const DXGI_RATIONAL& rr) {
        const UINT v = rr.Numerator * 100 / rr.Denominator;
        for (UINT i = 1u; i <= 100u; i += 1u) {
            const UINT range_low = (60u * i - i) * 100u;
            const UINT range_high = (60u * i + i) * 100u;
            if (v >= range_low && v <= range_high) {
                return static_cast<int>(i);
            }
        }
        return 0;
    }
    int calcRefreshRateScore(const DXGI_MODE_DESC1& mode) {
        const auto& rr = mode.RefreshRate;
        if (rr.Denominator == 0) {
            return 0;
        }
        const int v = std::min(static_cast<int>(rr.Numerator) * 100 / static_cast<int>(rr.Denominator), 9999'99);
        if (v < 59'00) {
            // 刷新率过低，不适合 STG
            return v;
        }
        else if (v > 61'00 && v < 118'00) {
            // 尚可接受的刷新率，比如 75Hz, 90Hz, 100Hz
            return v + 1'000000;
        }
        else if (v >= 59'00 && v <= 61'00) {
            // 标准的 60Hz
            return v + 2'000000;
        }
        else if (const int m = calcIntegerMultipleOf60Hz(rr); m >= 2) {
            // 刷新率是 60Hz 的倍数
            return v + 10'000000 * m;
        }
        else /* if (v >= 118'00) */ {
            // 只要是 120Hz 或更高的刷新率，都可以满足 STG 游戏需求，无论是 144Hz 还是 165Hz 这种奇异刷新率
            return v + 4'000000;
        }
    }

    bool compareRefreshRate(const DXGI_MODE_DESC1& a, const DXGI_MODE_DESC1& b) {
        // 得分从高到低
        return calcRefreshRateScore(a) > calcRefreshRateScore(b);
    }
    bool isRefreshRateMeetMinimumRequirements(const DXGI_MODE_DESC1& mode) {
        const UINT v = mode.RefreshRate.Numerator * 100 / mode.RefreshRate.Denominator;
        return v >= 59'00;
    }
    bool isSizeMeetMinimumRequirements(const DXGI_MODE_DESC1& mode, const UINT target_width, const UINT target_height) {
        return mode.Width >= target_width && mode.Height >= target_height;
    }
    bool isScanLineOrderMeetMinimumRequirements(const DXGI_MODE_DESC1& mode) {
        // 默认或逐行扫描
        return mode.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || mode.ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    }
    bool isAspectRatioMatch(const DXGI_MODE_DESC1& mode, const UINT display_width, const UINT display_height) {
        if (display_width == 0 || display_height == 0) {
            assert(false); return false;
        }
        if (display_width > display_height) {
            const UINT width = mode.Height * display_width / display_height;
            const int delta = static_cast<int>(mode.Width) - static_cast<int>(width);
            return width != 0 && delta >= -2 && delta <= 2;
        }
        else {
            const UINT height = mode.Width * display_height / display_width;
            const int delta = static_cast<int>(mode.Height) - static_cast<int>(height);
            return height != 0 && delta >= -2 && delta <= 2;
        }
    }
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

    bool findBestDisplayMode(IDXGISwapChain1* const swap_chain, const HWND window, const UINT target_width, const UINT target_height, DXGI_MODE_DESC1& mode) {
        if (swap_chain == nullptr) {
            assert(false); return false;
        }
        if (window == nullptr) {
            assert(false); return false;
        }
        if (target_width == 0 || target_height == 0) {
            assert(false); return false;
        }

        win32::com_ptr<IDXGIOutput> output;
        if (!win32::check_hresult_as_boolean(
            swap_chain->GetContainingOutput(output.put()),
            "IDXGISwapChain1::GetContainingOutput"sv
        )) {
            if (!d3d11::getSwapChainNearestOutputFromWindow(swap_chain, window, output.put())) {
                return false;
            }
        }

        DXGI_OUTPUT_DESC output_info{};
        if (!win32::check_hresult_as_boolean(
            output->GetDesc(&output_info),
            "IDXGIOutput::GetDesc"sv
        )) {
            assert(false); return false; // unlikely
        }

        assert(output_info.AttachedToDesktop);
        assert(output_info.Monitor != nullptr);
        assert(output_info.DesktopCoordinates.right > output_info.DesktopCoordinates.left);
        assert(output_info.DesktopCoordinates.bottom > output_info.DesktopCoordinates.top);

        const auto display_width = static_cast<UINT>(output_info.DesktopCoordinates.right - output_info.DesktopCoordinates.left);
        const auto display_height = static_cast<UINT>(output_info.DesktopCoordinates.bottom - output_info.DesktopCoordinates.top);


        win32::com_ptr<IDXGIOutput1> output1;
        if (!win32::check_hresult_as_boolean(
            output->QueryInterface(output1.put()),
            "IDXGIOutput::QueryInterface (IDXGIOutput1)"sv
        )) {
            return false;
        }

        std::vector<DXGI_MODE_DESC1> mode_list;
        if (!getDisplayModeList1(output1.get(), mode_list)) {
            return false;
        }

        // 默认的桌面分辨率

        std::vector<DXGI_MODE_DESC1> default_mode_list;
        for (auto const& v : mode_list) {
            if (v.Width == display_width && v.Height == display_height) {
                default_mode_list.emplace_back(v);
            }
        }

        if (default_mode_list.empty()) {
            assert(false); return false;
        }

        std::sort(default_mode_list.begin(), default_mode_list.end(), &compareRefreshRate);
        const DXGI_MODE_DESC1 default_mode = default_mode_list.at(0);

        // 剔除隔行扫描

        for (auto it = mode_list.begin(); it != mode_list.end();) {
            if (!isScanLineOrderMeetMinimumRequirements(*it))
                it = mode_list.erase(it);
            else
                it++;
        }

        // 剔除刷新率过低的

        for (auto it = mode_list.begin(); it != mode_list.end();) {
            if (!isRefreshRateMeetMinimumRequirements(*it))
                it = mode_list.erase(it);
            else
                it++;
        }

        // 剔除分辨率比画布分辨率低的

        for (auto it = mode_list.begin(); it != mode_list.end();) {
            if (!isSizeMeetMinimumRequirements(*it, target_width, target_height))
                it = mode_list.erase(it);
            else
                it++;
        }

        // 剔除横纵比不和显示器匹配的

        for (auto it = mode_list.begin(); it != mode_list.end();) {
            if (!isAspectRatioMatch(*it, display_width, display_height))
                it = mode_list.erase(it);
            else
                it++;
        }

        // 根据面积比和刷新率进行排序
        // 刷新率从高分到低分
        // 面积比（渲染分辨率/显示模式分辨率）从高到低，面积比越高代表空间利用率越高，浪费的像素越少

        auto compareRefreshRateAndSizeRatio = [=](DXGI_MODE_DESC1 const& a, DXGI_MODE_DESC1 const& b) -> bool {
            const double target_size = static_cast<double>(target_width) * static_cast<double>(target_height);
            const double a_size = static_cast<double>(a.Width) * static_cast<double>(a.Height);
            const double b_size = static_cast<double>(b.Width) * static_cast<double>(b.Height);
            const double a_ratio = target_size / a_size;
            const double b_ratio = target_size / b_size;
            if (a_ratio == b_ratio) {
                return compareRefreshRate(a, b);
            }
            else {
                return a_ratio > b_ratio;
            }
        };

        std::sort(mode_list.begin(), mode_list.end(), compareRefreshRateAndSizeRatio);

        // 打印结果

        core::Logger::info("[core] [SwapChain] found {} DisplayMode", mode_list.size());
        for (size_t i = 0; i < mode_list.size(); i += 1) {
            core::Logger::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz {} {}"
                , i
                , mode_list[i].Width, mode_list[i].Height
                , (double)mode_list[i].RefreshRate.Numerator / (double)mode_list[i].RefreshRate.Denominator
                , toStringView(mode_list[i].Scaling)
                , toStringView(mode_list[i].ScanlineOrdering)
            );
        }

        // 最终，挑选出面积比最大且刷新率最高的

        if (!mode_list.empty()) {
            mode = mode_list.at(0);
        }
        else {
            mode = default_mode;
        }

        return true;
    }
}
