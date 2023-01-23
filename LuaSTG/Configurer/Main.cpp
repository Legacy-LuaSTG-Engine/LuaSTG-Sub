#include "Platform/HighDPI.hpp"
#include "Platform/WindowTheme.hpp"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

enum class Language : size_t
{
    Chinese = 0,
    English = 1,
};
static Language i18n_map_index = Language::Chinese;
static std::unordered_map<std::string_view, std::string_view> i18n_map[2] = {
    {
        {"window-title", "启动配置"},
        {"language-chinese", "简体中文"},
        {"language-english", "English"},
        {"setting-language", "语言（Language）"},
        {"setting-graphic-card", "显卡"},
        {"setting-display-mode", "显示模式"},
        {"setting-canvas-size", "分辨率"},
        {"setting-fullscreen", "全屏"},
        {"setting-vsync", "垂直同步"},
        {"setting-cancel", "取消并退出"},
        {"setting-save", "保存并退出"},
    },
    {
        {"window-title", "Configuer"},
        {"language-chinese", "简体中文"},
        {"language-english", "English"},
        {"setting-language", "Language (语言)"},
        {"setting-graphic-card", "Graphic Card"},
        {"setting-display-mode", "Display Mode"},
        {"setting-canvas-size", "Resolution"},
        {"setting-fullscreen", "Fullscreen"},
        {"setting-vsync", "VSync"},
        {"setting-cancel", "Cancel & Exit"},
        {"setting-save", "Save & Exit"},
    },
};
std::string_view const& i18n(std::string_view const& key)
{
    auto it = i18n_map[size_t(i18n_map_index)].find(key);
    if (it != i18n_map[size_t(i18n_map_index)].end())
    {
        return it->second;
    }
    return key;
}
inline char const* i18n_c_str(std::string_view const& key)
{
    return i18n(key).data();
}

struct DisplayMode
{
    std::string name;
    DXGI_MODE_DESC mode = {};

    DisplayMode() = default;
    DisplayMode(UINT width, UINT height)
    {
        std::array<char, 256> buffer{};
        std::snprintf(buffer.data(), buffer.size(), "%ux%u", width, height);
        name = buffer.data();
        mode.Width = width;
        mode.Height = height;
        mode.RefreshRate.Numerator = 0;
        mode.RefreshRate.Denominator = 0;
        mode.Format = DXGI_FORMAT_UNKNOWN;
        mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    }
    DisplayMode(std::string_view name, UINT width, UINT height)
    {
        this->name = name;
        mode.Width = width;
        mode.Height = height;
        mode.RefreshRate.Numerator = 0;
        mode.RefreshRate.Denominator = 0;
        mode.Format = DXGI_FORMAT_UNKNOWN;
        mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    }
    DisplayMode(std::string_view name_, DXGI_MODE_DESC const& mode_) : name(name_), mode(mode_) {}
};
struct Config
{
    std::string adapter;
    int width = 640;
    int height = 480;
    int refresh_rate_numerator = 0;
    int refresh_rate_denominator = 0;
    bool windowed = true;
    bool vsync = false;

    int select_adapter = 0;
    int select_mode = 0;
};

constexpr UINT WINDOW_SIZE_X = 400;
constexpr UINT WINDOW_SIZE_Y = 200;

struct Window
{
    WNDCLASSEXW win32_wincls;
    ATOM win32_wincls_atom;
    HWND win32_window;
    DWORD win32_window_style;
    DWORD win32_window_style_ex;
    UINT win32_window_width;
    UINT win32_window_height;
    UINT win32_window_dpi;
    static LRESULT CALLBACK win32_window_proc(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
    {
        if (Window* self = (Window*)GetWindowLongPtrW(window, GWLP_USERDATA))
        {
            return self->WndProc(window, message, arg1, arg2);
        }
        switch (message)
        {
        case WM_CREATE:
            SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)arg2)->lpCreateParams);
            return 0;
        default:
            return DefWindowProcW(window, message, arg1, arg2);
        }
    }

    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
    D3D_FEATURE_LEVEL d3d11_feature_level = D3D_FEATURE_LEVEL_10_0;
    Microsoft::WRL::ComPtr<IDXGISwapChain> dxgi_swapchain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rtv;

    std::vector<std::string> dxgi_adapter_list;
    std::vector<DisplayMode> dxgi_output_mode_list;
    
    Config luastg_config;

    BOOL is_open = FALSE;
    BOOL want_exit = FALSE;

    LRESULT WndProc(HWND window, UINT message, WPARAM arg1, LPARAM arg2)
    {
        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        if (ImGui_ImplWin32_WndProcHandler(window, message, arg1, arg2))
        {
            return TRUE;
        }
        switch (message)
        {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            OnSize(LOWORD(arg2), HIWORD(arg2));
            return 0;
        case WM_SYSCOMMAND:
            if ((arg1 & 0xFFF0) == SC_KEYMENU) return 0; // Disable ALT application menu
            return DefWindowProcW(window, message, arg1, arg2);
        case WM_PAINT:
            OnUpdate();
            return 0;
        case WM_DPICHANGED:
            OnScaling(LOWORD(arg1));
            return 0;
        case WM_THEMECHANGED:
        case WM_SETTINGCHANGE:
            Platform::WindowTheme::UpdateColorMode(window, TRUE);
            ApplyStyle();
            return DefWindowProcW(window, message, arg1, arg2);
        default:
            return DefWindowProcW(window, message, arg1, arg2);
        }
    }

    int Run()
    {
        MSG msg = {};
        while (!want_exit)
        {
            BOOL result = GetMessageW(&msg, NULL, 0, 0);
            if (result == -1)
            {
                throw std::runtime_error("GetMessageW failed."); // 这不该发生
            }
            else if (result == 0)
            {
                want_exit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        return 0;
    }
    void OnSize(UINT width, UINT height)
    {
        win32_window_width = width;
        win32_window_height = height;
        if (dxgi_swapchain && width > 0 && height > 0)
        {
            DestroyRenderTarget();
            HRESULT hr = dxgi_swapchain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (FAILED(hr))
            {
                throw std::runtime_error("IDXGISwapChain::ResizeBuffers failed.");
            }
            CreateRenderTarget();
        }
    }
    void OnUpdate()
    {
        if (!is_open) return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)win32_window_width, (float)win32_window_height), ImGuiCond_Always);
        if (ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
        {
            int select_lang = (int)i18n_map_index;
            char const* langs[2] = {
                i18n_c_str("language-chinese"),
                i18n_c_str("language-english"),
            };
            if (ImGui::Combo(i18n_c_str("setting-language"), &select_lang, langs, 2))
            {
                i18n_map_index = (Language)select_lang;
                updateTitle();
            }

            ImGui::Separator();

            if (ImGui::BeginCombo(i18n_c_str("setting-graphic-card"), dxgi_adapter_list[luastg_config.select_adapter].c_str()))
            {
                for (auto& entry : dxgi_adapter_list)
                {
                    if (ImGui::Selectable(entry.c_str()))
                    {
                        luastg_config.select_adapter = &entry - dxgi_adapter_list.data();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo(i18n_c_str("setting-canvas-size"), dxgi_output_mode_list[luastg_config.select_mode].name.c_str()))
            {
                for (auto& entry : dxgi_output_mode_list)
                {
                    if (ImGui::Selectable(entry.name.c_str()))
                    {
                        luastg_config.select_mode = &entry - dxgi_output_mode_list.data();
                    }
                }
                ImGui::EndCombo();
            }

            bool is_fullscreen = !luastg_config.windowed;
            ImGui::Checkbox(i18n_c_str("setting-fullscreen"), &is_fullscreen);
            luastg_config.windowed = !is_fullscreen;

            ImGui::Checkbox(i18n_c_str("setting-vsync"), &luastg_config.vsync);

            if (ImGui::Button(i18n_c_str("setting-cancel")))
            {
                wantExit();
            }
            ImGui::SameLine();
            if (ImGui::Button(i18n_c_str("setting-save")))
            {
                saveConfigToJson();
                wantExit();
            }
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImVec4 const clear_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ID3D11RenderTargetView* rtvs[] = { d3d11_rtv.Get() };
        d3d11_devctx->OMSetRenderTargets(1, rtvs, NULL);
        d3d11_devctx->ClearRenderTargetView(rtvs[0], (FLOAT*)&clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        dxgi_swapchain->Present(0, 0);
    }
    void OnScaling(UINT dpi)
    {
        win32_window_dpi = dpi;
        updateWindowSize();
        UpdateStyleAndFont();
        ImGui_ImplDX11_InvalidateDeviceObjects();
        ImGui_ImplDX11_CreateDeviceObjects();
    }

    void wantExit()
    {
        want_exit = TRUE;
    }
    bool refreshOutputModeList()
    {
        bool const canvas_mode = true;
        dxgi_output_mode_list.clear();
        if (!canvas_mode && dxgi_swapchain)
        {
            HRESULT hr = S_OK;

            Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
            dxgi_swapchain->GetContainingOutput(&dxgi_output);

            UINT mode_count = 0;
            hr = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, NULL);
            if (FAILED(hr))
            {
                throw std::runtime_error("IDXGIOutput::GetDisplayModeList failed.");
                return false;
            }
            std::vector<DXGI_MODE_DESC> mode_list(mode_count);
            hr = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, mode_list.data());
            if (FAILED(hr))
            {
                throw std::runtime_error("IDXGIOutput::GetDisplayModeList failed.");
                return false;
            }
            for (auto it = mode_list.begin(); it != mode_list.end();)
            {
                if (it->Scaling != DXGI_MODE_SCALING_UNSPECIFIED)
                {
                    it = mode_list.erase(it);
                }
                else
                {
                    it++;
                }
            }
            dxgi_output_mode_list.reserve(mode_list.size());
            char buffer[64] = {};
            for (auto& mode : mode_list)
            {
                std::snprintf(buffer, 64, "%ux%u %.2fHz", mode.Width, mode.Height, (double)mode.RefreshRate.Numerator / (double)mode.RefreshRate.Denominator);
                dxgi_output_mode_list.emplace_back(DisplayMode{ buffer, mode });
            }
        }
        if (canvas_mode)
        {
            // 在这里自定义你的画布分辨率
            // Customize your canvas size here
            dxgi_output_mode_list.emplace_back(DisplayMode("640x480 (x1)"    ,  640,  480)); // x1
            dxgi_output_mode_list.emplace_back(DisplayMode("800x600 (x1.25)" ,  800,  600)); // x1.25
            dxgi_output_mode_list.emplace_back(DisplayMode("960x720 (x1.5)"  ,  960,  720)); // x1.5
            dxgi_output_mode_list.emplace_back(DisplayMode("1024x768 (x1.6)" , 1024,  768)); // x1.6
            dxgi_output_mode_list.emplace_back(DisplayMode("1152x864 (x1.8)" , 1152,  864)); // x1.8
            dxgi_output_mode_list.emplace_back(DisplayMode("1280x960 (x2)"   , 1280,  960)); // x2
            //dxgi_output_mode_list.emplace_back(DisplayMode("1440x1050"       , 1400, 1050));
            dxgi_output_mode_list.emplace_back(DisplayMode("1600x1200 (x2.5)", 1600, 1200)); // x2.5
            //dxgi_output_mode_list.emplace_back(DisplayMode("1792x1344"       , 1792, 1344));
            //dxgi_output_mode_list.emplace_back(DisplayMode("1856x1392"       , 1856, 1392));
            dxgi_output_mode_list.emplace_back(DisplayMode("1920x1440 (x3)"  , 1920, 1440)); // x3
            dxgi_output_mode_list.emplace_back(DisplayMode("2240x1680 (x3.5)", 2240, 1680)); // x3.5
            dxgi_output_mode_list.emplace_back(DisplayMode("2560x1920 (x4)"  , 2560, 1920)); // x4
            dxgi_output_mode_list.emplace_back(DisplayMode("2880x2160 (x4.5)", 2880, 2160)); // x4.5
            dxgi_output_mode_list.emplace_back(DisplayMode("3200x2400 (x5)"  , 2880, 2400)); // x5
        }
        return true;
    }
    
    void UpdateStyleAndFont()
    {
        if (!ImGui::GetCurrentContext()) return;

        ImGuiIO& io = ImGui::GetIO();
        float const scaling = Platform::HighDPI::ScalingFromDpi(win32_window_dpi);

        ApplyStyle();

        ImFontGlyphRangesBuilder builder;
        for (ImWchar c = 0x20; c < 0x7F; c += 1)
        {
            builder.AddChar(c);
        }
        for (auto& i : i18n_map)
        {
            for (auto& pair : i)
            {
                builder.AddText(pair.first.data());
                builder.AddText(pair.second.data());
            }
        }
        static ImVector<ImWchar> ranges;
        ranges.clear();
        builder.BuildRanges(&ranges);

        io.Fonts->Clear();
        ImFontConfig font_cfg;
        font_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_NoHinting;
        ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f * scaling, &font_cfg, ranges.Data);
        if (!font)
        {
            font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttf", 16.0f * scaling, &font_cfg, ranges.Data);
        }
        if (!font)
        {
            throw std::runtime_error("ImFontAtlas::AddFontFromFileTTF failed.");
        }
        io.Fonts->Build();
    }
    void ApplyStyle()
    {
        if (!ImGui::GetCurrentContext()) return;
        
        float const scaling = Platform::HighDPI::ScalingFromDpi(win32_window_dpi);

        ImGuiStyle style;
        if (Platform::WindowTheme::ShouldApplicationEnableDarkMode())
            ImGui::StyleColorsDark(&style);
        else
            ImGui::StyleColorsLight(&style);
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;
        style.WindowBorderSize = 1.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.TabRounding = 0.0f;
        style.WindowRounding = 0.0f;
        style.ScaleAllSizes(scaling);
        ImGui::GetStyle() = style;
    }

    bool CreateDirectX()
    {
        HRESULT hr = S_OK;

        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory))))
        {
            throw std::runtime_error("CreateDXGIFactory1 failed.");
        }
        if (FAILED(dxgi_factory->EnumAdapters1(0, &dxgi_adapter)))
        {
            throw std::runtime_error("IDXGIFactory1::EnumAdapters1 failed.");
        }
        
        dxgi_adapter_list.clear();
        Microsoft::WRL::ComPtr<IDXGIFactory6> dxgi_factory6;
        dxgi_factory.As(&dxgi_factory6);
        Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter1;
        DXGI_ADAPTER_DESC1 adapter_info = {};
        for (UINT index = 0; SUCCEEDED(dxgi_factory->EnumAdapters1(index, &dxgi_adapter1)); index += 1)
        //for (UINT index = 0; SUCCEEDED(dxgi_factory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgi_adapter1))); index += 1)
        {
            hr = dxgi_adapter1->GetDesc1(&adapter_info);
            if (FAILED(hr))
            {
                throw std::runtime_error("IDXGIAdapter1::GetDesc1 failed.");
                return false;
            }
            bool soft_dev_type = (adapter_info.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (adapter_info.Flags & DXGI_ADAPTER_FLAG_REMOTE);
            if (!soft_dev_type)
                dxgi_adapter_list.emplace_back(utility::encoding::to_utf8(adapter_info.Description));
        }
        if (dxgi_adapter_list.empty())
        {
            dxgi_adapter_list.emplace_back(); // WOW, you didn't have a GPU!
        }

        D3D_FEATURE_LEVEL target_levels[3] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        hr = D3D11CreateDevice(
            dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT, target_levels, 3, D3D11_SDK_VERSION,
            &d3d11_device, &d3d11_feature_level, &d3d11_devctx);
        if (FAILED(hr))
        {
            throw std::runtime_error("D3D11CreateDevice failed.");
            return false;
        }
        
        auto swpachain = DXGI_SWAP_CHAIN_DESC{
            .BufferDesc = DXGI_MODE_DESC{
                .Width = win32_window_width,
                .Height = win32_window_height,
                .RefreshRate = DXGI_RATIONAL{
                    .Numerator = 0,
                    .Denominator = 0,
                },
                .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
                .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
                .Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
            },
            .SampleDesc = DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = win32_window,
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
            .Flags = 0,
        };
        hr = dxgi_factory->CreateSwapChain(d3d11_device.Get(), &swpachain, &dxgi_swapchain);
        if (FAILED(hr))
        {
            throw std::runtime_error("IDXGIFactory1::CreateSwapChain failed.");
            return false;
        }

        hr = dxgi_factory->MakeWindowAssociation(win32_window, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
        {
            throw std::runtime_error("IDXGIFactory1::MakeWindowAssociation failed.");
            return false;
        }

        refreshOutputModeList();
        
        if (!CreateRenderTarget())
        {
            return false;
        }

        return true;
    }
    void DestroyDirectX()
    {
        DestroyRenderTarget();
        dxgi_swapchain.Reset();
        d3d11_feature_level = D3D_FEATURE_LEVEL_10_0;
        d3d11_devctx.Reset();
        d3d11_device.Reset();
        dxgi_adapter.Reset();
        dxgi_factory.Reset();
    }
    bool CreateRenderTarget()
    {
        HRESULT hr = S_OK;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
        hr = dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&d3d11_texture2d));
        if (FAILED(hr))
        {
            throw std::runtime_error("IDXGISwapChain::GetBuffer failed.");
            return false;
        }

        hr = d3d11_device->CreateRenderTargetView(d3d11_texture2d.Get(), NULL, &d3d11_rtv);
        if (FAILED(hr))
        {
            throw std::runtime_error("ID3D11Device::CreateRenderTargetView failed.");
            return false;
        }

        return true;
    }
    void DestroyRenderTarget()
    {
        if (d3d11_devctx)
        {
            d3d11_devctx->ClearState();
            d3d11_devctx->Flush();
        }
        d3d11_rtv.Reset();
    }

    void moveToCenter()
    {
        HMONITOR monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitor_info = { sizeof(MONITORINFO) };
        if (!GetMonitorInfoA(monitor, &monitor_info))
        {
            throw std::runtime_error("GetMonitorInfoA failed."); // 这不应该发生
        }

        RECT rc = {};
        if (!GetWindowRect(win32_window, &rc))
        {
            throw std::runtime_error("AdjustWindowRectExForDpi failed."); // 这不应该发生
        }

        LONG const x = (monitor_info.rcWork.left + monitor_info.rcWork.right) / 2 - (rc.left + rc.right) / 2;
        LONG const y = (monitor_info.rcWork.top + monitor_info.rcWork.bottom) / 2 - (rc.top + rc.bottom) / 2;
        if (!SetWindowPos(win32_window, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE))
        {
            throw std::runtime_error("SetWindowPos failed."); // 这不应该发生
        }
    }
    void updateWindowSize()
    {
        RECT rc = {
            .left = 0,
            .top = 0,
            .right = 0,
            .bottom = 0,
        };
        if (!Platform::HighDPI::AdjustWindowRectExForDpi(&rc, win32_window_style, FALSE, win32_window_style_ex, win32_window_dpi))
        {
            throw std::runtime_error("AdjustWindowRectExForDpi failed.");
        }
        win32_window_width = Platform::HighDPI::ScalingByDpi<UINT>(WINDOW_SIZE_X, win32_window_dpi);
        win32_window_height = Platform::HighDPI::ScalingByDpi<UINT>(WINDOW_SIZE_Y, win32_window_dpi);
        rc.right += win32_window_width;
        rc.bottom += win32_window_height;
        if (!SetWindowPos(win32_window, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE))
        {
            throw std::runtime_error("SetWindowPos failed.");
        }
    }
    void updateTitle()
    {
        std::wstring str(utility::encoding::to_wide(i18n("window-title")));
        SetWindowTextW(win32_window, str.c_str());
    }

    void loadConfigFromJson()
    {
        if (std::filesystem::is_regular_file(L"config.json"))
        {
            std::ifstream file(L"config.json", std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                nlohmann::json json;
                file >> json;
                luastg_config.adapter = json["gpu"].get<std::string>();
                luastg_config.width = json["width"].get<int>();
                luastg_config.height = json["height"].get<int>();
                luastg_config.refresh_rate_numerator = json["refresh_rate_numerator"].get<int>();
                luastg_config.refresh_rate_denominator = json["refresh_rate_denominator"].get<int>();
                luastg_config.windowed = json["windowed"].get<bool>();
                luastg_config.vsync = json["vsync"].get<bool>();
            }
        }

        luastg_config.select_adapter = 0;
        for (auto& v : dxgi_adapter_list)
        {
            if (luastg_config.adapter == v)
            {
                luastg_config.select_adapter = &v - dxgi_adapter_list.data();
                break;
            }
        }

        luastg_config.select_mode = 0;
        bool find_mode = false;
        for (auto& v : dxgi_output_mode_list)
        {
            if (luastg_config.width == (int)v.mode.Width && luastg_config.height == (int)v.mode.Height
                && luastg_config.refresh_rate_numerator == (int)v.mode.RefreshRate.Numerator
                && luastg_config.refresh_rate_denominator == (int)v.mode.RefreshRate.Denominator)
            {
                luastg_config.select_mode = &v - dxgi_output_mode_list.data();
                find_mode = true;
                break;
            }
        }
        if (!find_mode)
        {
            for (auto& v : dxgi_output_mode_list)
            {
                if (luastg_config.width == (int)v.mode.Width && luastg_config.height == (int)v.mode.Height)
                {
                    luastg_config.select_mode = &v - dxgi_output_mode_list.data();
                    find_mode = true;
                    break;
                }
            }
        }
        if (!find_mode)
        {
            for (auto& v : dxgi_output_mode_list)
            {
                if (luastg_config.width == (int)v.mode.Width || luastg_config.height == (int)v.mode.Height)
                {
                    luastg_config.select_mode = &v - dxgi_output_mode_list.data();
                    find_mode = true;
                    break;
                }
            }
        }
    }
    void saveConfigToJson()
    {
        luastg_config.adapter = dxgi_adapter_list[luastg_config.select_adapter];
        auto& mode = dxgi_output_mode_list[luastg_config.select_mode].mode;
        luastg_config.width = mode.Width;
        luastg_config.height = mode.Height;
        luastg_config.refresh_rate_numerator = mode.RefreshRate.Numerator;
        luastg_config.refresh_rate_denominator = mode.RefreshRate.Denominator;

        std::ofstream file(L"config.json", std::ios::out | std::ios::binary | std::ios::trunc);
        if (file.is_open())
        {
            nlohmann::json json;
            json["gpu"] = luastg_config.adapter;
            json["width"] = luastg_config.width;
            json["height"] = luastg_config.height;
            json["refresh_rate_numerator"] = luastg_config.refresh_rate_numerator;
            json["refresh_rate_denominator"] = luastg_config.refresh_rate_denominator;
            json["windowed"] = luastg_config.windowed;
            json["vsync"] = luastg_config.vsync;
            file << json;
        }
    }

    Window()
        : win32_wincls({ sizeof(WNDCLASSEXW) })
        , win32_wincls_atom(0)
        , win32_window(NULL)
        , win32_window_style(WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX))
        , win32_window_style_ex(0)
        , win32_window_width(WINDOW_SIZE_X)
        , win32_window_height(WINDOW_SIZE_Y)
        , win32_window_dpi(USER_DEFAULT_SCREEN_DPI)
        , is_open(FALSE)
    {
        win32_wincls.style = CS_HREDRAW | CS_VREDRAW;
        win32_wincls.lpfnWndProc = &win32_window_proc;
        win32_wincls.hInstance = GetModuleHandleW(NULL);
        win32_wincls.lpszClassName = L"SettingWindow";

        win32_wincls_atom = RegisterClassExW(&win32_wincls);
        if (win32_wincls_atom == 0)
        {
            throw std::runtime_error("RegisterClassExW failed."); // 这不应该发生
        }
        
        win32_window = CreateWindowExW(
            win32_window_style_ex, win32_wincls.lpszClassName, L"", win32_window_style,
            0, 0, (int)win32_window_width, (int)win32_window_height,
            NULL, NULL, win32_wincls.hInstance, this);
        if (win32_window == NULL)
        {
            throw std::runtime_error("CreateWindowExW failed."); // 这不应该发生
        }
        Platform::WindowTheme::UpdateColorMode(win32_window, TRUE);
        win32_window_dpi = Platform::HighDPI::GetDpiForWindow(win32_window);
        updateWindowSize();
        moveToCenter();
        updateTitle();

        ShowWindow(win32_window, SW_SHOWDEFAULT);
        UpdateWindow(win32_window);

        CreateDirectX();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = NULL;
        ImGui_ImplWin32_Init(win32_window);
        ImGui_ImplDX11_Init(d3d11_device.Get(), d3d11_devctx.Get());
        UpdateStyleAndFont();
        is_open = TRUE;

        loadConfigFromJson();
    }
    ~Window()
    {
        if (is_open)
        {
            is_open = FALSE;
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            DestroyDirectX();
        }
        if (win32_window) DestroyWindow(win32_window); win32_window = NULL;
        if (win32_wincls_atom) UnregisterClassW(win32_wincls.lpszClassName, win32_wincls.hInstance); win32_wincls_atom = 0;
        win32_wincls = WNDCLASSEXW{ sizeof(WNDCLASSEXW) };
    }
};

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    try
    {
        Window window;
        return window.Run();
    }
    catch (std::runtime_error const& e)
    {
        std::wstring err(utility::encoding::to_wide(e.what()));
        MessageBoxW(NULL, err.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
    return 0;
}
