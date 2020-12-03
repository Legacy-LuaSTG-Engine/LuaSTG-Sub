#include "slow/slow.hpp"
#include "RefImpl.hpp"
#include "EncodeConvert.hpp"
#include <cassert>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <Windows.h>
#include <Windowsx.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace slow {
    class MonitorHelperImpl : public RefImpl<MonitorHelper> {
    private:
        MonitorInfo _info_def;
        std::vector<MonitorInfo> _info;
        static BOOL CALLBACK Monitorenumproc(HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) {
            MonitorHelperImpl* self = (MonitorHelperImpl*)(ptrdiff_t)Arg4;
            MONITORINFO infoex; // using MONITORINFOEXW to get monitor name
            ZeroMemory(&infoex, sizeof(infoex));
            infoex.cbSize = sizeof(infoex);
            BOOL b = GetMonitorInfoW(Arg1, &infoex);
            if (b != FALSE) {
                MonitorInfo saveinfo;
                saveinfo.primary = infoex.dwFlags & MONITORINFOF_PRIMARY;
                saveinfo.rect.left   = infoex.rcMonitor.left;
                saveinfo.rect.right  = infoex.rcMonitor.right;
                saveinfo.rect.top    = infoex.rcMonitor.top;
                saveinfo.rect.bottom = infoex.rcMonitor.bottom;
                saveinfo.work.left   = infoex.rcWork.left;
                saveinfo.work.right  = infoex.rcWork.right;
                saveinfo.work.top    = infoex.rcWork.top;
                saveinfo.work.bottom = infoex.rcWork.bottom;
                self->_info.push_back(saveinfo);
            }
            return TRUE;
        }
    public:
        bool refresh() {
            return FALSE != EnumDisplayMonitors(
                NULL, NULL, MonitorHelperImpl::Monitorenumproc, (LPARAM)(ptrdiff_t)this);
        }
        uint32_t getSize() {
            return _info.size();
        }
        MonitorInfo getInfo(uint32_t index) {
            if (index < _info.size()) {
                return _info[index];
            }
            else {
                return _info_def;
            }
        }
        void print() {
            uint32_t idx = 0;
            for (auto& i : _info) {
                std::printf("Monitor[%u] primary:%s rect:(%d, %d, %d, %d)[%dx%d] work:(%d, %d, %d, %d)[%dx%d]\n", idx,
                    i.primary ? "true" : "false",
                    i.rect.left, i.rect.right, i.rect.top, i.rect.bottom,
                    i.rect.right - i.rect.left, i.rect.bottom - i.rect.top,
                    i.work.left, i.work.right, i.work.top, i.work.bottom,
                    i.work.right - i.work.left, i.work.bottom - i.work.top);
                idx++;
            }
        }
    public:
        MonitorHelperImpl() : _info_def({ false, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }) {
            refresh();
        }
        virtual ~MonitorHelperImpl() {}
    };
    
    bool MonitorHelper::create(MonitorHelper** output) {
        if (output != nullptr) {
            try {
                MonitorHelperImpl* p = new MonitorHelperImpl;
                *output = (MonitorHelper*)p;
                return true;
            }
            catch (...) {}
        }
        return false;
    }
    MonitorHelper* MonitorHelper::create() {
        MonitorHelper* output = nullptr;
        MonitorHelper::create(&output);
        return output;
    }
    
    class WindowImpl : public RefImpl<Window> {
    private:
        HINSTANCE _instance;
        HWND _window;
        ATOM _atom;
        WindowLayer _layer_v;
        HWND _layer;
        WindowStyle _style_v;
        DWORD _style;
        DWORD _style_ex;
        std::array<WCHAR, 64> _classname;
        std::string _title;
        std::wstring _wtitle;
        Vector2I _size;
        bool _sizemove;
        bool _exit;
        std::array<bool, 256> _input_botton;
        Vector2I _input_pointer_position;
        int32_t _input_pointer_wheel;
        UINT_PTR _timer;
        WindowCallback* _callback;
    private:
        static LRESULT WINAPI _window_callback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            WindowImpl* master = (WindowImpl*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            if (master != NULL) {
                switch (msg) {
                    // window focus
                    case WM_ACTIVATEAPP: {
                        master->_input_botton.fill(false);
                        master->_input_pointer_position = Vector2I { 0, 0 };
                        master->_input_pointer_wheel = 0;
                        if (wParam) {
                        }
                        else {
                        }
                        break;
                    }
                    // keyboard input
                    case WM_KEYUP:
                    case WM_SYSKEYUP: {
                        uint32_t vk = wParam;
                        switch (vk) {
                            case VK_SHIFT: {
                                vk = MapVirtualKeyW((lParam & 0x00ff0000) >> 16u, MAPVK_VSC_TO_VK_EX);
                                master->_input_botton[VK_LSHIFT] = false;
                                master->_input_botton[VK_RSHIFT] = false;
                                break;
                            }
                            case VK_CONTROL: {
                                vk = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
                                break;
                            }
                            case VK_MENU: {
                                vk = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
                                break;
                            }
                        }
                        master->_input_botton[vk] = false;
                        break;
                    }
                    case WM_KEYDOWN:
                    case WM_SYSKEYDOWN: {
                        uint32_t vk = wParam;
                        switch (vk) {
                        case VK_SHIFT:
                            vk = MapVirtualKeyW((lParam & 0x00ff0000) >> 16u, MAPVK_VSC_TO_VK_EX);
                            break;
                        case VK_CONTROL:
                            vk = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
                            break;
                        case VK_MENU:
                            vk = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
                            break;
                        }
                        master->_input_botton[vk] = true;
                        break;
                    }
                    // mouse input
                    case WM_MOUSEMOVE: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        break;
                    }
                    case WM_LBUTTONDOWN: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Left] = true;
                        break;
                    }
                    case WM_LBUTTONUP: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Left] = false;
                        break;
                    }
                    case WM_RBUTTONDOWN: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Right] = true;
                        break;
                    }
                    case WM_RBUTTONUP: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Right] = false;
                        break;
                    }
                    case WM_MBUTTONDOWN: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Middle] = true;
                        break;
                    }
                    case WM_MBUTTONUP: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_botton[(size_t)PointerBotton::Middle] = false;
                        break;
                    }
                    case WM_MOUSEWHEEL: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        master->_input_pointer_wheel += GET_WHEEL_DELTA_WPARAM(wParam);
                        break;
                    }
                    case WM_XBUTTONDOWN: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        switch (GET_XBUTTON_WPARAM(wParam)) {
                        case XBUTTON1:
                            master->_input_botton[(size_t)PointerBotton::X1] = true;
                            break;
                        case XBUTTON2:
                            master->_input_botton[(size_t)PointerBotton::X2] = true;
                            break;
                        }
                        break;
                    }
                    case WM_XBUTTONUP: {
                        master->_input_pointer_position.x = GET_X_LPARAM(lParam);
                        master->_input_pointer_position.y = GET_Y_LPARAM(lParam);
                        switch (GET_XBUTTON_WPARAM(wParam)) {
                        case XBUTTON1:
                            master->_input_botton[(size_t)PointerBotton::X1] = false;
                            break;
                        case XBUTTON2:
                            master->_input_botton[(size_t)PointerBotton::X2] = false;
                            break;
                        }
                        break;
                    }
                    // sizing moving
                    case WM_ENTERSIZEMOVE: {
                        master->_sizemove = true;
                        if (0 == master->_timer) {
                            master->_timer = ::SetTimer(master->_window, 0, USER_TIMER_MINIMUM, nullptr);
                        }
                        return 0;
                    }
                    case WM_EXITSIZEMOVE: {
                        master->_sizemove = false;
                        if (0 != master->_timer) {
                            ::KillTimer(master->_window, master->_timer);
                        }
                        return 0;
                    }
                    case WM_SIZE: {
                        uint32_t w = LOWORD(lParam);
                        uint32_t h = HIWORD(lParam);
                        master->_size = Vector2I { (int32_t)w, (int32_t)h };
                        if (nullptr != master->_callback) {
                            master->_callback->onSize(w, h);
                        }
                        break;
                    }
                    // timer
                    case WM_TIMER: {
                        if (nullptr != master->_callback) {
                            master->_callback->onTimer();
                        }
                        return 0; // we process this message
                    }
                    // exit window loop
                    case WM_DESTROY: {
                        master->setWindowClose(true);
                        break;
                    }
                }
            }
            switch (msg) {
                case WM_DESTROY: {
                    ::PostQuitMessage(0);
                    return 0;
                }
            }
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        }
        void _impl_updatestyle(WindowStyle style) {
            switch (style) {
            default:
            case WindowStyle::Normal:
                _style = WS_OVERLAPPEDWINDOW;
                _style_ex = WS_EX_OVERLAPPEDWINDOW;
                break;
            case WindowStyle::Fixed:
                _style = WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME;
                _style_ex = WS_EX_OVERLAPPEDWINDOW;
                break;
            case WindowStyle::Borderless:
                _style = WS_POPUP;
                _style_ex = 0;
                break;
            }
            _style_v = style;
        }
        void _impl_updatelayer(WindowLayer layer) {
            switch (layer) {
            default:
            case WindowLayer::Normal:
                _layer = HWND_NOTOPMOST;
                break;
            case WindowLayer::Top:
                _layer = HWND_TOP;
                break;
            case WindowLayer::Bottom:
                _layer = HWND_BOTTOM;
                break;
            case WindowLayer::Topmost:
                _layer = HWND_TOPMOST;
                break;
            }
            _layer_v = layer;
        }
        bool _impl_setsizeandmovetocenter(Vector2I size) {
            if (nullptr != _window) {
                HMONITOR monitor = ::MonitorFromWindow(_window, MONITOR_DEFAULTTONEAREST);
                MONITORINFO info;
                info.cbSize = sizeof(MONITORINFO);
                if (FALSE == ::GetMonitorInfoW(monitor, &info)) {
                    return false;
                }
                RECT rect;
                rect = { 0, 0, 0, 0 };
                if (FALSE == ::AdjustWindowRectEx(&rect, _style, FALSE, _style_ex)) {
                    return false;
                }
                int32_t ow = rect.right - rect.left;
                int32_t oh = rect.bottom - rect.top;
                rect = { 0, 0, size.x, size.y };
                if (FALSE == ::AdjustWindowRectEx(&rect, _style, FALSE, _style_ex)) {
                    return false;
                }
                int32_t ww = rect.right - rect.left;
                int32_t wh = rect.bottom - rect.top;
                int32_t x = 0, y = 0, w = 0, h = 0;
                if (_style_v == WindowStyle::Borderless) {
                    int32_t sw = info.rcMonitor.right - info.rcMonitor.left;
                    int32_t sh = info.rcMonitor.bottom - info.rcMonitor.top;
                    ww = std::min(ww, sw);
                    wh = std::min(wh, sh);
                    x = info.rcMonitor.left + (sw / 2) - (ww / 2);
                    y = info.rcMonitor.top + (sh / 2) - (wh / 2);
                    w = ww;
                    h = wh;
                }
                else {
                    int32_t sw = info.rcWork.right - info.rcWork.left;
                    int32_t sh = info.rcWork.bottom - info.rcWork.top;
                    ww = std::min(ww, sw);
                    wh = std::min(wh, sh);
                    x = info.rcWork.left + (sw / 2) - (ww / 2);
                    y = info.rcWork.top + (sh / 2) - (wh / 2);
                    w = ww;
                    h = wh;
                }
                if (FALSE == ::SetWindowPos(_window, _layer, x, y, w, h, SWP_SHOWWINDOW | SWP_FRAMECHANGED)) {
                    return false;
                }
                _size.x = w - ow;
                _size.y = h - oh;
                return true;
            }
            return false;
        }
    public:
        bool setTitle(const char* title) {
            std::wstring buffer;
            if (utf8_utf16(title, buffer)) {
                return setTitleW(buffer.c_str());
            }
            return false;
        }
        const char* getTitle() {
            return _title.c_str();
        }
        bool setStyle(WindowStyle style) {
            _impl_updatestyle(style);
            if (nullptr != _window) {
                bool failed = false;
                ::SetLastError(0);
                ::SetWindowLongPtrW(_window, GWL_STYLE, _style);
                failed |= (0 != ::GetLastError());
                ::SetLastError(0);
                ::SetWindowLongPtrW(_window, GWL_EXSTYLE, _style_ex);
                failed |= (0 != ::GetLastError());
                return _impl_setsizeandmovetocenter(_size) && !failed;
            }
            return false;
        }
        WindowStyle getStyle() {
            return _style_v;
        }
        bool setSize(Vector2I size) {
            return _impl_setsizeandmovetocenter(size);
        }
        Vector2I getSize() {
            Vector2I ret { 0, 0 };
            if (nullptr != _window) {
                RECT rect;
                if (FALSE != ::GetClientRect(_window, &rect)) {
                    ret.x = rect.right - rect.left;
                    ret.y = rect.bottom - rect.top;
                }
            }
            return ret;
        }
        Vector2I getActualSize() {
            Vector2I ret { 0, 0 };
            if (nullptr != _window) {
                RECT rect;
                if (FALSE != ::GetWindowRect(_window, &rect)) {
                    ret.x = rect.right - rect.left;
                    ret.y = rect.bottom - rect.top;
                }
            }
            return ret;
        }
        bool setLayer(WindowLayer layer) {
            _impl_updatelayer(layer);
            return _impl_setsizeandmovetocenter(_size);
        }
        WindowLayer getLayer() {
            return _layer_v;
        }
        bool setCentered() {
            return _impl_setsizeandmovetocenter(_size);
        }
        
        bool setPointerHide(bool hide) {
            return false;
        }
        bool getPointerHide() {
            return false;
        }
        bool setPointerPosition(Vector2I position) {
            return false;
        }
        Vector2I getPointerPosition() {
            return _input_pointer_position;
        }
        int32_t getPointerWheel() {
            return _input_pointer_wheel;
        }
        bool getPointerBotton(PointerBotton botton) {
            return _input_botton[(size_t)botton];
        }
        bool getKeyboardBotton(KeyboardBotton botton) {
            return _input_botton[(size_t)botton];
        }
        
        void setCallback(WindowCallback* callback) {
            _callback = callback;
        }
        void setWindowClose(bool v) {
            _exit = v;
        }
        bool shouldWindowClose() {
            return _exit;
        }
        bool dispatchMessage() {
            MSG msg;
            ZeroMemory(&msg, sizeof(MSG));
            while (::PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
                if (msg.message == WM_QUIT) {
                    setWindowClose(true);
                }
            }
            return !shouldWindowClose();
        }
        
        bool open(const char* title, Vector2I size, WindowStyle style, WindowLayer layer) {
            std::wstring buffer;
            if (utf8_utf16(title, buffer)) {
                return open(buffer.c_str(), size, style, layer);
            }
            return false;
        }
        void close() {
            if (_window != nullptr) {
                if (0 != _timer) {
                    ::KillTimer(_window, _timer);
                    _timer = 0;
                }
                ::DestroyWindow(_window);
                _window = nullptr;
            }
            if (_atom != 0) {
                ::UnregisterClassW(_classname.data(), _instance);
                _atom = 0;
            }
            _instance = nullptr;
            _layer_v = WindowLayer::Invalid;
            _layer = nullptr;
            _style_v = WindowStyle::Invalid;
            _style = 0;
            _style_ex = 0;
            _classname.fill(L'\0');
            _title.clear();
            _wtitle.clear();
            _size = { 0, 0 };
            _exit = false;
            _input_botton.fill(false);
            _input_pointer_position = { 0, 0 };
            _input_pointer_wheel = 0;
        }
        
        bool open(const wchar_t* title, Vector2I size, WindowStyle style, WindowLayer layer) {
            if (FALSE == ::GetModuleHandleExW(0, nullptr, &_instance)) {
                return false;
            }
            if (0 > std::swprintf(_classname.data(), 63, L"slow::Window::%p", this)) {
                return false;
            }
            _wtitle = title;
            if (!utf16_utf8(_wtitle, _title)) {
                return false;
            }
            
            WNDCLASSEXW classdata = {
                sizeof(WNDCLASSEXW),                   // struct size
                CS_HREDRAW | CS_VREDRAW,               // class style
                &_window_callback,                     // message callback
                0,                                     // class extra
                0,                                     // window extra
                _instance,                             // module handle
                ::LoadIconW(0, IDI_APPLICATION),       // icon
                ::LoadCursorW(0, IDC_ARROW),           // cursor
                (HBRUSH)::GetStockObject(BLACK_BRUSH), // background
                nullptr,                               // menu name
                _classname.data(),                     // class name
                nullptr,                               // small icon
            };
            _atom = ::RegisterClassExW(&classdata);
            if (_atom == 0) {
                return false;
            }
            
            _impl_updatestyle(style);
            _layer_v = WindowLayer::Normal;
            _window = ::CreateWindowExW(
                _style_ex, _classname.data(), _wtitle.c_str(), _style,
                0, 0, 640, 480,
                /*parent*/ nullptr, /*menu*/ nullptr, _instance, nullptr
            );
            if (_window == nullptr) {
                return false;
            }
            
            ::SetLastError(0);
            ::SetWindowLongPtrW(_window, GWLP_USERDATA, (LONG_PTR)this);
            if (0 != ::GetLastError()) {
                return false;
            }
            ::ShowWindow(_window, SW_SHOWDEFAULT);
            ::UpdateWindow(_window);
            
            _impl_updatelayer(layer);
            if (!_impl_setsizeandmovetocenter(size)) {
                return false;
            }
            
            return true;
        }
        bool setTitleW(const wchar_t* title) {
            if (nullptr != _window) {
                _wtitle = title;
                if (!utf16_utf8(_wtitle, _title)) {
                    return false;
                }
                if (FALSE != SetWindowTextW(_window, title)) {
                    return true;
                }
            }
            return false;
        }
        const wchar_t* getTitleW() {
            return _wtitle.c_str();
        }
        ptrdiff_t getNativeWindow() {
            return (ptrdiff_t)_window;
        }
    public:
        WindowImpl() :
            _instance(nullptr), _window(nullptr), _atom(0),
            _layer_v(WindowLayer::Invalid), _layer(nullptr),
            _style_v(WindowStyle::Invalid), _style(0), _style_ex(0),
            _size({ 0, 0 }), _sizemove(false),
            _exit(false),
            _input_pointer_position({ 0, 0 }), _input_pointer_wheel(0),
            _timer(0), _callback(nullptr)
        {
        }
        virtual ~WindowImpl() {
            close();
        }
    };
    
    bool Window::create(Window** output) {
        assert(output != nullptr);
        if (output != nullptr) {
            try {
                WindowImpl* p = new WindowImpl;
                *output = (Window*)p;
                return true;
            }
            catch (...) {}
        }
        return false;
    }
    Window* Window::create() {
        Window* output = nullptr;
        Window::create(&output);
        return output;
    }
};
