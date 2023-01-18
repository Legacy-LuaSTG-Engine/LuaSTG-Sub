#pragma once

namespace Platform
{
    class WindowSizeMoveController
    {
    public:
        struct Result
        {
            LRESULT lResult;
            BOOL bReturn;
        };
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
        void enterSizeMove(HWND hWnd, LPARAM lParam);
        void onSizeMove(HWND hWnd, LPARAM lParam);
        void leaveSizeMove(HWND hWnd, LPARAM lParam);
        void abortSizeMove();
        bool isMouseInMinimizeButton(LPARAM lParam);
        bool isMouseInCloseButton(LPARAM lParam);
        bool isMouseInTitleBar(LPARAM lParam);
        void onMouseLeftButtonDown(HWND hWnd, LPARAM lParam);
        void onMouseLeftButtonUp(HWND hWnd, LPARAM lParam);
    public:
        void setEnable(BOOL b);
        void setMinimizeButtonRect(RECT rc);
        void setCloseButtonRect(RECT rc);
        void setTitleBarRect(RECT rc);
        void setWindow(HWND hWnd);
        Result handleSizeMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        static Result handleSizeMoveStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    public:
        WindowSizeMoveController();
        ~WindowSizeMoveController();
    };
}
