// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: It's safety to using this implement on a thread different to "Win32 GUI thread"
//      (or named "Win32 Window message thread").
//  [X] Platform: Dynamic link to XInput library (LoadLibrary and GetProcAddress), support multi XInput versions

// Change logs:
//  2022-02-15: Update to Dear ImGui v1.87
//  2020-11-01: Fixed the wrong IME candidate list position bug.
//  2020-12-09: Rename backend to "imgui_impl_win32ex" and rename all methods to "ImGui_ImplWin32Ex"
//  2020-12-09: Rewrite the way exchanging messages, using Win32 API (SendMessage) to sync

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <cstdint>
#include <string>
#include "imgui.h"
#include "imgui_impl_win32ex.h"

// Windows.h
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
// Using XInput for gamepad (will load DLL dynamically)
#include <Xinput.h>
typedef DWORD(WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD(WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);
// Dbt.h
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif
// There is no distinct VK_xxx for keypad enter, instead it is VK_RETURN + KF_EXTENDED, we assign it an arbitrary value to make code more readable (VK_ codes go up to 255)
#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

constexpr UINT MSG_NONE             = IMGUI_IMPL_WIN32EX_WM_USER;
constexpr UINT MSG_MOUSE_CAPTURE    = IMGUI_IMPL_WIN32EX_WM_USER + 1;
constexpr UINT MSG_SET_MOUSE_POS    = IMGUI_IMPL_WIN32EX_WM_USER + 2;
constexpr UINT MSG_SET_MOUSE_CURSOR = IMGUI_IMPL_WIN32EX_WM_USER + 3;
constexpr UINT MSG_SET_IME_POS      = IMGUI_IMPL_WIN32EX_WM_USER + 4;

constexpr WPARAM MSG_MOUSE_CAPTURE_SET     = 1;
constexpr WPARAM MSG_MOUSE_CAPTURE_RELEASE = 2;

struct Win32Message
{
    HWND   hWnd;
    UINT   uMsg;
    WPARAM wParam;
    LPARAM lParam;
};
struct Win32MessageQueueSafe
{
    size_t const size = 256;
    Win32Message data[256] = {};
    size_t writer_index = 0;
    size_t reader_index = 0;
    HANDLE semaphore_space = NULL;
    HANDLE semaphore_data = NULL;

    bool write(Win32Message const& v)
    {
        if (::WaitForSingleObject(semaphore_space, 0) == WAIT_OBJECT_0)
        {
            data[writer_index] = v;
            writer_index = (writer_index + 1) % size;
            ::ReleaseSemaphore(semaphore_data, 1, NULL);
            return true;
        }
        return false;
    }
    bool read(Win32Message& v)
    {
        if (::WaitForSingleObject(semaphore_data, 0) == WAIT_OBJECT_0)
        {
            v = data[reader_index];
            reader_index = (reader_index + 1) % size;
            ::ReleaseSemaphore(semaphore_space, 1, NULL);
            return true;
        }
        return false;
    }

    Win32MessageQueueSafe()
    {
        LONG const value = (LONG)size;
        writer_index = 0;
        reader_index = 0;
        semaphore_space = ::CreateSemaphoreExW(NULL, value, value, NULL, 0, SEMAPHORE_ALL_ACCESS);
        semaphore_data = ::CreateSemaphoreExW(NULL, 0, value, NULL, 0, SEMAPHORE_ALL_ACCESS);
        if (semaphore_space == NULL || semaphore_data == NULL)
            throw;
    }
    ~Win32MessageQueueSafe()
    {
        writer_index = 0;
        reader_index = 0;
        if (semaphore_space) ::CloseHandle(semaphore_space); semaphore_space = NULL;
        if (semaphore_data) ::CloseHandle(semaphore_data); semaphore_data = NULL;
    }
};
struct Win32MessageQueue
{
    size_t const size = 0x100;
    Win32Message data[0x100] {};
    volatile int cave[0x100] {};
    size_t writer_index { 0 };
    size_t reader_index { 0 };
    
    bool write(Win32Message const& v)
    {
        if (!cave[writer_index])
        {
            cave[writer_index] = 1;
            data[writer_index] = v;
            writer_index = (writer_index + 1) % 0x100;
            return true;
        }
        return false;
    }
    bool read(Win32Message& v)
    {
        if (cave[reader_index])
        {
            v = data[reader_index];
            cave[reader_index] = 0;
            reader_index = (reader_index + 1) % 0x100;
            return true;
        }
        return false;
    }
};

struct ImGui_ImplWin32Ex_Data
{
    HWND                        hWnd                    = NULL;
    HWND                        MouseHwnd               = NULL;
    bool                        MouseTracked            = false;
    int                         MouseButtonsDown        = 0;
    LARGE_INTEGER               Time                    = {};
    LARGE_INTEGER               TicksPerSecond          = {};
    ImGuiMouseCursor            LastMouseCursor         = ImGuiMouseCursor_Arrow;
    bool                        HasGamepad              = false;
    bool                        WantUpdateHasGamepad    = true; // want checkout first

    HMODULE                     XInputDLL               = NULL;
    PFN_XInputGetCapabilities   XInputGetCapabilities   = NULL;
    PFN_XInputGetState          XInputGetState          = NULL;

    Win32MessageQueue           MessageQueue;
};

static ImGui_ImplWin32Ex_Data* ImGui_ImplWin32Ex_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplWin32Ex_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;
}

static bool IsVkDown(int vk)
{
    return (::GetKeyState(vk) & 0x8000) != 0;
}
static void ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(key, down);
    io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
    IM_UNUSED(native_scancode);
}
static ImGuiKey ImGui_ImplWin32Ex_VirtualKeyToImGuiKey(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_TAB: return ImGuiKey_Tab;
    case VK_LEFT: return ImGuiKey_LeftArrow;
    case VK_RIGHT: return ImGuiKey_RightArrow;
    case VK_UP: return ImGuiKey_UpArrow;
    case VK_DOWN: return ImGuiKey_DownArrow;
    case VK_PRIOR: return ImGuiKey_PageUp;
    case VK_NEXT: return ImGuiKey_PageDown;
    case VK_HOME: return ImGuiKey_Home;
    case VK_END: return ImGuiKey_End;
    case VK_INSERT: return ImGuiKey_Insert;
    case VK_DELETE: return ImGuiKey_Delete;
    case VK_BACK: return ImGuiKey_Backspace;
    case VK_SPACE: return ImGuiKey_Space;
    case VK_RETURN: return ImGuiKey_Enter;
    case VK_ESCAPE: return ImGuiKey_Escape;
    case VK_OEM_7: return ImGuiKey_Apostrophe;
    case VK_OEM_COMMA: return ImGuiKey_Comma;
    case VK_OEM_MINUS: return ImGuiKey_Minus;
    case VK_OEM_PERIOD: return ImGuiKey_Period;
    case VK_OEM_2: return ImGuiKey_Slash;
    case VK_OEM_1: return ImGuiKey_Semicolon;
    case VK_OEM_PLUS: return ImGuiKey_Equal;
    case VK_OEM_4: return ImGuiKey_LeftBracket;
    case VK_OEM_5: return ImGuiKey_Backslash;
    case VK_OEM_6: return ImGuiKey_RightBracket;
    case VK_OEM_3: return ImGuiKey_GraveAccent;
    case VK_CAPITAL: return ImGuiKey_CapsLock;
    case VK_SCROLL: return ImGuiKey_ScrollLock;
    case VK_NUMLOCK: return ImGuiKey_NumLock;
    case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
    case VK_PAUSE: return ImGuiKey_Pause;
    case VK_NUMPAD0: return ImGuiKey_Keypad0;
    case VK_NUMPAD1: return ImGuiKey_Keypad1;
    case VK_NUMPAD2: return ImGuiKey_Keypad2;
    case VK_NUMPAD3: return ImGuiKey_Keypad3;
    case VK_NUMPAD4: return ImGuiKey_Keypad4;
    case VK_NUMPAD5: return ImGuiKey_Keypad5;
    case VK_NUMPAD6: return ImGuiKey_Keypad6;
    case VK_NUMPAD7: return ImGuiKey_Keypad7;
    case VK_NUMPAD8: return ImGuiKey_Keypad8;
    case VK_NUMPAD9: return ImGuiKey_Keypad9;
    case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
    case VK_DIVIDE: return ImGuiKey_KeypadDivide;
    case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
    case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
    case VK_ADD: return ImGuiKey_KeypadAdd;
    case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
    case VK_LSHIFT: return ImGuiKey_LeftShift;
    case VK_LCONTROL: return ImGuiKey_LeftCtrl;
    case VK_LMENU: return ImGuiKey_LeftAlt;
    case VK_LWIN: return ImGuiKey_LeftSuper;
    case VK_RSHIFT: return ImGuiKey_RightShift;
    case VK_RCONTROL: return ImGuiKey_RightCtrl;
    case VK_RMENU: return ImGuiKey_RightAlt;
    case VK_RWIN: return ImGuiKey_RightSuper;
    case VK_APPS: return ImGuiKey_Menu;
    case '0': return ImGuiKey_0;
    case '1': return ImGuiKey_1;
    case '2': return ImGuiKey_2;
    case '3': return ImGuiKey_3;
    case '4': return ImGuiKey_4;
    case '5': return ImGuiKey_5;
    case '6': return ImGuiKey_6;
    case '7': return ImGuiKey_7;
    case '8': return ImGuiKey_8;
    case '9': return ImGuiKey_9;
    case 'A': return ImGuiKey_A;
    case 'B': return ImGuiKey_B;
    case 'C': return ImGuiKey_C;
    case 'D': return ImGuiKey_D;
    case 'E': return ImGuiKey_E;
    case 'F': return ImGuiKey_F;
    case 'G': return ImGuiKey_G;
    case 'H': return ImGuiKey_H;
    case 'I': return ImGuiKey_I;
    case 'J': return ImGuiKey_J;
    case 'K': return ImGuiKey_K;
    case 'L': return ImGuiKey_L;
    case 'M': return ImGuiKey_M;
    case 'N': return ImGuiKey_N;
    case 'O': return ImGuiKey_O;
    case 'P': return ImGuiKey_P;
    case 'Q': return ImGuiKey_Q;
    case 'R': return ImGuiKey_R;
    case 'S': return ImGuiKey_S;
    case 'T': return ImGuiKey_T;
    case 'U': return ImGuiKey_U;
    case 'V': return ImGuiKey_V;
    case 'W': return ImGuiKey_W;
    case 'X': return ImGuiKey_X;
    case 'Y': return ImGuiKey_Y;
    case 'Z': return ImGuiKey_Z;
    case VK_F1: return ImGuiKey_F1;
    case VK_F2: return ImGuiKey_F2;
    case VK_F3: return ImGuiKey_F3;
    case VK_F4: return ImGuiKey_F4;
    case VK_F5: return ImGuiKey_F5;
    case VK_F6: return ImGuiKey_F6;
    case VK_F7: return ImGuiKey_F7;
    case VK_F8: return ImGuiKey_F8;
    case VK_F9: return ImGuiKey_F9;
    case VK_F10: return ImGuiKey_F10;
    case VK_F11: return ImGuiKey_F11;
    case VK_F12: return ImGuiKey_F12;
    default: return ImGuiKey_None;
    }
}
static void ImGui_ImplWin32Ex_UpdateKeyModifiers()
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, IsVkDown(VK_CONTROL));
    io.AddKeyEvent(ImGuiMod_Shift, IsVkDown(VK_SHIFT));
    io.AddKeyEvent(ImGuiMod_Alt, IsVkDown(VK_MENU));
    io.AddKeyEvent(ImGuiMod_Super, IsVkDown(VK_APPS));
}

static void ImGui_ImplWin32Ex_ProcessKeyEventsWorkarounds()
{
    // Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !IsVkDown(VK_LSHIFT))
        ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_LeftShift, false, VK_LSHIFT);
    if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !IsVkDown(VK_RSHIFT))
        ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_RightShift, false, VK_RSHIFT);

    // Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
    if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !IsVkDown(VK_LWIN))
        ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_LeftSuper, false, VK_LWIN);
    if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !IsVkDown(VK_RWIN))
        ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_RightSuper, false, VK_RWIN);
}
static void ImGui_ImplWin32Ex_UpdateMouseData()
{
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(bd->hWnd != NULL);

    const bool is_app_focused = (::GetForegroundWindow() == bd->hWnd);
    if (is_app_focused)
    {
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if (io.WantSetMousePos)
        {
            ::PostMessageW(bd->hWnd, MSG_SET_MOUSE_POS, (WPARAM)(LONG)(io.MousePos.x), (LPARAM)(LONG)(io.MousePos.y));
        }

        // (Optional) Fallback to provide mouse position when focused (WM_MOUSEMOVE already provides this when hovered or captured)
        if (!io.WantSetMousePos && !bd->MouseTracked)
        {
            POINT pos = {};
            if (::GetCursorPos(&pos) && ::ScreenToClient(bd->hWnd, &pos))
            {
                io.AddMousePosEvent((float)pos.x, (float)pos.y);
            }
        }
    }
}
static bool ImGui_ImplWin32Ex_MapMouseCursor(LPWSTR* outValue)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        *outValue = NULL;
    }
    else
    {
        // Show OS mouse cursor
        LPWSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        *outValue = win32_cursor;
    }
    return true;
}
static bool ImGui_ImplWin32Ex_UpdateMouseCursor()
{
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    LPWSTR win32_cursor = NULL;
    if (ImGui_ImplWin32Ex_MapMouseCursor(&win32_cursor))
    {
        ::PostMessageW(bd->hWnd, MSG_SET_MOUSE_CURSOR, (WPARAM)win32_cursor, 0);
        return true;
    }
    return false;
}
static void ImGui_ImplWin32Ex_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
    if (bd->WantUpdateHasGamepad)
    {
        XINPUT_CAPABILITIES caps = {};
        bd->HasGamepad = bd->XInputGetCapabilities ? (bd->XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS) : false;
        bd->WantUpdateHasGamepad = false;
    }

    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    XINPUT_STATE xinput_state;
    XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;
    if (!bd->HasGamepad || bd->XInputGetState == NULL || bd->XInputGetState(0, &xinput_state) != ERROR_SUCCESS)
        return;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

#define IM_SATURATE(V)                      (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_ENUM)     { io.AddKeyEvent(KEY_NO, (gamepad.wButtons & BUTTON_ENUM) != 0); }
#define MAP_ANALOG(KEY_NO, VALUE, V0, V1)   { float vn = (float)(VALUE - V0) / (float)(V1 - V0); io.AddKeyAnalogEvent(KEY_NO, vn > 0.10f, IM_SATURATE(vn)); }
    MAP_BUTTON(ImGuiKey_GamepadStart, XINPUT_GAMEPAD_START);
    MAP_BUTTON(ImGuiKey_GamepadBack, XINPUT_GAMEPAD_BACK);
    MAP_BUTTON(ImGuiKey_GamepadFaceDown, XINPUT_GAMEPAD_A);
    MAP_BUTTON(ImGuiKey_GamepadFaceRight, XINPUT_GAMEPAD_B);
    MAP_BUTTON(ImGuiKey_GamepadFaceLeft, XINPUT_GAMEPAD_X);
    MAP_BUTTON(ImGuiKey_GamepadFaceUp, XINPUT_GAMEPAD_Y);
    MAP_BUTTON(ImGuiKey_GamepadDpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
    MAP_BUTTON(ImGuiKey_GamepadDpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
    MAP_BUTTON(ImGuiKey_GamepadDpadUp, XINPUT_GAMEPAD_DPAD_UP);
    MAP_BUTTON(ImGuiKey_GamepadDpadDown, XINPUT_GAMEPAD_DPAD_DOWN);
    MAP_BUTTON(ImGuiKey_GamepadL1, XINPUT_GAMEPAD_LEFT_SHOULDER);
    MAP_BUTTON(ImGuiKey_GamepadR1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
    MAP_ANALOG(ImGuiKey_GamepadL2, gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
    MAP_ANALOG(ImGuiKey_GamepadR2, gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
    MAP_BUTTON(ImGuiKey_GamepadL3, XINPUT_GAMEPAD_LEFT_THUMB);
    MAP_BUTTON(ImGuiKey_GamepadR3, XINPUT_GAMEPAD_RIGHT_THUMB);
    MAP_ANALOG(ImGuiKey_GamepadLStickLeft, gamepad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(ImGuiKey_GamepadLStickRight, gamepad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ImGuiKey_GamepadLStickUp, gamepad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ImGuiKey_GamepadLStickDown, gamepad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(ImGuiKey_GamepadRStickLeft, gamepad.sThumbRX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(ImGuiKey_GamepadRStickRight, gamepad.sThumbRX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ImGuiKey_GamepadRStickUp, gamepad.sThumbRY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ImGuiKey_GamepadRStickDown, gamepad.sThumbRY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
#undef MAP_BUTTON
#undef MAP_ANALOG
}
static void ImGui_ImplWin32Ex_UpdateIME(ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
    std::ignore = viewport; // We only have one window
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    ::PostMessageW(bd->hWnd, MSG_SET_IME_POS, (WPARAM)(LONG)data->InputPos.x, (LPARAM)(LONG)data->InputPos.y);
}
static void ImGui_ImplWin32Ex_ProcessMessage()
{
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    Win32Message msg = {};

    auto processInputEvent = [&]() -> bool
    {
        // mouse
        switch (msg.uMsg)
        {
        case WM_MOUSEMOVE:
            io.AddMousePosEvent((float)GET_X_LPARAM(msg.lParam), (float)GET_Y_LPARAM(msg.lParam));
            return true;
        case WM_MOUSELEAVE:
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            return true;
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
            {
                int button = 0;
                if (msg.uMsg == WM_LBUTTONDOWN || msg.uMsg == WM_LBUTTONDBLCLK) { button = 0; }
                if (msg.uMsg == WM_RBUTTONDOWN || msg.uMsg == WM_RBUTTONDBLCLK) { button = 1; }
                if (msg.uMsg == WM_MBUTTONDOWN || msg.uMsg == WM_MBUTTONDBLCLK) { button = 2; }
                if (msg.uMsg == WM_XBUTTONDOWN || msg.uMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4; }
                if (bd->MouseButtonsDown == 0)
                    ::PostMessageW(bd->hWnd, MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_SET, 0);
                bd->MouseButtonsDown |= 1 << button;
                io.AddMouseButtonEvent(button, true);
            }
            return true;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
            {
                int button = 0;
                if (msg.uMsg == WM_LBUTTONUP) { button = 0; }
                if (msg.uMsg == WM_RBUTTONUP) { button = 1; }
                if (msg.uMsg == WM_MBUTTONUP) { button = 2; }
                if (msg.uMsg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? 3 : 4; }
                bd->MouseButtonsDown &= ~(1 << button);
                if (bd->MouseButtonsDown == 0)
                    ::PostMessageW(bd->hWnd, MSG_MOUSE_CAPTURE, MSG_MOUSE_CAPTURE_RELEASE, 0);
                io.AddMouseButtonEvent(button, false);
            }
            return true;
        case WM_MOUSEWHEEL:
            io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA);
            return true;
        case WM_MOUSEHWHEEL:
            io.AddMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(msg.wParam) / (float)WHEEL_DELTA, 0.0f);
            return true;
        }

        // keyboard
        switch (msg.uMsg)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            {
                const bool is_key_down = (msg.uMsg == WM_KEYDOWN || msg.uMsg == WM_SYSKEYDOWN);
                if (msg.wParam < 256)
                {
                    // Submit modifiers
                    ImGui_ImplWin32Ex_UpdateKeyModifiers();

                    // Obtain virtual key code
                    // (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
                    int vk = (int)msg.wParam;
                    if ((msg.wParam == VK_RETURN) && (HIWORD(msg.lParam) & KF_EXTENDED))
                        vk = IM_VK_KEYPAD_ENTER;

                    // Submit key event
                    const ImGuiKey key = ImGui_ImplWin32Ex_VirtualKeyToImGuiKey(vk);
                    const int scancode = (int)LOBYTE(HIWORD(msg.lParam));
                    if (key != ImGuiKey_None)
                        ImGui_ImplWin32Ex_AddKeyEvent(key, is_key_down, vk, scancode);

                    // Submit individual left/right modifier events
                    if (vk == VK_SHIFT)
                    {
                        // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
                        if (IsVkDown(VK_LSHIFT) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
                        if (IsVkDown(VK_RSHIFT) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
                    }
                    else if (vk == VK_CONTROL)
                    {
                        if (IsVkDown(VK_LCONTROL) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
                        if (IsVkDown(VK_RCONTROL) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
                    }
                    else if (vk == VK_MENU)
                    {
                        if (IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
                        if (IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplWin32Ex_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
                    }
                }
            }
            return true;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            io.AddFocusEvent(msg.uMsg == WM_SETFOCUS);
            return true;
        case WM_CHAR:
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (msg.wParam > 0 && msg.wParam < 0x10000)
                io.AddInputCharacterUTF16((unsigned short)msg.wParam);
            return true;
        }

        return false;
    };

    auto processOtherEvent = [&]() -> bool
    {
        switch (msg.uMsg)
        {
        case WM_ACTIVATEAPP:
            io.AddFocusEvent(msg.wParam == TRUE);
            return true;
        case WM_SIZE:
            io.DisplaySize = ImVec2((float)(LOWORD(msg.lParam)), (float)(HIWORD(msg.lParam)));
            return true;
        case WM_DEVICECHANGE:
            if ((UINT)msg.wParam == DBT_DEVNODES_CHANGED)
                bd->WantUpdateHasGamepad = true;
            return true;
        }

        return false;
    };

    for (size_t i = 0; i < bd->MessageQueue.size; i += 1)
    {
        if (bd->MessageQueue.read(msg))
        {
            if (!processInputEvent())
            {
                processOtherEvent();
            }
        }
        else
        {
            break;
        }
    }
}

bool ImGui_ImplWin32Ex_Init(void* window)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    LARGE_INTEGER perf_frequency = {};
    LARGE_INTEGER perf_counter = {};
    if (!::QueryPerformanceFrequency(&perf_frequency))
        return false;
    if (!::QueryPerformanceCounter(&perf_counter))
        return false;
    
    // create data
    ImGui_ImplWin32Ex_Data* bd = IM_NEW(ImGui_ImplWin32Ex_Data)();
    io.BackendPlatformUserData = (void*)bd;

    // Setup backend capabilities flags
    io.BackendPlatformName = "imgui_impl_win32ex";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->hWnd = (HWND)window;
    bd->TicksPerSecond = perf_frequency;
    bd->Time = perf_counter;

    // Set platform dependent data in viewport
    ImGui::GetMainViewport()->PlatformHandleRaw = (void*)window;
    
    // Setup backend IME support
    io.SetPlatformImeDataFn = &ImGui_ImplWin32Ex_UpdateIME;
    
    // Gamepad
    wchar_t const* xinput_dll_names[] = {
        L"XInput1_4.dll",   // Windows 8+
        L"xinput1_3.dll",   // DirectX SDK
        L"XInput9_1_0.dll", // Windows Vista, Windows 7
        L"xinput1_2.dll",   // DirectX SDK
        L"xinput1_1.dll",   // DirectX SDK
    };
    for (auto name : xinput_dll_names)
    {
        if (HMODULE dll = ::LoadLibraryW(name))
        {
            bd->XInputDLL = dll;
            bd->XInputGetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
            bd->XInputGetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
            break;
        }
    }
    
    return true;
}
void ImGui_ImplWin32Ex_Shutdown()
{
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    // Unload XInput library
    if (bd->XInputDLL) ::FreeLibrary(bd->XInputDLL);

    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    IM_DELETE(bd);
}
void ImGui_ImplWin32Ex_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplWin32Ex_Init()?");

    // message
    ImGui_ImplWin32Ex_ProcessMessage();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect = { 0, 0, 0, 0 };
    ::GetClientRect(bd->hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    LARGE_INTEGER current_time = {};
    ::QueryPerformanceCounter(&current_time);
    io.DeltaTime = (float)(current_time.QuadPart - bd->Time.QuadPart) / bd->TicksPerSecond.QuadPart;
    bd->Time = current_time;

    // Update OS mouse position
    ImGui_ImplWin32Ex_UpdateMouseData();

    // Process workarounds for known Windows key handling issues
    ImGui_ImplWin32Ex_ProcessKeyEventsWorkarounds();

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (bd->LastMouseCursor != mouse_cursor)
    {
        bd->LastMouseCursor = mouse_cursor;
        ImGui_ImplWin32Ex_UpdateMouseCursor();
    }

    // Update game controllers (if enabled and available)
    ImGui_ImplWin32Ex_UpdateGamepads();
}

IMGUI_IMPL_API LRESULT ImGui_ImplWin32Ex_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!ImGui::GetCurrentContext())
        return 0;

    ImGui_ImplWin32Ex_Data* bd = ImGui_ImplWin32Ex_GetBackendData();
    if (!bd || bd->hWnd != hWnd)
        return 0;
    
    switch (uMsg)
    {
    // messages want to process by imgui
    
    case WM_MOUSEMOVE:
        // We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
        bd->MouseHwnd = hWnd;
        if (!bd->MouseTracked)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
            ::TrackMouseEvent(&tme);
            bd->MouseTracked = true;
        }
        bd->MessageQueue.write({ hWnd, uMsg, wParam, lParam });
        return 0;
    case WM_MOUSELEAVE:
        if (bd->MouseHwnd == hWnd)
            bd->MouseHwnd = NULL;
        bd->MouseTracked = false;
        bd->MessageQueue.write({ hWnd, uMsg, wParam, lParam });
        return 0;

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
    
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_CHAR:
    
    case WM_ACTIVATEAPP:
    case WM_SIZE:
    case WM_DEVICECHANGE:
        bd->MessageQueue.write({ hWnd, uMsg, wParam, lParam });
        return 0;
    
    // messages we recive from imgui (working thread)
    
    case MSG_MOUSE_CAPTURE:
        switch(wParam)
        {
        case MSG_MOUSE_CAPTURE_SET:
            if (::GetCapture() == NULL) ::SetCapture(hWnd); break;
        case MSG_MOUSE_CAPTURE_RELEASE:
            if (::GetCapture() == hWnd) ::ReleaseCapture(); break;
        }
        return 1; // tell GUI thread do not continue to pass this message
    case MSG_SET_MOUSE_POS:
        {
            POINT pos;
            pos.x = (LONG)wParam;
            pos.y = (LONG)lParam;
            ::ClientToScreen(hWnd, &pos);
            ::SetCursorPos(pos.x, pos.y);
        }
        return 1; // tell GUI thread do not continue to pass this message
    case MSG_SET_MOUSE_CURSOR:
        {
            if (wParam)
                ::SetCursor(::LoadCursorW(NULL, (LPCWSTR)wParam));
            else
                ::SetCursor(NULL);
        }
        return 1; // tell GUI thread do not continue to pass this message
    case MSG_SET_IME_POS:
        if (HIMC himc = ::ImmGetContext(hWnd))
        {
            POINT pos = { (LONG)wParam, (LONG)lParam };
            COMPOSITIONFORM cf = { CFS_FORCE_POSITION, pos, {} };
            ::ImmSetCompositionWindow(himc, &cf);
            ::ImmReleaseContext(hWnd, himc);
        }
        return 1; // tell GUI thread do not continue to pass this message
    
    // other messages
    
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            LPWSTR win32_cursor = NULL;
            if (ImGui_ImplWin32Ex_MapMouseCursor(&win32_cursor))
            {
                if (win32_cursor)
                    ::SetCursor(::LoadCursorW(NULL, win32_cursor));
                else
                    ::SetCursor(NULL);
                return 1; // tell GUI thread do not continue to pass this message
            }
        }
        return 0;
    }
    
    return 0;
}
