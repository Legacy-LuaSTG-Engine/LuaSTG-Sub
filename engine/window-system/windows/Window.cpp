#include "windows/Window.hpp"
#include "core/Logger.hpp"
#include "core/Application.hpp"
#include "core/FrameRateController.hpp"
#include "core/SmartReference.hpp"
#include "core/Configuration.hpp"
#include "utf8.hpp"
#include "simdutf.h"
#include "win32/win32.hpp"
#include "win32/abi.hpp"
#include <dwmapi.h>
#include "windows/WindowsVersion.hpp"
#include "windows/WindowTheme.hpp"
#include "windows/RuntimeLoader/DesktopWindowManager.hpp"
#include "win32/base.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr int LUASTG_WM_UPDAE_TITLE = WM_APP + __LINE__;
    constexpr int LUASTG_WM_RECREATE = WM_APP + __LINE__;
    constexpr int LUASTG_WM_SET_WINDOW_MODE = WM_APP + __LINE__;
    constexpr int LUASTG_WM_SET_FULLSCREEN_MODE = WM_APP + __LINE__;
    constexpr int LUASTG_WM_UPDATE = WM_APP + __LINE__;

    Platform::RuntimeLoader::DesktopWindowManager dwmapi_loader;

    DWORD mapWindowStyle(const core::WindowFrameStyle style, const bool fullscreen) {
        if (fullscreen) {
            return WS_VISIBLE | WS_POPUP;
        }
        switch (style) {
        case core::WindowFrameStyle::None:
            return WS_VISIBLE | WS_POPUP;
        case core::WindowFrameStyle::Fixed:
            return WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
        case core::WindowFrameStyle::Normal:
            return WS_VISIBLE | WS_OVERLAPPEDWINDOW;
        default:
            assert(false); return WS_VISIBLE | WS_POPUP;
        }
    }

    void clearWindowBackground(const HWND window) {
        PAINTSTRUCT ps{};
        if (auto const dc = BeginPaint(window, &ps); dc != nullptr) {
            RECT rc{};
            GetClientRect(window, &rc);
            auto const brush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
            FillRect(dc, &rc, brush);
            EndPaint(window, &ps);
        }
    }

    void setAllowWindowCorner(const HWND window, const bool allow) {
        DWM_WINDOW_CORNER_PREFERENCE const attr = allow ? DWMWCP_DEFAULT : DWMWCP_DONOTROUND;
        if (!win32::check_hresult_as_boolean(
            dwmapi_loader.SetWindowAttribute(window, DWMWA_WINDOW_CORNER_PREFERENCE, &attr, sizeof(attr)),
            "SetWindowAttribute"sv
        )) {
            core::Logger::error("[core] DwmSetWindowAttribute ({}) failed"sv, allow ? "DWMWCP_DEFAULT"sv : "DWMWCP_DONOTROUND"sv);
        }
    }
}

namespace core {
    void       Window::textInput_updateBuffer() {
        auto const& s32 = m_text_input_buffer;
        auto& s8 = m_text_input_buffer_u8;
        s8.resize(simdutf::utf8_length_from_utf32(s32.data(), s32.size()));
        simdutf::convert_valid_utf32_to_utf8(s32.data(), s32.size(), reinterpret_cast<char*>(s8.data()));
    }
    void       Window::textInput_addChar32(char32_t const code) {
        m_text_input_buffer.insert(m_text_input_buffer.begin() + m_text_input_cursor, code);
        m_text_input_cursor += 1;
        textInput_updateBuffer();
    }
    void       Window::textInput_handleChar32(char32_t const code) {
        if (code == U'\t' || code == U'\n' /* || code == U'\r' */ || (code >= 0x20 && code <= 0x7e) || code >= 0x80) {
            textInput_addChar32(code);
        }
    }
    void       Window::textInput_handleChar16(char16_t const code) {
        if (IS_HIGH_SURROGATE(code)) {
            m_text_input_last_high_surrogate = code;
        }
        else if (IS_LOW_SURROGATE(code)) {
            if (IS_SURROGATE_PAIR(m_text_input_last_high_surrogate, code)) {
                char16_t const str[3]{ m_text_input_last_high_surrogate, code ,0 };
                char32_t buf[2]{};
                simdutf::convert_valid_utf16le_to_utf32(str, 2, buf);
                textInput_handleChar32(buf[0]);
            }
            m_text_input_last_high_surrogate = {};
        }
        else {
            textInput_handleChar32(static_cast<char32_t>(code));
        }
    }

    bool       Window::textInput_isEnabled() {
        return m_text_input_enabled;
    }
    void       Window::textInput_setEnabled(bool const enabled) {
        m_text_input_enabled = enabled;
    }
    StringView Window::textInput_getBuffer() {
        return { reinterpret_cast<char const*>(m_text_input_buffer_u8.data()), m_text_input_buffer_u8.size() };
    }
    void       Window::textInput_clearBuffer() {
        m_text_input_buffer.clear();
        m_text_input_buffer_u8.clear();
        m_text_input_cursor = 0;
    }
    uint32_t   Window::textInput_getCursorPosition() {
        return m_text_input_cursor;
    }
    void       Window::textInput_setCursorPosition(uint32_t const code_point_index) {
        m_text_input_cursor = std::min(code_point_index, static_cast<uint32_t>(m_text_input_buffer.size()));
    }
    void       Window::textInput_addCursorPosition(int32_t const offset_by_code_point) {
        if (offset_by_code_point == 0) {
            return;
        }
        if (offset_by_code_point > 0) {
            if ((static_cast<uint64_t>(m_text_input_cursor) + offset_by_code_point) <= m_text_input_buffer.size()) {
                m_text_input_cursor += offset_by_code_point;
            }
            else {
                m_text_input_cursor = static_cast<uint32_t>(m_text_input_buffer.size());
            }
        }
        else {
            if ((static_cast<int64_t>(m_text_input_cursor) + offset_by_code_point) >= 0) {
                m_text_input_cursor += offset_by_code_point;
            }
            else {
                m_text_input_cursor = 0;
            }
        }
    }
    void       Window::textInput_removeBufferRange(uint32_t const code_point_index, uint32_t const code_point_count) {
        auto& s32 = m_text_input_buffer;
        if (code_point_index >= s32.size()) {
            return;
        }
        auto const right = static_cast<uint32_t>(s32.size()) - code_point_index;
        auto const count = std::min(code_point_count, right);
        s32.erase(code_point_index, count);
        m_text_input_cursor = std::min(m_text_input_cursor, static_cast<uint32_t>(s32.size()));
        textInput_updateBuffer();
    }
    void       Window::textInput_insertBufferRange(uint32_t const code_point_index, StringView const str) {
        if (!simdutf::validate_utf8(str.data(), str.size())) {
            return;
        }
        std::u32string buf;
        buf.resize(simdutf::utf32_length_from_utf8(str.data(), str.size()));
        simdutf::convert_valid_utf8_to_utf32(str.data(), str.size(), buf.data());
        if (code_point_index < m_text_input_buffer.size()) {
            m_text_input_buffer.insert(code_point_index, buf);
        }
        else {
            m_text_input_buffer.append(buf);
        }
        m_text_input_cursor += static_cast<uint32_t>(str.size());
        textInput_updateBuffer();
    }
    void       Window::textInput_backspace(uint32_t const code_point_count) {
        auto const count = std::min(m_text_input_cursor, code_point_count);
        m_text_input_cursor -= count;
        m_text_input_buffer.erase(m_text_input_cursor, count);
        textInput_updateBuffer();
    }

    void Window::setInputMethodPosition(Vector2I position) {
        if (!win32_window) {
            return;
        }
        if (HIMC himc = ImmGetContext(win32_window); win32_window) {
            COMPOSITIONFORM composition_form = {};
            composition_form.ptCurrentPos.x = position.x;
            composition_form.ptCurrentPos.y = position.y;
            composition_form.dwStyle = CFS_FORCE_POSITION;
            ImmSetCompositionWindow(himc, &composition_form);
            CANDIDATEFORM candidate_form = {};
            candidate_form.dwStyle = CFS_CANDIDATEPOS;
            candidate_form.ptCurrentPos.x = position.x;
            candidate_form.ptCurrentPos.y = position.y;
            ImmSetCandidateWindow(himc, &candidate_form);
            ImmReleaseContext(win32_window, himc);
        }
    }
}

namespace core {
    LRESULT CALLBACK Window::win32_window_callback(HWND const window, UINT const message, WPARAM const arg1, LPARAM const arg2) {
        if (auto const self = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA))) {
            return self->onMessage(window, message, arg1, arg2);
        }
        if (message == WM_NCCREATE) {
            win32::enableNonClientDpiScaling(window);
            if (auto const params = reinterpret_cast<CREATESTRUCTW*>(arg2); params->lpCreateParams) {
                auto const self = static_cast<Window*>(params->lpCreateParams);

                SetLastError(0);
                SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
                if (auto const code = GetLastError(); code != 0) {
                    Logger::error("[core] (LastError = {}) SetWindowLongPtrW (GWLP_USERDATA) failed", code);
                    return FALSE;
                }

                Platform::WindowTheme::UpdateColorMode(window, TRUE);
                setAllowWindowCorner(window, self->m_allow_windows_11_window_corner);
            }
        }
        return DefWindowProcW(window, message, arg1, arg2);
    }

    LRESULT Window::onMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2) {
        // Window focus debug
        if (enable_track_window_focus && win32_window_want_track_focus) {
            HWND focus_window = GetForegroundWindow();
            if (focus_window && focus_window != window)
            {
                win32_window_want_track_focus = FALSE;
                std::array<WCHAR, 256> buffer1{};
                std::array<WCHAR, 256> buffer2{};
                LRESULT const length1 = SendMessageW(focus_window, WM_GETTEXT, 256, (LPARAM)buffer1.data());
                int const length2 = GetClassNameW(focus_window, buffer2.data(), 256);
                Logger::info("[core] The window focus has been gained by: [hwnd: {}] (window class: {}) {}",
                    reinterpret_cast<intptr_t>(focus_window),
                    utf8::to_string(std::wstring_view(buffer2.data(), static_cast<size_t>(length2))),
                    utf8::to_string(std::wstring_view(buffer1.data(), static_cast<size_t>(length1)))
                );
            }
        }

        // Text input
        if (m_text_input_enabled) {
            switch (message) {
            case WM_CHAR:
                if (arg1 <= 0xFFFF) {
                    textInput_handleChar16(static_cast<char16_t>(arg1));
                }
                break;
            }
        }
        
        // Window size/move
        if (auto const result = m_sizemove.handleSizeMove(window, message, arg1, arg2); result.bReturn) {
            return result.lResult;
        }

        // TitleBar customize
        if (auto const result = m_title_bar_controller.handleWindowMessage(window, message, arg1, arg2); result.returnResult) {
            return result.result;
        }
        
        // Additional callback
        for (auto& v : m_eventobj) {
            if (v) {
                const auto r = v->onNativeWindowMessage(window, message, arg1, arg2);
                if (r.should_return) {
                    return r.result;
                }
            }
        }

        // Messages
        switch (message) {
        case WM_ACTIVATEAPP:
            if (arg1 /* == TRUE */)
            {
                win32_window_want_track_focus = FALSE;
                Platform::WindowTheme::UpdateColorMode(window, TRUE);
                dispatchEvent(EventType::WindowActive);
            }
            else
            {
                win32_window_want_track_focus = TRUE; // 要开始抓内鬼了
                Platform::WindowTheme::UpdateColorMode(window, FALSE);
                dispatchEvent(EventType::WindowInactive);
            }
            m_alt_down = FALSE; // 无论如何，清除该按键状态
            break;
        case WM_SIZE:
            if (!m_ignore_size_message)
            {
                EventData d = {};
                d.window_size = Vector2U(LOWORD(arg2), HIWORD(arg2));
                dispatchEvent(EventType::WindowSize, d);
            }
            break;
        case WM_ENTERSIZEMOVE:
            win32_window_is_sizemove = TRUE;
            InvalidateRect(window, NULL, FALSE); // 标记窗口区域为需要重新绘制，以便产生 WM_PAINT 消息
            return 0;
        case WM_EXITSIZEMOVE:
            win32_window_is_sizemove = FALSE;
            return 0;
        case WM_ENTERMENULOOP:
            win32_window_is_menu_loop = TRUE;
            InvalidateRect(window, NULL, FALSE); // 标记窗口区域为需要重新绘制，以便产生 WM_PAINT 消息
            return 0;
        case WM_EXITMENULOOP:
            win32_window_is_menu_loop = FALSE;
            return 0;
        case WM_PAINT:
            if (win32_window_is_sizemove || win32_window_is_menu_loop) {
                
            }
            ValidateRect(window, nullptr);
            return 0;
        case LUASTG_WM_UPDATE:
            if (const auto application = core::ApplicationManager::getApplication(); application != nullptr) {
                const auto frame_rate_controller = core::IFrameRateController::getInstance();
                if (core::ApplicationManager::isDelegateUpdateEnabled() && !core::ApplicationManager::isUpdating() && frame_rate_controller->arrived()) {
                    core::ApplicationManager::runBeforeUpdate();
                    core::ApplicationManager::runUpdate();
                }
            }
            return 0;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if (arg1 == VK_MENU)
            {
                m_alt_down = TRUE;
                return 0;
            }
            if (m_alt_down && arg1 == VK_RETURN)
            {
                _toggleFullScreenMode();
                return 0;
            }
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            if (arg1 == VK_MENU)
            {
                m_alt_down = FALSE;
                return 0;
            }
            break;
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* info = (MINMAXINFO*)arg2;
            RECT rect_min = { 0, 0, 320, 240 };
            UINT const dpi = win32::getDpiForWindow(window);
            if (m_title_bar_controller.adjustWindowRectExForDpi(&rect_min, win32_window_style, FALSE, win32_window_style_ex, dpi))
            {
                info->ptMinTrackSize.x = rect_min.right - rect_min.left;
                info->ptMinTrackSize.y = rect_min.bottom - rect_min.top;
            }
        }
        return 0;
        case WM_DPICHANGED:
            if (getFrameStyle() != WindowFrameStyle::None)
            {
                RECT rc{};
                GetWindowRect(window, &rc);
                setSize(getSize()); // 刷新一次尺寸（因为非客户区可能会变化）
                SetWindowPos(window, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                dispatchEvent(EventType::WindowDpiChanged);
                return 0;
            }
            dispatchEvent(EventType::WindowDpiChanged); // 仍然需要通知
            break;
        case WM_SETTINGCHANGE:
        case WM_THEMECHANGED:
            Platform::WindowTheme::UpdateColorMode(window, TRUE);
            break;
        case WM_DEVICECHANGE:
            if (arg1 == 0x0007 /* DBT_DEVNODES_CHANGED */)
            {
                dispatchEvent(EventType::DeviceChange);
            }
            break;
        case WM_SETCURSOR:
            if (LOWORD(arg2) == HTCLIENT)
            {
                SetCursor(win32_window_cursor);
                return TRUE;
            }
            break;
        case WM_MENUCHAR:
            // 快捷键能不能死全家
            return MAKELRESULT(0, MNC_CLOSE);
        case WM_SYSCOMMAND:
            // 鼠标左键点击标题栏图标或者 Alt+Space 不会出现菜单
            switch (arg1 & 0xFFF0)
            {
            case SC_KEYMENU:
            case SC_MOUSEMENU:
                return 0;
            }
            break;
        case WM_CLOSE:
            dispatchEvent(EventType::WindowClose);
            PostQuitMessage(EXIT_SUCCESS);
            return 0;
        case LUASTG_WM_UPDAE_TITLE:
            SetWindowTextW(window, win32_window_text_w.data());
            return 0;
        case LUASTG_WM_RECREATE:
            _recreateWindow();
            return 0;
        case LUASTG_WM_SET_WINDOW_MODE:
            _setWindowMode(reinterpret_cast<SetWindowedModeParameters*>(arg1), arg2);
            return 0;
        case LUASTG_WM_SET_FULLSCREEN_MODE:
            _setFullScreenMode(reinterpret_cast<IDisplay*>(arg2));
            return 0;
        }
        const auto result = DefWindowProcW(window, message, arg1, arg2);
        if (!m_window_created) {
            // 在 CreateWindow/CreateWindowEx 期间，
            // 即使设置了窗口类的 hbrBackground 为黑色笔刷，窗口背景也会先绘制为白色，再转为黑色。
            // 只有不断追着窗口消息重绘背景，才能 **一定程度** 上避免白色背景（不保证 100% 有效）。
            // CreateWindow/CreateWindowEx 一般会产生以下窗口消息（按先后顺序）：
            // - WM_GETMINMAXINFO
            // - WM_NCCREATE
            // - WM_NCCALCSIZE
            // - WM_CREATE
            // - WM_SHOWWINDOW
            // - WM_WINDOWPOSCHANGING
            // - WM_NCPAINT
            // - WM_GETICON
            // - WM_ERASEBKGND
            // - WM_GETICON
            // - WM_ACTIVATEAPP
            // - WM_NCACTIVATE
            // - WM_ACTIVATE
            // - WM_IME_SETCONTEXT
            // - WM_IME_NOTIFY
            // - WM_SETFOCUS
            // - WM_WINDOWPOSCHANGED
            // - WM_SIZE
            // - WM_MOVE
            // 经过测试，如果在以下三类消息之后重绘背景为黑色，能极大程度地减少看到白色背景的概率：
            // - WM_NCPAINT
            // - WM_GETICON
            // - WM_ERASEBKGND
            // 但仅处理以上三类消息，仍然会有一定概率出现白色背景，
            // 因此，有理由猜测白色背景的绘制在窗口弹出动画期间都会持续执行（可能由桌面窗口管理器合成）。
            clearWindowBackground(window);
        }
        return result;
    }
    bool Window::createWindowClass() {
        auto const instance_handle = GetModuleHandleW(nullptr);
        if (instance_handle == nullptr) {
            assert(false); // unlikely
            return false;
        }

        HICON icon{};
        if (win32_window_icon_id != 0) {
            icon = LoadIcon(instance_handle, MAKEINTRESOURCE(win32_window_icon_id));
        }

        auto& cls = win32_window_class;
        cls.style = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc = &win32_window_callback;
        cls.hInstance = instance_handle;
        cls.hIcon = icon;
        cls.hCursor = LoadCursor(nullptr, IDC_ARROW);
        cls.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        cls.lpszClassName = L"LuaSTG::Sub::Window";
        cls.hIconSm = icon;

        win32_window_class_atom = RegisterClassExW(&cls);

        if (icon != nullptr) {
            DestroyIcon(icon);
        }

        if (win32_window_class_atom == 0) {
            Logger::error("[core] (LastError = {}) RegisterClassExW failed", GetLastError());
            return false;
        }

        return true;
    }
    void Window::destroyWindowClass()  {
        if (win32_window_class_atom != 0) {
            UnregisterClassW(win32_window_class.lpszClassName, win32_window_class.hInstance);
        }
        win32_window_class_atom = 0;
    }
    bool Window::createWindow() {
        if (win32_window_class_atom == 0) {
            return false;
        }

        // 拿到原点位置的显示器信息
        SmartReference<IDisplay> display;
        if (!IDisplay::getPrimary(display.put())) {
            return false; // 理论上 Windows 平台的主显示器都在原点
        }
        auto const display_rect = display->getWorkAreaRect();
        auto const display_dpi = static_cast<UINT>(display->getDisplayScale() * USER_DEFAULT_SCREEN_DPI);

        // 计算初始大小

        RECT client{ 0, 0, static_cast<LONG>(win32_window_width),static_cast<LONG>(win32_window_height) };
        m_title_bar_controller.setEnable(auto_hide_title_bar);
        m_title_bar_controller.adjustWindowRectExForDpi(&client, win32_window_style, FALSE, win32_window_style_ex, display_dpi);

        // 创建窗口

        convertTitleText();
        win32_window = CreateWindowExW(
            win32_window_style_ex,
            win32_window_class.lpszClassName,
            win32_window_text_w.data(),
            win32_window_style,
            (display_rect.a.x + display_rect.b.x) / 2 - (client.right - client.left) / 2, // x (left)
            (display_rect.a.y + display_rect.b.y) / 2 - (client.bottom - client.top) / 2, // y (top)
            client.right - client.left,
            client.bottom - client.top,
            nullptr, // parent
            nullptr, // menu
            win32_window_class.hInstance,
            this
        );
        if (win32_window == nullptr) {
            Logger::error("[core] (LastError = {}) CreateWindowExW failed", GetLastError());
            return false;
        }
        m_window_created = true;

        // 配置输入法

        if (!win32_window_ime_enable) {
            ImmAssociateContext(win32_window, nullptr);
        }

        // 配置窗口挪动器

        m_sizemove.setWindow(win32_window);

        // 启动消息生成器

        m_heartbeat_running = true;
        m_heartbeat_thread = std::move(std::jthread([this]() -> void {
            const auto frame_rate_controller = core::IFrameRateController::getInstance();
            constexpr UINT flags{ SMTO_BLOCK };
            while (m_heartbeat_running) {
                while (m_heartbeat_running && !frame_rate_controller->arrived()) {
                    Sleep(0);
                }
                SetLastError(ERROR_SUCCESS);
                const LRESULT result = SendMessageTimeoutW(win32_window, LUASTG_WM_UPDATE, 0, 0, flags, 100, nullptr);
                if (result == 0) {
                    const DWORD last_error = GetLastError();
                    if (last_error != ERROR_SUCCESS && last_error != ERROR_TIMEOUT) {
                        core::Logger::error("[core] SendMessageTimeoutW failed (last error {})"sv, GetLastError());
                    }
                }
            }
        }));

        return true;
    }
    void Window::destroyWindow() {
        m_heartbeat_running = false;
        if (m_heartbeat_thread.joinable()) {
            m_heartbeat_thread.join();
        }
        m_sizemove.setWindow(nullptr);
        m_window_created = false;
        if (win32_window) {
            DestroyWindow(win32_window);
            win32_window = nullptr;
        }
    }
    bool Window::_recreateWindow() {
        BOOL result{ FALSE };
        WINDOWPLACEMENT last_window_placement{};
        last_window_placement.length = sizeof(last_window_placement);

        assert(win32_window);
        if (result = GetWindowPlacement(win32_window, &last_window_placement); !result) {
            assert(result);
            return false;
        }
        
        destroyWindow();
        if (!createWindow()) {
            return false;
        }

        assert(win32_window);
        if (result = SetWindowPlacement(win32_window, &last_window_placement); !result) {
            assert(result);
            return false;
        }
        
        return true;
    }
    bool Window::recreateWindow() {
        dispatchEvent(EventType::WindowDestroy);
        SendMessageW(win32_window, LUASTG_WM_RECREATE, 0, 0);
        //if (!_recreateWindow()) {
        //	return false;
        //}
        dispatchEvent(EventType::WindowCreate);
        return true;
    }
    void Window::_toggleFullScreenMode() {
        if (m_fullscreen_mode) {
            SetWindowedModeParameters parameters{};
            parameters.size = Vector2U(win32_window_width, win32_window_height);
            parameters.style = m_framestyle;
            _setWindowMode(&parameters, true);
        }
        else {
            _setFullScreenMode(nullptr);
        }
    }
    void Window::_setWindowMode(SetWindowedModeParameters* parameters, bool ignore_size) {
        assert(parameters);

        m_title_bar_controller.setEnable(auto_hide_title_bar);

        HMONITOR win32_monitor{};
        if (parameters->display) {
            win32_monitor = static_cast<HMONITOR>(parameters->display->getNativeHandle());
        }
        else {
            win32_monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST);
        }
        assert(win32_monitor);
        MONITORINFO monitor_info = {};
        monitor_info.cbSize = sizeof(monitor_info);
        BOOL const get_monitor_info_result = GetMonitorInfoW(win32_monitor, &monitor_info);
        assert(get_monitor_info_result); (void)get_monitor_info_result;
        assert(monitor_info.rcWork.right > monitor_info.rcWork.left);
        assert(monitor_info.rcWork.bottom > monitor_info.rcWork.top);

        bool const new_fullsceen_mode = false;
        m_framestyle = parameters->style;
        DWORD new_win32_window_style = mapWindowStyle(m_framestyle, new_fullsceen_mode);

        RECT rect = { 0, 0, (int32_t)parameters->size.x, (int32_t)parameters->size.y };
        m_title_bar_controller.adjustWindowRectExForDpi(
            &rect, new_win32_window_style, FALSE, 0,
            win32::getDpiForWindow(win32_window));

        //m_ignore_size_message = TRUE;
        SetLastError(0);
        SetWindowLongPtrW(win32_window, GWL_STYLE, new_win32_window_style);
        DWORD const set_style_result = GetLastError();
        assert(set_style_result == 0); (void)set_style_result;
        //SetLastError(0);
        //SetWindowLongPtrW(win32_window, GWL_EXSTYLE, 0);
        //DWORD const set_style_ex_result = GetLastError();
        //assert(set_style_ex_result == 0); (void)set_style_ex_result;
        //m_ignore_size_message = FALSE;

        bool want_restore_placement = false;

        if (m_fullscreen_mode && ignore_size)
        {
            want_restore_placement = true;
        }
        else
        {
            BOOL const set_window_pos_result = SetWindowPos(
                win32_window,
                HWND_TOP,
                (monitor_info.rcWork.right + monitor_info.rcWork.left) / 2 - (rect.right - rect.left) / 2,
                (monitor_info.rcWork.bottom + monitor_info.rcWork.top) / 2 - (rect.bottom - rect.top) / 2,
                rect.right - rect.left,
                rect.bottom - rect.top,
                SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            assert(set_window_pos_result); (void)set_window_pos_result;
        }

        RECT client_rect = {};
        BOOL get_client_rect_result = GetClientRect(win32_window, &client_rect);
        assert(get_client_rect_result); (void)get_client_rect_result;

        m_fullscreen_mode = new_fullsceen_mode;
        win32_window_style = new_win32_window_style;
        win32_window_width = UINT(client_rect.right - client_rect.left);
        win32_window_height = UINT(client_rect.bottom - client_rect.top);

        EventData event_data{};
        event_data.window_fullscreen_state = false;
        dispatchEvent(EventType::WindowFullscreenStateChange, event_data);

        if (want_restore_placement)
        {
            BOOL const set_placement_result = SetWindowPlacement(win32_window, &m_last_window_placement);
            assert(set_placement_result); (void)set_placement_result;
        }
    }
    void Window::_setFullScreenMode(IDisplay* display) {
        m_title_bar_controller.setEnable(false);

        if (!m_fullscreen_mode)
        {
            BOOL const get_placement_result = GetWindowPlacement(win32_window, &m_last_window_placement);
            assert(get_placement_result); (void)get_placement_result;
        }

        HMONITOR win32_monitor{};
        if (display) {
            win32_monitor = static_cast<HMONITOR>(display->getNativeHandle());
        }
        else {
            win32_monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST);
        }
        assert(win32_monitor);
        MONITORINFO monitor_info = {};
        monitor_info.cbSize = sizeof(monitor_info);
        BOOL const get_monitor_info_result = GetMonitorInfoW(win32_monitor, &monitor_info);
        assert(get_monitor_info_result); (void)get_monitor_info_result;
        assert(monitor_info.rcMonitor.right > monitor_info.rcMonitor.left);
        assert(monitor_info.rcMonitor.bottom > monitor_info.rcMonitor.top);

        bool const new_fullsceen_mode = true;
        DWORD new_win32_window_style = mapWindowStyle(m_framestyle, new_fullsceen_mode);

        //m_ignore_size_message = TRUE;
        SetLastError(0);
        SetWindowLongPtrW(win32_window, GWL_STYLE, new_win32_window_style);
        DWORD const set_style_result = GetLastError();
        assert(set_style_result == 0); (void)set_style_result;
        //SetLastError(0);
        //SetWindowLongPtrW(win32_window, GWL_EXSTYLE, 0);
        //DWORD const set_style_ex_result = GetLastError();
        //assert(set_style_ex_result == 0); (void)set_style_ex_result;
        //m_ignore_size_message = FALSE;

        BOOL const set_window_pos_result = SetWindowPos(
            win32_window,
            HWND_TOP,
            monitor_info.rcMonitor.left,
            monitor_info.rcMonitor.top,
            monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
            monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        assert(set_window_pos_result); (void)set_window_pos_result;

        m_fullscreen_mode = new_fullsceen_mode;
        win32_window_style = new_win32_window_style;
        win32_window_width = UINT(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
        win32_window_height = UINT(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

        EventData event_data{};
        event_data.window_fullscreen_state = true;
        dispatchEvent(EventType::WindowFullscreenStateChange, event_data);
    }

    void Window::convertTitleText() {
        win32_window_text_w[0] = L'\0';
        int const size = MultiByteToWideChar(CP_UTF8, 0, win32_window_text.data(), (int)win32_window_text.size(), NULL, 0);
        if (size <= 0 || size > (int)(win32_window_text_w.size() - 1))
        {
            assert(false); return;
        }
        win32_window_text_w[size] = L'\0';
        int const result = MultiByteToWideChar(CP_UTF8, 0, win32_window_text.data(), (int)win32_window_text.size(), win32_window_text_w.data(), size);
        if (result <= 0 || result != size)
        {
            assert(false); return;
        }
        win32_window_text_w[result] = L'\0';
    }

    RectI Window::getRect() {
        RECT rc = {};
        GetWindowRect(win32_window, &rc);
        return RectI(rc.left, rc.top, rc.right, rc.bottom);
    }
    bool Window::setRect(RectI v) {
        return SetWindowPos(win32_window, NULL,
            v.a.x, v.a.y,
            v.b.x - v.a.x, v.b.y - v.a.y,
            SWP_NOZORDER) != FALSE;
    }
    RectI Window::getClientRect() {
        RECT rc = {};
        GetClientRect(win32_window, &rc);
        return RectI(rc.left, rc.top, rc.right, rc.bottom);
    }
    bool Window::setClientRect(RectI v)  {
        // 更新 DPI
        win32_window_dpi = win32::getDpiForWindow(win32_window);
        // 计算包括窗口框架的尺寸
        RECT rc = { v.a.x , v.a.y , v.b.x , v.b.y };
        m_title_bar_controller.adjustWindowRectExForDpi(
            &rc,
            win32_window_style,
            FALSE,
            win32_window_style_ex,
            win32_window_dpi);
        // 获取最近的显示器的位置
        if (HMONITOR monitor = MonitorFromWindow(win32_window, MONITOR_DEFAULTTONEAREST))
        {
            MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
            if (GetMonitorInfoA(monitor, &moninfo))
            {
                // 偏移到该显示器0点位置
                rc.left += moninfo.rcMonitor.left;
                rc.right += moninfo.rcMonitor.left;
                rc.top += moninfo.rcMonitor.top;
                rc.bottom += moninfo.rcMonitor.top;
            }
        }
        // 最后再应用
        return SetWindowPos(win32_window, NULL,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_NOZORDER) != FALSE;
    }
    uint32_t Window::getDPI() {
        win32_window_dpi = win32::getDpiForWindow(win32_window);
        return win32_window_dpi;
    }

    void Window::dispatchEvent(EventType t, EventData d) {
        // 回调
        m_is_dispatch_event = true;
        switch (t)
        {
        case EventType::WindowCreate:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowCreate();
            }
            break;
        case EventType::WindowDestroy:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowDestroy();
            }
            break;
        case EventType::WindowActive:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowActive();
            }
            break;
        case EventType::WindowInactive:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowInactive();
            }
            break;
        case EventType::WindowClose:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowClose();
            }
            break;
        case EventType::WindowSize:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowSize(d.window_size);
            }
            break;
        case EventType::WindowFullscreenStateChange:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowFullscreenStateChange(d.window_fullscreen_state);
            }
            break;
        case EventType::WindowDpiChanged:
            for (auto& v : m_eventobj)
            {
                if (v) v->onWindowDpiChange();
            }
            break;
        case EventType::DeviceChange:
            for (auto& v : m_eventobj)
            {
                if (v) v->onDeviceChange();
            }
            break;
        }
        m_is_dispatch_event = false;
        // 处理那些延迟的对象
        removeEventListener(nullptr);
        for (auto& v : m_eventobj_late)
        {
            m_eventobj.emplace_back(v);
        }
        m_eventobj_late.clear();
    }
    void Window::addEventListener(IWindowEventListener* e) {
        removeEventListener(e);
        if (m_is_dispatch_event)
        {
            m_eventobj_late.emplace_back(e);
        }
        else
        {
            m_eventobj.emplace_back(e);
        }
    }
    void Window::removeEventListener(IWindowEventListener* e) {
        if (m_is_dispatch_event)
        {
            for (auto& v : m_eventobj)
            {
                if (v == e)
                {
                    v = nullptr; // 不破坏遍历过程
                }
            }
        }
        else
        {
            for (auto it = m_eventobj.begin(); it != m_eventobj.end();)
            {
                if (*it == e)
                    it = m_eventobj.erase(it);
                else
                    it++;
            }
        }
    }

    void* Window::getNativeHandle() { return win32_window; }

    void Window::setIMEState(bool enable) {
        if (!win32_window_ime_enable && enable) {
        #pragma warning(push)
        #pragma warning(disable: 6387)
            // See: https://learn.microsoft.com/en-us/windows/win32/api/imm/nf-imm-immassociatecontextex
            // If the application calls this function with IACE_DEFAULT, the operating system restores the default input method context for the window.
            ImmAssociateContextEx(win32_window, nullptr /* In this case, the hIMC parameter is ignored. */, IACE_DEFAULT);
        #pragma warning (pop)
        }
        else if (win32_window_ime_enable && !enable) {
            ImmAssociateContext(win32_window, nullptr);
        }
        win32_window_ime_enable = enable;
    }
    bool Window::getIMEState() {
        return win32_window_ime_enable;
    }

    void Window::setTitleText(StringView const str) {
        win32_window_text = str;
        m_title_bar_controller.setTitle(std::string(str));
        convertTitleText();
        if (win32_window) {
            PostMessageW(win32_window, LUASTG_WM_UPDAE_TITLE, 0, 0);
        }
    }
    StringView Window::getTitleText() {
        return win32_window_text;
    }

    bool Window::setFrameStyle(WindowFrameStyle style) {
        m_framestyle = style;
        win32_window_style = mapWindowStyle(m_framestyle, m_fullscreen_mode);
        SetWindowLongPtrW(win32_window, GWL_STYLE, win32_window_style);
        //SetWindowLongPtrW(win32_window, GWL_EXSTYLE, win32_window_style_ex);
        constexpr UINT flags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW;
        SetWindowPos(win32_window, nullptr, 0, 0, 0, 0, flags);
        return true;
    }
    WindowFrameStyle Window::getFrameStyle() {
        return m_framestyle;
    }

    Vector2U Window::getSize() {
        return { win32_window_width, win32_window_height };
    }
    Vector2U Window::_getCurrentSize() {
        RECT rc{};
        GetClientRect(win32_window, &rc);
        return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
    }
    bool Window::setSize(Vector2U v) {
        win32_window_width = v.x;
        win32_window_height = v.y;
        return setClientRect(RectI(0, 0, (int)v.x, (int)v.y));
    }
    bool Window::setLayer(WindowLayer const layer) {
        HWND native_layer{};
        switch (layer) {
        case WindowLayer::Bottom:
            native_layer = HWND_BOTTOM;
            break;
        case WindowLayer::Normal:
            native_layer = HWND_NOTOPMOST;
            break;
        case WindowLayer::Top:
            native_layer = HWND_TOP;
            break;
        case WindowLayer::TopMost:
            native_layer = HWND_TOPMOST;
            break;
        default:
            assert(false);
            return false;
        }
        return SetWindowPos(win32_window, native_layer, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW) != FALSE;
    }

    float Window::getDPIScaling()  {
        return win32::getScalingFromDpi(getDPI());
    }

    void Window::setWindowMode(Vector2U size, WindowFrameStyle style, IDisplay* display) {
        SetWindowedModeParameters parameters{};
        parameters.size = size;
        parameters.style = style;
        parameters.display = display;
        SendMessageW(win32_window, LUASTG_WM_SET_WINDOW_MODE, reinterpret_cast<WPARAM>(&parameters), FALSE);
    }
    void Window::setFullScreenMode(IDisplay* display) {
        SendMessageW(win32_window, LUASTG_WM_SET_FULLSCREEN_MODE, 0, reinterpret_cast<LPARAM>(display));
    }
    void Window::setCentered(bool show, IDisplay* display) {
        core::SmartReference<IDisplay> local_display;
        if (!display) {
            if (!IDisplay::getNearestFromWindow(this, local_display.put())) {
                return;
            }
            display = local_display.get();
        }
        RECT r{};
        [[maybe_unused]] auto const result1 = GetWindowRect(win32_window, &r);
        assert(result1);
        auto const& m = display->getWorkAreaRect();
        UINT flags = SWP_FRAMECHANGED;
        if (show) {
            flags |= SWP_SHOWWINDOW;
        }
        [[maybe_unused]] auto const result2 = SetWindowPos(
            win32_window,
            HWND_TOP,
            (m.a.x + m.b.x) / 2 - (r.right - r.left) / 2,
            (m.a.y + m.b.y) / 2 - (r.bottom - r.top) / 2,
            r.right - r.left,
            r.bottom - r.top,
            flags);
        assert(result2);
    }

    void Window::setCustomSizeMoveEnable(bool v) {
        m_sizemove.setEnable(v ? TRUE : FALSE);
    }
    void Window::setCustomMinimizeButtonRect(RectI v) {
        m_sizemove.setMinimizeButtonRect(RECT{
            .left = v.a.x,
            .top = v.a.y,
            .right = v.b.x,
            .bottom = v.b.y,
            });
    }
    void Window::setCustomCloseButtonRect(RectI v) {
        m_sizemove.setCloseButtonRect(RECT{
            .left = v.a.x,
            .top = v.a.y,
            .right = v.b.x,
            .bottom = v.b.y,
            });
    }
    void Window::setCustomMoveButtonRect(RectI v) {
        m_sizemove.setTitleBarRect(RECT{
            .left = v.a.x,
            .top = v.a.y,
            .right = v.b.x,
            .bottom = v.b.y,
            });
    }

    bool Window::setCursor(WindowCursor type) {
        m_cursor = type;
        switch (type)
        {
        default:
            assert(false); return false;

        case WindowCursor::None:
            win32_window_cursor = NULL;
            break;

        case WindowCursor::Arrow:
            win32_window_cursor = LoadCursor(NULL, IDC_ARROW);
            break;
        case WindowCursor::Hand:
            win32_window_cursor = LoadCursor(NULL, IDC_HAND);
            break;

        case WindowCursor::Cross:
            win32_window_cursor = LoadCursor(NULL, IDC_CROSS);
            break;
        case WindowCursor::TextInput:
            win32_window_cursor = LoadCursor(NULL, IDC_IBEAM);
            break;

        case WindowCursor::Resize:
            win32_window_cursor = LoadCursor(NULL, IDC_SIZEALL);
            break;
        case WindowCursor::ResizeEW:
            win32_window_cursor = LoadCursor(NULL, IDC_SIZEWE);
            break;
        case WindowCursor::ResizeNS:
            win32_window_cursor = LoadCursor(NULL, IDC_SIZENS);
            break;
        case WindowCursor::ResizeNESW:
            win32_window_cursor = LoadCursor(NULL, IDC_SIZENESW);
            break;
        case WindowCursor::ResizeNWSE:
            win32_window_cursor = LoadCursor(NULL, IDC_SIZENWSE);
            break;

        case WindowCursor::NotAllowed:
            win32_window_cursor = LoadCursor(NULL, IDC_NO);
            break;
        case WindowCursor::Wait:
            win32_window_cursor = LoadCursor(NULL, IDC_WAIT);
            break;
        }
        POINT pt = {};
        GetCursorPos(&pt);
        SetCursorPos(pt.x, pt.y);
        return true;
    }
    WindowCursor Window::getCursor() {
        return m_cursor;
    }

    void Window::setWindowCornerPreference(bool const allow) {
        m_allow_windows_11_window_corner = allow;
        if (!Platform::WindowsVersion::Is11()) {
            return;
        }
        assert(win32_window);
        setAllowWindowCorner(win32_window, allow);
    }
    void Window::setTitleBarAutoHidePreference(bool const allow) {
        auto_hide_title_bar = allow;
        m_title_bar_controller.setEnable(auto_hide_title_bar && !m_fullscreen_mode);
        if (!m_fullscreen_mode) {
            WINDOWPLACEMENT placement{ .length{sizeof(WINDOWPLACEMENT)} };
            GetWindowPlacement(win32_window, &placement);
            SetWindowPos(win32_window, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            SetWindowPlacement(win32_window, &placement);
        }
        else {
            SetWindowPos(win32_window, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
    }

    Window::Window() {
        auto const& debug_config = ConfigurationLoader::getInstance().getDebug();
        enable_track_window_focus = debug_config.isTrackWindowFocus();

        auto const& window_config = ConfigurationLoader::getInstance().getWindow();
        if (window_config.hasTitle()) {
            win32_window_text = window_config.getTitle();
        }
        m_cursor = window_config.isCursorVisible() ? WindowCursor::Arrow : WindowCursor::None;
        m_allow_windows_11_window_corner = window_config.isAllowWindowCorner();
        auto_hide_title_bar = window_config.isAllowTitleBarAutoHide();

        auto const& graphics_config = ConfigurationLoader::getInstance().getGraphicsSystem();
        win32_window_width = graphics_config.getWidth();
        win32_window_height = graphics_config.getHeight();

        m_title_bar_controller.setTitle(win32_window_text);
        convertTitleText();
        win32_window_dpi = win32::getUserDefaultScreenDpi();

        if (!createWindowClass())
            throw std::runtime_error("createWindowClass failed");
        if (!createWindow())
            throw std::runtime_error("createWindow failed");
    }
    Window::~Window() {
        destroyWindow();
        destroyWindowClass();
    }

    bool Window::create(Window** pp_window) {
        try {
            *pp_window = new Window();
            return true;
        }
        catch (...) {
            *pp_window = nullptr;
            return false;
        }
    }
    bool Window::create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, Window** pp_window) {
        try {
            auto* p = new Window();
            *pp_window = p;
            p->setSize(size);
            p->setTitleText(title_text);
            p->setFrameStyle(style);
            if (show)
                p->setLayer(WindowLayer::Normal);
            return true;
        }
        catch (...) {
            *pp_window = nullptr;
            return false;
        }
    }
}

namespace core {
    bool IWindow::create(IWindow** pp_window) {
        try {
            *pp_window = new Window();
            return true;
        }
        catch (...) {
            *pp_window = nullptr;
            return false;
        }
    }
    bool IWindow::create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, IWindow** pp_window) {
        try {
            auto* p = new Window();
            *pp_window = p;
            p->setSize(size);
            p->setTitleText(title_text);
            p->setFrameStyle(style);
            if (show)
                p->setLayer(WindowLayer::Normal);
            return true;
        }
        catch (...) {
            *pp_window = nullptr;
            return false;
        }
    }
}
