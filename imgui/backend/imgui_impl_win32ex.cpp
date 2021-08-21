// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

// Experimental features:
//  [X] Platform: It's safety to using this implement on a thread different to "Win32 GUI thread"
//      (or named "Win32 Window message thread").
//  [X] Platform: Dynamic link to XInput library (LoadLibrary and GetProcAddress), support multi XInput versions

// Warning:
//  1. This is a experimental implement and may have some bugs.
//  2. Mouse capture (Win32 API SetCapture, GetCapture and ReleaseCapture) may not working in some case
//     because it is asynchronous (execution order is not guaranteed).
//  3. Input latency may be large.
//  4. Support ImGuiBackendFlags_HasSetMousePos, but the reason same as (2), it may not working in some case.

// Change logs:
//  2020-11-01: First implement and add a example ("example_win32_workingthread_directx11").
//  2020-11-01: Fixed the wrong IME candidate list position bug.
//  2020-12-09: Rename backend to "imgui_impl_win32ex" and rename all methods to "ImGui_ImplWin32Ex"
//  2020-12-09: Rewrite the way exchanging messages, using Win32 API (SendMessage) to sync
//  2020-12-09: Rewrite the way linking to XInput library (LoadLibrary and GetProcAddress)
//  2020-12-09: Rename example to "example_win32ex_directx11"

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <cstdint>
#include <string>
#include "imgui.h"
#include "imgui_impl_win32ex.h"

// Windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// Allow compilation with old Windows SDK. MinGW doesn't have default _WIN32_WINNT/WINVER versions.
// WinUser.h
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
// Dbt.h
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif
// windowsx.h
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

// Using XInput library for gamepad
#include <XInput.h>
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif
typedef DWORD (WINAPI *f_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI *f_XInputGetState)(DWORD, XINPUT_STATE*);

#define BAND_BOOL(x, flag) ((x & flag) == flag)

constexpr UINT MSG_NONE             = IMGUI_IMPL_WIN32EX_WM_USER;
constexpr UINT MSG_MOUSE_CAPTURE    = IMGUI_IMPL_WIN32EX_WM_USER + 1;
constexpr UINT MSG_SET_MOUSE_POS    = IMGUI_IMPL_WIN32EX_WM_USER + 2;
constexpr UINT MSG_SET_MOUSE_CURSOR = IMGUI_IMPL_WIN32EX_WM_USER + 3;
constexpr UINT MSG_SET_IME_POS      = IMGUI_IMPL_WIN32EX_WM_USER + 4;
constexpr UINT MSG_EXCHANGE_DATA    = IMGUI_IMPL_WIN32EX_WM_USER + 5;

constexpr WPARAM MSG_MOUSE_CAPTURE_SET     = 1;
constexpr WPARAM MSG_MOUSE_CAPTURE_RELEASE = 2;

template<typename T, size_t N>
class CircularQueue
{
private:
    size_t _count;
    int _back;
    int _front;
    T* _data;
public:
    // design: the front is the earliest data, the back is the latest data
    // ...[...][...][back][front][...][...]...
    T& back()
    {
        return _data[_back % N];
    }
    T& front()
    {
        return _data[(_front - 1) % N];
    }
    bool push_back(const T& v)
    {
        if (_count < N)
        {
            _back -= 1;
            _data[_back % N] = v;
            _count += 1;
            return true;
        }
        return false;
    };
    bool pop_back()
    {
        if (_count > 0);
        {
            _back += 1;
            _count -= 1;
            return true;
        }
        return false;
    };
    bool pop_back(T& v)
    {
        if (_count > 0);
        {
            v = _data[_back % N];
            _back += 1;
            _count -= 1;
            return true;
        }
        return false;
    };
    bool push_front(const T& v)
    {
        if (_count < N)
        {
            _data[_front % N] = v;
            _front += 1;
            _count += 1;
            return true;
        }
        return false;
    };
    bool pop_front()
    {
        if (_count > 0)
        {
            _front -= 1;
            _count -= 1;
            return true;
        }
        return false;
    };
    bool pop_front(T& v)
    {
        if (_count > 0)
        {
            _front -= 1;
            v = _data[_front % N];
            _count -= 1;
            return true;
        }
        return false;
    };
    bool empty()
    {
        return (_count == 0);
    }
    size_t size()
    {
        return _count;
    }
    size_t max_size()
    {
        return N;
    }
    void clear()
    {
        _count = 0;
        _back = 0;
        _front = 0;
    }
public:
    T& operator[](size_t idx)
    {
        return _data[(_back + idx) % N];
    }
    void swap(CircularQueue& right)
    {
        std::swap(_count, right._count);
        std::swap(_back, right._back);
        std::swap(_front, right._front);
        std::swap(_data, right._data);
    }
public:
    CircularQueue() : _count(0), _back(0), _front(0), _data(nullptr) {
        _data = new T[N];
    }
    ~CircularQueue() {
        delete[] _data;
        _data = nullptr;
    }
};

struct Win32Message
{
    UINT   uMsg;
    WPARAM wParam;
    LPARAM lParam;
};

using Win32Messages = CircularQueue<Win32Message, 64>; // size might be too small?

struct ImGui_ImplWin32Data
{
    ImGui_ImplWin32ExFlags flags;
    
    HWND window;
    RECT windowRect;
    bool isWindowFocus;
    LPCWSTR cursorName;
    LARGE_INTEGER timeFrequency;
    LARGE_INTEGER lastTime;
    LARGE_INTEGER lastExchangeTime;
    
    HMODULE xinput;
    DWORD (WINAPI *XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
    DWORD (WINAPI *XInputGetState)(DWORD, XINPUT_STATE*);
    bool haveGamepad;
    bool wantUpdateGamepad;
    
    Win32Messages messages;
    
    bool updateCursor;
    ImGuiMouseCursor lastCursor;
    void (*lastSetIMEPosFn)(int x, int y);
    
    void clear()
    {
        flags = 0;
        
        window = NULL;
        windowRect = { 0, 0, 0, 0 };
        isWindowFocus = true; // Q: why default to "true"? A: because we don't known the initial state
        cursorName = NULL;
        timeFrequency.QuadPart = 0;
        lastTime.QuadPart = 0;
        
        xinput = NULL;
        XInputGetCapabilities = NULL;
        XInputGetState = NULL;
        haveGamepad = false;
        wantUpdateGamepad = true; // yes we want!
        
        messages.clear();
        
        updateCursor = false;
        lastCursor = ImGuiMouseCursor_COUNT;
        lastSetIMEPosFn = nullptr;
    }
};

static ImGui_ImplWin32Data g_tData;

bool ImGui_ImplWin32Ex_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (g_tData.lastCursor != mouse_cursor)
    {
        g_tData.lastCursor = mouse_cursor;
        
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        {
            g_tData.updateCursor = false;
            return false;
        }
        else
        {
            g_tData.updateCursor = true;
        }
        
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            g_tData.cursorName = NULL;
            PostMessageW(g_tData.window, MSG_SET_MOUSE_CURSOR, 0, 0);
        }
        else
        {
            // Show OS mouse cursor
            g_tData.cursorName = IDC_ARROW;
            switch (imgui_cursor)
            {
            case ImGuiMouseCursor_Arrow:        g_tData.cursorName = IDC_ARROW;       break;
            case ImGuiMouseCursor_TextInput:    g_tData.cursorName = IDC_IBEAM;       break;
            case ImGuiMouseCursor_ResizeAll:    g_tData.cursorName = IDC_SIZEALL;     break;
            case ImGuiMouseCursor_ResizeEW:     g_tData.cursorName = IDC_SIZEWE;      break;
            case ImGuiMouseCursor_ResizeNS:     g_tData.cursorName = IDC_SIZENS;      break;
            case ImGuiMouseCursor_ResizeNESW:   g_tData.cursorName = IDC_SIZENESW;    break;
            case ImGuiMouseCursor_ResizeNWSE:   g_tData.cursorName = IDC_SIZENWSE;    break;
            case ImGuiMouseCursor_Hand:         g_tData.cursorName = IDC_HAND;        break;
            case ImGuiMouseCursor_NotAllowed:   g_tData.cursorName = IDC_NO;          break;
            }
            PostMessageW(g_tData.window, MSG_SET_MOUSE_CURSOR, 0, 0);
        }
        
        return true;
    }
    
    return true;
}
void ImGui_ImplWin32Ex_UpdateMousePos()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        PostMessageW(g_tData.window, MSG_SET_MOUSE_POS, (WPARAM)(LONG)(io.MousePos.x), (LPARAM)(LONG)(io.MousePos.y));
    }
}
void ImGui_ImplWin32Ex_UpdateIMEPos(int x, int y)
{
    PostMessageW(g_tData.window, MSG_SET_IME_POS, (WPARAM)(LONG)x, (LPARAM)(LONG)y);
}
void ImGui_ImplWin32Ex_ProcessMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    Win32Message msg = { 0, 0, 0 };
    
    auto resetImGuiInput = [&]() -> void
    {
        io.AddFocusEvent(false);
        
        std::memset(&io.MouseDown, 0, sizeof(io.MouseDown));
        io.MouseWheel = 0;
        io.MouseWheelH = 0;
        io.MousePos = ImVec2(0.0f, 0.0f);
        PostMessageW(g_tData.window, MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_RELEASE, 0); // cancel capture
    };
    
    auto updateMousePosition = [&]() -> void
    {
        io.MousePos = ImVec2((float)GET_X_LPARAM(msg.lParam), (float)GET_Y_LPARAM(msg.lParam));
    };
    
    auto updateKeyboardKey = [&](bool down) -> void
    {
        // https://github.com/Microsoft/DirectXTK
        WPARAM vk = msg.wParam;
        switch (vk)
        {
        case VK_SHIFT:
            vk = (WPARAM)MapVirtualKeyW((msg.lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            if (!down)
            {
                // Workaround to ensure left vs. right shift get cleared when both were pressed at same time
                io.KeysDown[VK_LSHIFT] = down;
                io.KeysDown[VK_RSHIFT] = down;
            }
            io.KeyShift = down;
            break;
        case VK_CONTROL:
            vk = (msg.lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
            io.KeyCtrl = down;
            break;
        case VK_MENU:
            vk = (msg.lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
            io.KeyAlt = down;
            break;
        }
        if (vk < 256)
            io.KeysDown[vk] = down;
    };
    
    auto processInputEvent = [&]() -> bool
    {
        switch (msg.uMsg)
        {
        // mouse
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
            {
                int button = 0;
                if      (msg.uMsg == WM_LBUTTONDOWN || msg.uMsg == WM_LBUTTONDBLCLK)
                    button = 0;
                else if (msg.uMsg == WM_RBUTTONDOWN || msg.uMsg == WM_RBUTTONDBLCLK)
                    button = 1;
                else if (msg.uMsg == WM_MBUTTONDOWN || msg.uMsg == WM_MBUTTONDBLCLK)
                    button = 2;
                else if (msg.uMsg == WM_XBUTTONDOWN || msg.uMsg == WM_XBUTTONDBLCLK)
                    button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4;
                if (!ImGui::IsAnyMouseDown())
                {
                    PostMessageW(g_tData.window, MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_SET, 0);
                }
                io.MouseDown[button] = true;
                updateMousePosition();
            }
            return true;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
            {
                int button = 0;
                if      (msg.uMsg == WM_LBUTTONUP)
                    button = 0;
                else if (msg.uMsg == WM_RBUTTONUP)
                    button = 1;
                else if (msg.uMsg == WM_MBUTTONUP)
                    button = 2;
                else if (msg.uMsg == WM_XBUTTONUP)
                    button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4;
                io.MouseDown[button] = false;
                if (!ImGui::IsAnyMouseDown())
                {
                    PostMessageW(g_tData.window, MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_RELEASE, 0);
                }
                updateMousePosition();
            }
            return true;
        case WM_MOUSEMOVE:
        case WM_MOUSEHOVER:
            updateMousePosition();
            return true;
        case WM_MOUSEWHEEL:
            io.MouseWheel  += (float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA;
            return true;
        case WM_MOUSEHWHEEL:
            io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA;
            return true;
        
        // keyboard
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            updateKeyboardKey(true);
            return true;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            updateKeyboardKey(false);
            return true;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            io.AddFocusEvent(msg.uMsg == WM_SETFOCUS);
            return true;
        case WM_CHAR:
            if (msg.wParam > 0 && msg.wParam < 0x10000)
            {
                io.AddInputCharacterUTF16((ImWchar16)msg.wParam);
            }
            return true;
        }
        
        return false;
    };
    
    auto processOtherEvent = [&]() -> bool
    {
        switch (msg.uMsg)
        {
        case WM_ACTIVATEAPP:
            switch(msg.wParam)
            {
            case TRUE:
                g_tData.isWindowFocus = true;
                break;
            case FALSE:
                g_tData.isWindowFocus = false;
                break;
            }
            if (BAND_BOOL(g_tData.flags, ImGui_ImplWin32ExFlags_ResetInputWhenWindowFocusLose))
            {
                resetImGuiInput();
            }
            return true;
        case WM_SIZE:
            io.DisplaySize = ImVec2((float)(LOWORD(msg.lParam)), (float)(HIWORD(msg.lParam)));
            return true;
        case WM_DEVICECHANGE:
            if ((UINT)msg.wParam == DBT_DEVNODES_CHANGED)
            {
                g_tData.wantUpdateGamepad = true;
            }
            return true;
        }
        
        return false;
    };
    
    while (g_tData.messages.pop_front(msg))
    {
        if (g_tData.isWindowFocus || BAND_BOOL(g_tData.flags, ImGui_ImplWin32ExFlags_ReceiveInputInBackground))
        {
            if (!processInputEvent())
            {
                processOtherEvent();
            }
        }
        else
        {
            processOtherEvent();
        }
    }
}
void ImGui_ImplWin32Ex_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;
    
    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
    if (g_tData.wantUpdateGamepad)
    {
        g_tData.wantUpdateGamepad = false;
        if (g_tData.XInputGetCapabilities)
        {
            XINPUT_CAPABILITIES caps;
            g_tData.haveGamepad = (g_tData.XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS);
        }
        else
        {
            g_tData.haveGamepad = false;
        }
    }
    
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (g_tData.haveGamepad && g_tData.XInputGetState)
    {
        XINPUT_STATE xinput_state;
        if (g_tData.XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
        {
            const XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;
            io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
            
            #define MAP_BUTTON(NAV_NO, BUTTON_ENUM)     { io.NavInputs[NAV_NO] = (gamepad.wButtons & BUTTON_ENUM) ? 1.0f : 0.0f; }
            #define MAP_ANALOG(NAV_NO, VALUE, V0, V1)   { float vn = (float)(VALUE - V0) / (float)(V1 - V0); if (vn > 1.0f) vn = 1.0f; if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) io.NavInputs[NAV_NO] = vn; }
            MAP_BUTTON(ImGuiNavInput_Activate,      XINPUT_GAMEPAD_A);              // Cross / A
            MAP_BUTTON(ImGuiNavInput_Cancel,        XINPUT_GAMEPAD_B);              // Circle / B
            MAP_BUTTON(ImGuiNavInput_Menu,          XINPUT_GAMEPAD_X);              // Square / X
            MAP_BUTTON(ImGuiNavInput_Input,         XINPUT_GAMEPAD_Y);              // Triangle / Y
            MAP_BUTTON(ImGuiNavInput_DpadLeft,      XINPUT_GAMEPAD_DPAD_LEFT);      // D-Pad Left
            MAP_BUTTON(ImGuiNavInput_DpadRight,     XINPUT_GAMEPAD_DPAD_RIGHT);     // D-Pad Right
            MAP_BUTTON(ImGuiNavInput_DpadUp,        XINPUT_GAMEPAD_DPAD_UP);        // D-Pad Up
            MAP_BUTTON(ImGuiNavInput_DpadDown,      XINPUT_GAMEPAD_DPAD_DOWN);      // D-Pad Down
            MAP_BUTTON(ImGuiNavInput_FocusPrev,     XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
            MAP_BUTTON(ImGuiNavInput_FocusNext,     XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
            MAP_BUTTON(ImGuiNavInput_TweakSlow,     XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
            MAP_BUTTON(ImGuiNavInput_TweakFast,     XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
            MAP_ANALOG(ImGuiNavInput_LStickLeft,    gamepad.sThumbLX,  -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
            MAP_ANALOG(ImGuiNavInput_LStickRight,   gamepad.sThumbLX,  +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
            MAP_ANALOG(ImGuiNavInput_LStickUp,      gamepad.sThumbLY,  +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
            MAP_ANALOG(ImGuiNavInput_LStickDown,    gamepad.sThumbLY,  -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32767);
            #undef MAP_BUTTON
            #undef MAP_ANALOG
        }
    }
}

bool ImGui_ImplWin32Ex_Init(void* window, ImGui_ImplWin32ExFlags flags)
{
    g_tData.clear();
    g_tData.flags = flags;
    g_tData.window = (HWND)window;
    if (FALSE == QueryPerformanceFrequency(&g_tData.timeFrequency))
        return false;
    if (FALSE == QueryPerformanceCounter(&g_tData.lastTime))
        return false;
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup backend capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32ex";
    
    // Setup backend IME support
    io.ImeWindowHandle = (HWND)window;
    g_tData.lastSetIMEPosFn = io.ImeSetInputScreenPosFn;        // backup
    io.ImeSetInputScreenPosFn = &ImGui_ImplWin32Ex_UpdateIMEPos;
    
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
    
    // Gamepad
    const wchar_t* xinput_dll_name[] = {
        L"xinput1_4.dll",
        L"xinput1_3.dll",
        L"xinput9_1_0.dll",
    };
    for (size_t idx = 0; idx < 3; idx += 1)
    {
        HMODULE dll = LoadLibraryW(xinput_dll_name[idx]);
        if (dll != NULL)
        {
            g_tData.xinput = dll;
            g_tData.XInputGetCapabilities = (f_XInputGetCapabilities)GetProcAddress(dll, "XInputGetCapabilities");
            g_tData.XInputGetState = (f_XInputGetState)GetProcAddress(dll, "XInputGetState");
            break;
        }
    }
    
    return true;
}
void ImGui_ImplWin32Ex_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup backend IME support
    io.ImeWindowHandle = NULL;
    io.ImeSetInputScreenPosFn = g_tData.lastSetIMEPosFn;    // restore
    g_tData.lastSetIMEPosFn = NULL;
    
    // Gamepad
    if (g_tData.xinput != NULL)
        FreeLibrary(g_tData.xinput);
    
    g_tData.clear();
}
void ImGui_ImplWin32Ex_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");
    
    // Setup time step
    LARGE_INTEGER current_time;
    if (FALSE != QueryPerformanceCounter(&current_time))
    {
        io.DeltaTime = (float)((double)(current_time.QuadPart - g_tData.lastTime.QuadPart) / (double)(g_tData.timeFrequency.QuadPart));
        g_tData.lastTime = current_time;
        g_tData.lastExchangeTime = current_time;
    }
    
    // Exchange data
    SendMessageW(g_tData.window, MSG_EXCHANGE_DATA, 0, 0);
    
    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)(g_tData.windowRect.right - g_tData.windowRect.left), (float)(g_tData.windowRect.bottom - g_tData.windowRect.top));
    
    // Now process all messages
    io.KeySuper = false; // "Win" key
    ImGui_ImplWin32Ex_ProcessMessage();
    
    // Set mouse postion
    ImGui_ImplWin32Ex_UpdateMousePos();
    
    // Update OS mouse cursor with the cursor requested by imgui
    ImGui_ImplWin32Ex_UpdateMouseCursor();
    
    // Update Gamepad
    ImGui_ImplWin32Ex_UpdateGamepads();
}

IMGUI_IMPL_API LRESULT ImGui_ImplWin32Ex_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Win32Messages g_tMessage;
    
    if (g_tData.window == NULL || hWnd != g_tData.window)
        return 0; // we only need to process messages belong to our target window
    
    switch (uMsg)
    {
    // messages want to process by imgui
    
    case WM_ACTIVATEAPP:
    
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_MOUSEMOVE:
    case WM_MOUSEHOVER:
    
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_CHAR:
    
    case WM_SIZE:
    case WM_DEVICECHANGE:
        {
            if (g_tMessage.size() >= g_tMessage.max_size())
            {
                g_tMessage.pop_front();
            }
            g_tMessage.push_back({ uMsg, wParam, lParam });
        }
        return 0;
    
    // messages we recive from imgui (working thread)
    
    case MSG_MOUSE_CAPTURE:
        switch(wParam)
        {
        case MSG_MOUSE_CAPTURE_SET:
            if (GetCapture() == NULL)
            {
                SetCapture(hWnd);
            }
            break;
        case MSG_MOUSE_CAPTURE_RELEASE:
            if (GetCapture() == hWnd)
            {
                ReleaseCapture();
            }
            break;
        }
        return 0;
    case MSG_SET_MOUSE_POS:
        {
            POINT pos;
            pos.x = (LONG)wParam;
            pos.y = (LONG)lParam;
            ClientToScreen(hWnd, &pos);
            SetCursorPos(pos.x, pos.y);
        }
        return 0;
    case MSG_SET_MOUSE_CURSOR:
        {
            const LPCWSTR cn = g_tData.cursorName;
            if (cn != NULL)
                SetCursor(LoadCursorW(NULL, cn));
            else
                SetCursor(NULL);
        }
        return 1; // tell GUI thread do not continue to pass this message
    case MSG_SET_IME_POS:
        if (HIMC himc = ImmGetContext(hWnd))
        {
            POINT pos;
            pos.x = (LONG)wParam;
            pos.y = (LONG)lParam;
            COMPOSITIONFORM cf;
            cf.ptCurrentPos = pos;
            cf.dwStyle = CFS_FORCE_POSITION;
            ImmSetCompositionWindow(himc, &cf);
            ImmReleaseContext(hWnd, himc);
        }
        return 0;
    case MSG_EXCHANGE_DATA:
        {
            GetClientRect(g_tData.window, &g_tData.windowRect);
            
            #if 0
            const auto cnt = g_tMessage.size();
            #endif
            g_tData.messages.swap(g_tMessage);
            g_tMessage.clear();
            
            #if 0
            LARGE_INTEGER current_time;
            QueryPerformanceCounter(&current_time);
            const double dt = (double)(current_time.QuadPart - g_tData.lastExchangeTime.QuadPart) / (double)(g_tData.timeFrequency.QuadPart);
            char cbuf[64] = { 0 };
            snprintf(cbuf, 64, "exchange time %.3fms, exchange %u messages\n", dt * 1000.0, cnt);
            OutputDebugStringA(cbuf);
            #endif
        }
        return 0;
    
    // other messages
    
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && g_tData.updateCursor)
        {
            const LPCWSTR cn = g_tData.cursorName;
            if (cn != NULL)
                SetCursor(LoadCursorW(NULL, cn));
            else
                SetCursor(NULL);
            return 1; // tell GUI thread do not continue to pass this message
        }
        return 0;
    }
    
    return 0;
}
