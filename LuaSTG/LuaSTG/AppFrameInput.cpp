#include "AppFrame.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Mouse.h"
#include "Platform/Keyboard.hpp"

namespace LuaSTGPlus
{
    static Platform::Keyboard g_Keyboard;
    static Platform::Keyboard::State g_KeyboardState;
}

static struct InputEventListener : public Core::Graphics::IWindowEventListener
{
    NativeWindowMessageResult onNativeWindowMessage(void* window, uint32_t message, uintptr_t arg1, intptr_t arg2)
    {
        switch (message)
        {
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            LuaSTGPlus::g_Keyboard.ProcessMessage((HWND)window, message, arg1, arg2);
            break;
        }

        switch (message)
        {
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
            DirectX::Mouse::ProcessMessage(message, arg1, arg2);
            break;
        }

        return {};
    }
} g_InputEventListener;

namespace LuaSTGPlus
{
    static std::unique_ptr<DirectX::Mouse> Mouse;
    static DirectX::Mouse::State MouseState;

    void AppFrame::OpenInput()
    {
        g_Keyboard.Reset();
        Mouse = std::make_unique<DirectX::Mouse>();
        ZeroMemory(&MouseState, sizeof(MouseState));
        m_pAppModel->getWindow()->addEventListener(&g_InputEventListener);
        Mouse->SetWindow((HWND)m_pAppModel->getWindow()->getNativeHandle());
    }
    void AppFrame::CloseInput()
    {
        m_pAppModel->getWindow()->removeEventListener(&g_InputEventListener);
        Mouse = nullptr;
    }
    void AppFrame::UpdateInput()
    {
        g_Keyboard.GetState(g_KeyboardState, true);
        if (Mouse)
        {
            MouseState = Mouse->GetState();
            Mouse->ResetScrollWheelValue();
        }
        else
        {
            ZeroMemory(&MouseState, sizeof(MouseState));
        }
    }
    void AppFrame::ResetKeyboardInput()
    {
        g_KeyboardState.Reset();
    }
    void AppFrame::ResetMouseInput()
    {
        ZeroMemory(&MouseState, sizeof(MouseState));
    }

    bool AppFrame::GetKeyState(int VKCode)noexcept
    {
        return g_KeyboardState.IsKeyDown((Platform::Keyboard::Key)VKCode);
    }
    
    int AppFrame::GetLastKey()noexcept
    {
        return (int)g_KeyboardState.LastKeyDown;
    }
    
    inline Core::Vector2F MapLetterBoxingPosition(Core::Vector2U isize, Core::Vector2U osize, Core::Vector2I pos)
    {
        float const hscale = (float)osize.x / (float)isize.x;
        float const vscale = (float)osize.y / (float)isize.y;
        float const scale = std::min(hscale, vscale);
        float const sizew = scale * (float)isize.x;
        float const sizeh = scale * (float)isize.y;
        float const dx = ((float)osize.x - sizew) * 0.5f;
        float const dy = ((float)osize.y - sizeh) * 0.5f;
        float const x1 = (float)pos.x - dx;
        float const y1 = (float)pos.y - dy;
        float const x2 = x1 / scale;
        float const y2 = y1 / scale;
        return Core::Vector2F(x2, y2);
    }

    bool AppFrame::GetMouseState_legacy(int button)noexcept
    {
        switch (button)
        {
        case 0:
            return MouseState.leftButton;
        case 1:
            return MouseState.middleButton;
        case 2:
            return MouseState.rightButton;
        case 3:
            return MouseState.xButton1;
        case 4:
            return MouseState.xButton2;
        default:
            return false;
        }
    }
    bool AppFrame::GetMouseState(int button)noexcept
    {
        switch (button)
        {
        case VK_LBUTTON:
            return MouseState.leftButton;
        case VK_MBUTTON:
            return MouseState.middleButton;
        case VK_RBUTTON:
            return MouseState.rightButton;
        case VK_XBUTTON1:
            return MouseState.xButton1;
        case VK_XBUTTON2:
            return MouseState.xButton2;
        default:
            return false;
        }
    }
    Core::Vector2F AppFrame::GetMousePosition(bool no_flip)noexcept
    {
        if (m_win32_window_size.x == 0 || m_win32_window_size.y == 0)
        {
            RECT rc = {};
            GetClientRect((HWND)GetAppModel()->getWindow()->getNativeHandle(), &rc);
            m_win32_window_size = Core::Vector2U((uint32_t)(rc.right - rc.left), (uint32_t)(rc.bottom - rc.top));
        }
        auto const w_size = m_win32_window_size;
        auto const c_size = GetAppModel()->getSwapChain()->getCanvasSize();
        auto const m_p = MapLetterBoxingPosition(c_size, w_size, Core::Vector2I(MouseState.x, MouseState.y));
        if (no_flip)
        {
            return m_p;
        }
        else
        {
            return Core::Vector2F(m_p.x, (float)c_size.y - m_p.y);
        }
    }
    Core::Vector2F AppFrame::GetCurrentWindowSizeF()
    {
        if (m_win32_window_size.x == 0 || m_win32_window_size.y == 0)
        {
            RECT rc = {};
            GetClientRect((HWND)GetAppModel()->getWindow()->getNativeHandle(), &rc);
            m_win32_window_size = Core::Vector2U((uint32_t)(rc.right - rc.left), (uint32_t)(rc.bottom - rc.top));
        }
        auto const w_size = m_win32_window_size;
        return Core::Vector2F((float)w_size.x, (float)w_size.y);
    }
    Core::Vector4F AppFrame::GetMousePositionTransformF()
    {
        if (m_win32_window_size.x == 0 || m_win32_window_size.y == 0)
        {
            RECT rc = {};
            GetClientRect((HWND)GetAppModel()->getWindow()->getNativeHandle(), &rc);
            m_win32_window_size = Core::Vector2U((uint32_t)(rc.right - rc.left), (uint32_t)(rc.bottom - rc.top));
        }
        auto const w_size = m_win32_window_size;
        auto const c_size = GetAppModel()->getSwapChain()->getCanvasSize();

        float const hscale = (float)w_size.x / (float)c_size.x;
        float const vscale = (float)w_size.y / (float)c_size.y;
        float const scale = std::min(hscale, vscale);
        float const sizew = scale * (float)c_size.x;
        float const sizeh = scale * (float)c_size.y;
        float const dx = ((float)w_size.x - sizew) * 0.5f;
        float const dy = ((float)w_size.y - sizeh) * 0.5f;

        return Core::Vector4F(-dx, -dy, 1.0f / scale, 1.0f / scale);
    }
    int32_t AppFrame::GetMouseWheelDelta()noexcept
    {
        return MouseState.scrollWheelValue;
    }
};
