#include "MessageBox.hpp"

namespace Platform
{
    static bool Show(int type, std::string_view title, std::string_view message, HWND window = NULL)
    {
        std::wstring wide_title(std::move(to_wide(title)));
        std::wstring wide_message(std::move(to_wide(message)));
        UINT flags = 0;
        switch (type)
        {
        case 1:
            flags |= MB_ICONWARNING;
            flags |= MB_OKCANCEL;
            break;
        case 2:
            flags = MB_ICONERROR;
            flags |= MB_OK;
            break;
        default:
            assert(false);
            break;
        }
        return IDOK == MessageBoxW(window, wide_message.c_str(), wide_title.c_str(), flags);
    }
    bool MessageBox::Warning(std::string_view title, std::string_view message)
    {
        return Show(1, title, message);
    }
    void MessageBox::Error(std::string_view title, std::string_view message)
    {
        Show(2, title, message);
    }
    bool MessageBox::WarningFromWindow(std::string_view title, std::string_view message, void* window)
    {
        return Show(1, title, message, (HWND)window);
    }
    void MessageBox::ErrorFromWindow(std::string_view title, std::string_view message, void* window)
    {
        Show(2, title, message, (HWND)window);
    }
}
