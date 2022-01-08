#pragma once
#ifndef WM_LBUTTONDOWN
#include <Windows.h>
#endif
#ifndef GET_X_LPARAM
#include <windowsx.h>
#endif

class WindowMoveSizeController
{
private:
    HWND window;
    RECT title_bar_rect;
    RECT minimize_button_rect;
    RECT close_button_rect;
    POINT last_mouse_pos;
    POINT last_mouse_dpos;
    POINT last_window_pos;
    BOOL is_enable;
    BOOL is_size_or_move;
    BOOL is_minimize_button_down;
    BOOL is_close_button_down;
private:
    void enterSizeMove(HWND hWnd, LPARAM lParam)
    {
        if (is_enable)
        {
            if (!is_size_or_move)
            {
                is_size_or_move = TRUE;
                ::SetCapture(hWnd);
            }
            POINT pt = {
                .x = GET_X_LPARAM(lParam),
                .y = GET_Y_LPARAM(lParam),
            };
            ::MapWindowPoints(hWnd, NULL, &pt, 1);
            last_mouse_pos.x = pt.x;
            last_mouse_pos.y = pt.y;
            last_mouse_dpos.x = 0;
            last_mouse_dpos.y = 0;
            RECT rc = {};
            ::GetWindowRect(hWnd, &rc);
            last_window_pos.x = rc.left;
            last_window_pos.y = rc.top;
        }
    }
    void onSizeMove(HWND hWnd, LPARAM lParam)
    {
        if (is_size_or_move)
        {
            POINT pt = {
                .x = GET_X_LPARAM(lParam),
                .y = GET_Y_LPARAM(lParam),
            };
            ::MapWindowPoints(hWnd, NULL, &pt, 1);
            POINT dpt = {
                .x = pt.x - last_mouse_pos.x,
                .y = pt.y - last_mouse_pos.y,
            };
            if (dpt.x != last_mouse_dpos.x || dpt.y != last_mouse_dpos.y)
            {
                last_mouse_dpos.x = dpt.x;
                last_mouse_dpos.y = dpt.y;
                if (is_enable)
                {
                    ::SetWindowPos(hWnd, NULL, last_window_pos.x + dpt.x, last_window_pos.y + dpt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }
        }
    }
    void leaveSizeMove(HWND hWnd, LPARAM lParam)
    {
        if (is_size_or_move)
        {
            is_size_or_move = FALSE;
            POINT pt = {
                .x = GET_X_LPARAM(lParam),
                .y = GET_Y_LPARAM(lParam),
            };
            ::MapWindowPoints(hWnd, NULL, &pt, 1);
            POINT dpt = {
                .x = pt.x - last_mouse_pos.x,
                .y = pt.y - last_mouse_pos.y,
            };
            if (dpt.x != last_mouse_dpos.x || dpt.y != last_mouse_dpos.y)
            {
                last_mouse_dpos.x = dpt.x;
                last_mouse_dpos.y = dpt.y;
                if (is_enable)
                {
                    ::SetWindowPos(hWnd, NULL, last_window_pos.x + dpt.x, last_window_pos.y + dpt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }
            ::ReleaseCapture();
        }
    }
    void abortSizeMove()
    {
        if (is_size_or_move)
        {
            is_size_or_move = FALSE;
            ::ReleaseCapture();
        }
    }
    bool isMouseInMinimizeButton(LPARAM lParam)
    {
        LONG mx = GET_X_LPARAM(lParam);
        LONG my = GET_Y_LPARAM(lParam);
        if (mx >= minimize_button_rect.left && mx < minimize_button_rect.right && my >= minimize_button_rect.top && my < minimize_button_rect.bottom)
        {
            return true;
        }
        return false;
    }
    bool isMouseInCloseButton(LPARAM lParam)
    {
        LONG mx = GET_X_LPARAM(lParam);
        LONG my = GET_Y_LPARAM(lParam);
        if (mx >= close_button_rect.left && mx < close_button_rect.right && my >= close_button_rect.top && my < close_button_rect.bottom)
        {
            return true;
        }
        return false;
    }
    bool isMouseInTitleBar(LPARAM lParam)
    {
        LONG mx = GET_X_LPARAM(lParam);
        LONG my = GET_Y_LPARAM(lParam);
        if (mx >= title_bar_rect.left && mx < title_bar_rect.right && my >= title_bar_rect.top && my < title_bar_rect.bottom)
        {
            return true;
        }
        return false;
    }
    void onMouseLeftButtonDown(HWND hWnd, LPARAM lParam)
    {
        if (isMouseInMinimizeButton(lParam))
        {
            is_minimize_button_down = TRUE;
        }
        else if (isMouseInCloseButton(lParam))
        {
            is_close_button_down = TRUE;
        }
        else if (isMouseInTitleBar(lParam))
        {
            enterSizeMove(hWnd, lParam);
        }
    }
    void onMouseLeftButtonUp(HWND hWnd, LPARAM lParam)
    {
        if (is_minimize_button_down)
        {
            is_minimize_button_down = FALSE;
            if (is_enable && isMouseInMinimizeButton(lParam))
            {
                ::ShowWindow(hWnd, SW_MINIMIZE);
            }
        }
        else if (is_close_button_down)
        {
            is_close_button_down = FALSE;
            if (is_enable && isMouseInCloseButton(lParam))
            {
                //::CloseWindow(hWnd);
                //::PostQuitMessage(0);
                ::SendMessageW(hWnd, WM_CLOSE, 0, 0);
            }
        }
        leaveSizeMove(hWnd, lParam);
    }
public:
    void setEnable(BOOL b) { is_enable = b; }
    void setMinimizeButtonRect(RECT rc) { minimize_button_rect = rc; }
    void setCloseButtonRect(RECT rc) { close_button_rect = rc; }
    void setTitleBarRect(RECT rc) { title_bar_rect = rc; }
    void setWindow(HWND hWnd)
    {
        if (window != hWnd)
        {
            is_minimize_button_down = FALSE;
            is_close_button_down = FALSE;
            abortSizeMove();
            if (window)
            {
                ::RemovePropW(window, L"W:SWCTRL");
            }
            window = hWnd;
            if (window)
            {
                ::SetPropW(window, L"W:SWCTRL", this);
            }
        }
    }
    struct Result
    {
        LRESULT lResult;
        BOOL bReturn;
    };
    static Result handleSizeMove(WindowMoveSizeController* self, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (self->window == hWnd)
        {
            switch (message)
            {
            case WM_ACTIVATEAPP:
                switch (wParam)
                {
                case FALSE:
                    self->is_minimize_button_down = FALSE;
                    self->is_close_button_down = FALSE;
                    self->abortSizeMove();
                    break;
                case TRUE:
                default:
                    break;
                }
            case WM_ACTIVATE:
                switch (wParam)
                {
                case WA_INACTIVE:
                    self->is_minimize_button_down = FALSE;
                    self->is_close_button_down = FALSE;
                    self->abortSizeMove();
                    break;
                case WA_ACTIVE:
                case WA_CLICKACTIVE:
                default:
                    break;
                }
            case WM_DEVMODECHANGE:
            case WM_DISPLAYCHANGE:
                //case WM_DPICHANGED:
                self->is_minimize_button_down = FALSE;
                self->is_close_button_down = FALSE;
                self->abortSizeMove();
                break;
            case WM_LBUTTONDOWN:
                self->onMouseLeftButtonDown(hWnd, lParam);
                break;
            case WM_MOUSEMOVE:
                self->onSizeMove(hWnd, lParam);
                break;
            case WM_LBUTTONUP:
                self->onMouseLeftButtonUp(hWnd, lParam);
                break;
            }
        }
        return Result{ .lResult = 0, .bReturn = FALSE };
    }
    static Result handleSizeMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (WindowMoveSizeController* self = (WindowMoveSizeController*)GetPropW(hWnd, L"W:SWCTRL"))
        {
            return handleSizeMove(self, hWnd, message, wParam, lParam);
        }
        return Result{ .lResult = 0, .bReturn = FALSE };
    }
public:
    WindowMoveSizeController()
        : window(NULL)
        , title_bar_rect({})
        , minimize_button_rect({})
        , close_button_rect({})
        , last_mouse_pos({})
        , last_mouse_dpos({})
        , last_window_pos({})
        , is_enable(FALSE)
        , is_size_or_move(FALSE)
        , is_minimize_button_down(FALSE)
        , is_close_button_down(FALSE)
    {
    }
    ~WindowMoveSizeController()
    {
        setWindow(NULL);
    }
};
