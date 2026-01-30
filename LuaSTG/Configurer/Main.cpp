#include "win32/win32.hpp"
#include "win32/abi.hpp"
#include "windows/WindowTheme.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "luastg_config_generated.h"
#include "LConfig.h"

using std::string_view_literals::operator""sv;
using std::string_literals::operator""s;
using nlohmann::operator""_json_pointer;

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

        {"common-enable", "启用"},
        {"common-default1", "默认"},
        {"common-default2", "（默认）"},

        {"config-show-advance", "显示高级设置"},

        {"config-application", "应用"},
        {"config-application-uuid", "UUID"},
        {"config-application-single-instance", "单例模式"},
        {"config-debug", "调试"},
        {"config-debug-track-window-focus", "记录窗口焦点被哪个应用占用"},
        {"config-logging", "日志"},
        {"config-logging-level", "日志等级"},
        {"config-logging-level-debug", "调试（debug）"},
        {"config-logging-level-info", "信息（info）"},
        {"config-logging-level-warn", "警告（warn）"},
        {"config-logging-level-error", "错误（error）"},
        {"config-logging-level-fatal", "严重错误（fatal）"},
        {"config-logging-debugger", "Windows调试器"},
        {"config-logging-console", "控制台窗口"},
        {"config-logging-console-preserve", "关闭程序后保留控制台窗口"},
        {"config-logging-file", "日志文件"},
        {"config-logging-file-path", "文件路径"},
        {"config-logging-rolling-file", "滚动日志文件"},
        {"config-logging-rolling-file-path", "文件夹路径"},
        {"config-logging-rolling-file-max-history", "保留的文件数量"},
        {"config-timing", "计时系统"},
        {"config-timing-frame-rate", "目标帧率"},
        {"config-timing-frame-rate-warn", "警告：随意修改目标帧率可能会造成严重后果"},
        {"config-window", "窗口"},
        {"config-window-title", "窗口标题"},
        {"config-window-cursor-visible", "显示鼠标"},
        {"config-window-allow-window-corner", "允许窗口圆角（Windows 11）"},
        {"config-graphics-system", "显示"},
        {"config-graphics-system-preferred-device-name", "显卡"},
        {"config-graphics-system-resolution", "分辨率"},
        {"config-graphics-system-fullscreen", "全屏显示"},
        {"config-graphics-system-vsync", "垂直同步（防止画面撕裂）"},
        {"config-audio-system", "音频"},
        {"config-audio-system-sound-effect-volume", "音效音量"},
        {"config-audio-system-music-volume", "背景音乐音量"},
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

        {"common-enable", "Enable"},
        {"common-default1", "Default"},
        {"common-default2", "(Default)"},

        {"config-show-advance", "Show Advance"},

        {"config-application", "Application"},
        {"config-application-uuid", "UUID"},
        {"config-application-single-instance", "Single instance"},
        {"config-debug", "Debug"},
        {"config-debug-track-window-focus", "Log which application has acquired the window focus"},
        {"config-logging", "Logging"},
        {"config-logging-level", "Logging level"},
        {"config-logging-level-debug", "Debug"},
        {"config-logging-level-info", "Info"},
        {"config-logging-level-warn", "Warn"},
        {"config-logging-level-error", "Error"},
        {"config-logging-level-fatal", "Fatal"},
        {"config-logging-debugger", "Windows Debugger"},
        {"config-logging-console", "Console Window"},
        {"config-logging-console-preserve", "Keep console window open after closing the program"},
        {"config-logging-file", "File"},
        {"config-logging-file-path", "File path"},
        {"config-logging-rolling-file", "Rolling File"},
        {"config-logging-rolling-file-path", "Folder path"},
        {"config-logging-rolling-file-max-history", "Number of files to retain"},
        {"config-timing", "Timing"},
        {"config-timing-frame-rate", "Target frame rate"},
        {"config-timing-frame-rate-warn", "Warning: Arbitrarily modifying the target frame rate may lead to serious consequences."},
        {"config-window", "Window"},
        {"config-window-title", "Window title"},
        {"config-window-cursor-visible", "Show mouse cursor"},
        {"config-window-allow-window-corner", "Allow window corner (Windows 11)"},
        {"config-graphics-system", "Display"},
        {"config-graphics-system-preferred-device-name", "Graphics card"},
        {"config-graphics-system-resolution", "Resolution"},
        {"config-graphics-system-fullscreen", "Fullscreen"},
        {"config-graphics-system-vsync", "VSync (prevent screen tearing)"},
        {"config-audio-system", "Audio"},
        {"config-audio-system-sound-effect-volume", "Sound effect volume"},
        {"config-audio-system-music-volume", "Music volume"},
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

// Common Controls

namespace {
#define EDIT_COMMON_PARAMS nlohmann::json& json, const nlohmann::json_pointer<std::string>& path, const std::string_view i18n_id
    void showCheckBoxEdit(EDIT_COMMON_PARAMS, const bool default_value = false) {
        bool enable = json.value(path, default_value);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str(i18n_id));
        ImGui::PushID(i18n_c_str(i18n_id));
        if (ImGui::Checkbox("##", &enable)) {
            json[path] = enable;
        }
        ImGui::PopID();
    }
    void showIntegerEdit(EDIT_COMMON_PARAMS, const int default_value = 0, const int min_value = 0) {
        int value = json.value(path, default_value);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str(i18n_id));
        ImGui::PushID(i18n_c_str(i18n_id));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputInt("##", &value)) {
            json[path] = (std::max)(min_value, value);
        }
        ImGui::PopID();
    }
    void showNumberSliderEdit(EDIT_COMMON_PARAMS, const double default_value = 0, const double min_value = 0.0, const double max_value = 1.0) {
        double value = json.value(path, default_value);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str(i18n_id));
        ImGui::PushID(i18n_c_str(i18n_id));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::SliderScalar("##", ImGuiDataType_Double, &value, &min_value, &max_value)) {
            json[path] = value;
        }
        ImGui::PopID();
    }
    void showTextFieldEdit(EDIT_COMMON_PARAMS, const std::string& default_value) {
        std::string value = json.value(path, default_value);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str(i18n_id));
        ImGui::PushID(i18n_c_str(i18n_id));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##", &value)) {
            json[path] = value;
        }
        ImGui::PopID();
    }
#undef EDIT_COMMON_PARAMS
}

// Logging Level

namespace {
    enum class LoggingLevel {
        debug,
        info,
        warn,
        error,
        fatal,
    };
    LoggingLevel toLoggingLevel(const std::string& s) {
        if (s == "debug"sv) return LoggingLevel::debug;
        if (s == "info"sv) return LoggingLevel::info;
        if (s == "warn"sv) return LoggingLevel::warn;
        if (s == "error"sv) return LoggingLevel::error;
        if (s == "fatal"sv) return LoggingLevel::fatal;
        return LoggingLevel::info;
    }
    std::string_view toLocalizedStringView(const LoggingLevel l) {
        switch (l) {
        case LoggingLevel::debug: return i18n("config-logging-level-debug"sv);
        case LoggingLevel::info: return i18n("config-logging-level-info"sv);
        case LoggingLevel::warn: return i18n("config-logging-level-warn"sv);
        case LoggingLevel::error: return i18n("config-logging-level-error"sv);
        case LoggingLevel::fatal: return i18n("config-logging-level-fatal"sv);
        default: return i18n("config-logging-level-info"sv);
        }
    }
    std::string_view toStringView(const LoggingLevel l) {
        switch (l) {
        case LoggingLevel::debug: return "debug"sv;
        case LoggingLevel::info: return "info"sv;
        case LoggingLevel::warn: return "warn"sv;
        case LoggingLevel::error: return "error"sv;
        case LoggingLevel::fatal: return "fatal"sv;
        default: return "info"sv;
        }
    }
    void showLoggingEnableEdit(nlohmann::json& json, const nlohmann::json_pointer<std::string>& path, const bool default_value = false) {
        showCheckBoxEdit(json, path, "common-enable"sv, default_value);
    }
    void showLoggingThresholdEdit(nlohmann::json& json, const nlohmann::json_pointer<std::string>& path) {
        LoggingLevel threshold = toLoggingLevel(json.value(path, "info"s));
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str("config-logging-level"sv));
        ImGui::PushID(i18n_c_str("config-logging-level"sv));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##", toLocalizedStringView(threshold).data())) {
            bool changed = false;
            if (ImGui::Selectable(i18n_c_str("config-logging-level-debug"sv), threshold == LoggingLevel::debug)) {
                threshold = LoggingLevel::debug;
                changed = true;
            }
            if (ImGui::Selectable(i18n_c_str("config-logging-level-info"sv), threshold == LoggingLevel::info)) {
                threshold = LoggingLevel::info;
                changed = true;
            }
            if (ImGui::Selectable(i18n_c_str("config-logging-level-warn"sv), threshold == LoggingLevel::warn)) {
                threshold = LoggingLevel::warn;
                changed = true;
            }
            if (ImGui::Selectable(i18n_c_str("config-logging-level-error"sv), threshold == LoggingLevel::error)) {
                threshold = LoggingLevel::error;
                changed = true;
            }
            if (ImGui::Selectable(i18n_c_str("config-logging-level-fatal"sv), threshold == LoggingLevel::fatal)) {
                threshold = LoggingLevel::fatal;
                changed = true;
            }
            if (changed) {
                json[path] = toStringView(threshold);
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();
    }
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

constexpr UINT WINDOW_SIZE_X = 400;
constexpr UINT WINDOW_SIZE_Y = 300;

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
    
    nlohmann::json config_json = nlohmann::json::object();
    bool show_advance{false};

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

        Layout();

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
        bool canvas_mode = true;
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
        ApplyStyle();
    }
    void ApplyStyle()
    {
        if (!ImGui::GetCurrentContext()) return;
        
        float const scaling = win32::getScalingFromDpi(win32_window_dpi);

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
        style.FontSizeBase = 16.0f;
        style.FontScaleMain = 1.0f;
        style.FontScaleDpi = scaling;
        ImGui::GetStyle() = style;
    }
    void LayoutApplicationTab() {
        showTextFieldEdit(config_json, "/application/uuid"_json_pointer, "config-application-uuid"sv, ""s);
        showCheckBoxEdit(config_json, "/application/single_instance"_json_pointer, "config-application-single-instance"sv, false);
    }
    void LayoutDebugTab() {
        showCheckBoxEdit(config_json, "/debug/track_window_focus"_json_pointer, "config-debug-track-window-focus"sv, false);
    }
    void LayoutLoggingTab() {
        ImGui::PushID(1);
        ImGui::SeparatorText(i18n_c_str("config-logging-debugger"));
        showLoggingEnableEdit(config_json, "/logging/debugger/enable"_json_pointer);
        showLoggingThresholdEdit(config_json, "/logging/debugger/threshold"_json_pointer);
        ImGui::PopID();

        ImGui::PushID(2);
        ImGui::SeparatorText(i18n_c_str("config-logging-console"));
        showLoggingEnableEdit(config_json, "/logging/console/enable"_json_pointer);
        showLoggingThresholdEdit(config_json, "/logging/console/threshold"_json_pointer);
        showCheckBoxEdit(config_json, "/logging/console/preserve"_json_pointer, "config-logging-console-preserve"sv);
        ImGui::PopID();

        ImGui::PushID(3);
        ImGui::SeparatorText(i18n_c_str("config-logging-file"));
        showLoggingEnableEdit(config_json, "/logging/file/enable"_json_pointer, true);
        showLoggingThresholdEdit(config_json, "/logging/file/threshold"_json_pointer);
        showTextFieldEdit(config_json, "/logging/file/path"_json_pointer, "config-logging-file-path"sv, LUASTG_LOGGING_DEFAULT_FILE_PATH ""s);
        ImGui::PopID();

        ImGui::PushID(4);
        ImGui::SeparatorText(i18n_c_str("config-logging-rolling-file"));
        showLoggingEnableEdit(config_json, "/logging/rolling_file/enable"_json_pointer);
        showLoggingThresholdEdit(config_json, "/logging/rolling_file/threshold"_json_pointer);
        showTextFieldEdit(config_json, "/logging/rolling_file/path"_json_pointer, "config-logging-rolling-file-path"sv, ""s);
        showIntegerEdit(config_json, "/logging/rolling_file/max_history"_json_pointer, "config-logging-rolling-file-max-history"sv, 10, 1);
        ImGui::PopID();
    }
    void LayoutTimingTab() {
        showIntegerEdit(config_json, "/timing/frame_rate"_json_pointer, "config-timing-frame-rate"sv, 60, 1);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImColor(1.0f, 0.1f, 0.1f), i18n_c_str("config-timing-frame-rate-warn"sv));
    }
    void LayoutWindowTab() {
        if (show_advance) {
	        showTextFieldEdit(config_json, "/window/title"_json_pointer, "config-window-title"sv, LUASTG_INFO ""s);
	        showCheckBoxEdit(config_json, "/window/cursor_visible"_json_pointer, "config-window-cursor-visible"sv, true);
        }
        showCheckBoxEdit(config_json, "/window/allow_window_corner"_json_pointer, "config-window-allow-window-corner"sv, true);
    }
    void LayoutGraphicsSystemTab() {
        const auto& preferred_device_name = config_json.value("/graphics_system/preferred_device_name"_json_pointer, ""s);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str("config-graphics-system-preferred-device-name"sv));
        ImGui::PushID(i18n_c_str("config-graphics-system-preferred-device-name"sv));
        ImGui::SetNextItemWidth(-FLT_MIN);
    	if (ImGui::BeginCombo("##", !preferred_device_name.empty() ? preferred_device_name.c_str() : i18n_c_str("common-default2"sv))) {
            if (ImGui::Selectable(i18n_c_str("common-default2"sv), preferred_device_name.empty())) {
                config_json["/graphics_system/preferred_device_name"_json_pointer] = ""sv;
            }
            for (const auto& device_name : dxgi_adapter_list) {
                if (ImGui::Selectable(device_name.c_str(), device_name == preferred_device_name)) {
                    config_json["/graphics_system/preferred_device_name"_json_pointer] = device_name;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        const int width = config_json.value("/graphics_system/width"_json_pointer, 640);
        const int height = config_json.value("/graphics_system/height"_json_pointer, 480);
        const auto resolution = std::format("{}x{}"sv, width, height);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(i18n_c_str("config-graphics-system-resolution"sv));
        ImGui::PushID(i18n_c_str("config-graphics-system-resolution"sv));
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##", resolution.c_str())) {
            for (double s = 1.0; s < 9.00001; s += 0.25) {
                const int w = static_cast<int>(640 * s);
                const int h = static_cast<int>(480 * s);
                const auto r = std::format("{}x{}", w, h);
                if (ImGui::Selectable(r.c_str(), width == w && height == h)) {
                    config_json["/graphics_system/width"_json_pointer] = w;
                    config_json["/graphics_system/height"_json_pointer] = h;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        showCheckBoxEdit(config_json, "/graphics_system/fullscreen"_json_pointer, "config-graphics-system-fullscreen"sv, false);
        showCheckBoxEdit(config_json, "/graphics_system/vsync"_json_pointer, "config-graphics-system-vsync"sv, false);
    }
    void LayoutAudioSystemTab() {
        // TODO: "/audio_system/preferred_endpoint_name"_json_pointer
        showNumberSliderEdit(config_json, "/audio_system/sound_effect_volume"_json_pointer, "config-audio-system-sound-effect-volume"sv, 1.0);
        showNumberSliderEdit(config_json, "/audio_system/music_volume"_json_pointer, "config-audio-system-music-volume"sv, 1.0);
    }
    void Layout() {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)win32_window_width, (float)win32_window_height), ImGuiCond_Always);
        if (ImGui::Begin("##MainWindow", nullptr, (ImGuiWindowFlags_NoDecoration ^ ImGuiWindowFlags_NoScrollbar) | ImGuiWindowFlags_NoBackground))
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

            if (ImGui::Button(i18n_c_str("setting-cancel"))) {
                wantExit();
            }
            ImGui::SameLine();
            if (ImGui::Button(i18n_c_str("setting-save"))) {
                saveConfigToJson();
                wantExit();
            }
            ImGui::SameLine();
            ImGui::Checkbox(i18n_c_str("config-show-advance"), &show_advance);

            if (ImGui::BeginTabBar("##SettingTabs")) {
                if (show_advance && ImGui::BeginTabItem(i18n_c_str("config-application"))) {
                    LayoutApplicationTab();
                    ImGui::EndTabItem();
                }
                if (show_advance && ImGui::BeginTabItem(i18n_c_str("config-debug"))) {
                    LayoutDebugTab();
                    ImGui::EndTabItem();
                }
                if (show_advance && ImGui::BeginTabItem(i18n_c_str("config-logging"))) {
                    LayoutLoggingTab();
                    ImGui::EndTabItem();
                }
                if (show_advance && ImGui::BeginTabItem(i18n_c_str("config-timing"))) {
                    LayoutTimingTab();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(i18n_c_str("config-window"))) {
                    LayoutWindowTab();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(i18n_c_str("config-graphics-system"))) {
                    LayoutGraphicsSystemTab();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(i18n_c_str("config-audio-system"))) {
                    LayoutAudioSystemTab();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
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
                dxgi_adapter_list.emplace_back(utf8::to_string(adapter_info.Description));
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
        if (!win32::adjustWindowRectExForDpi(&rc, win32_window_style, FALSE, win32_window_style_ex, win32_window_dpi))
        {
            throw std::runtime_error("AdjustWindowRectExForDpi failed.");
        }
        win32_window_width = win32::scaleByDpi(WINDOW_SIZE_X, win32_window_dpi);
        win32_window_height = win32::scaleByDpi(WINDOW_SIZE_Y, win32_window_dpi);
        rc.right += win32_window_width;
        rc.bottom += win32_window_height;
        if (!SetWindowPos(win32_window, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE))
        {
            throw std::runtime_error("SetWindowPos failed.");
        }
    }
    void updateTitle()
    {
        std::wstring str(utf8::to_wstring(i18n("window-title")));
        SetWindowTextW(win32_window, str.c_str());
    }

    void loadConfigFromJson() {
        if (std::filesystem::is_regular_file(L"config.json")) {
            std::ifstream file(L"config.json", std::ios::in | std::ios::binary);
            if (file.is_open()) {
                config_json = nlohmann::json::object();
                file >> config_json;
            }
        }
    }
    void saveConfigToJson() {
        std::ofstream file(L"config.json", std::ios::out | std::ios::binary | std::ios::trunc);
        if (file.is_open()) {
            file << config_json.dump(4);
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
        , win32_window_dpi(win32::getUserDefaultScreenDpi())
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
        win32_window_dpi = win32::getDpiForWindow(win32_window);
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

        auto const scaling = ImGui_ImplWin32_GetDpiScaleForHwnd(win32_window);
        ImFontConfig font_cfg;
        font_cfg.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_NoHinting | ImGuiFreeTypeLoaderFlags_LoadColor;
        ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f * scaling, &font_cfg, nullptr);
        if (!font)
        {
            font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttf", 16.0f * scaling, &font_cfg, nullptr);
        }
        if (!font)
        {
            throw std::runtime_error("ImFontAtlas::AddFontFromFileTTF failed.");
        }

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
        std::wstring err(utf8::to_wstring(e.what()));
        MessageBoxW(NULL, err.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
    return 0;
}
