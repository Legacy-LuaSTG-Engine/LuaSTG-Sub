#include "utility/encoding.hpp"
#include <Windows.h>

namespace utility::encoding
{
    std::string to_utf8(std::wstring_view wide_string)
    {
        int const size = ::WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.length(), NULL, 0, NULL, NULL);
        if (size > 0)
        {
            std::string buf(size, '\0');
            int const result = ::WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.length(), buf.data(), size, NULL, NULL);
            if (result == size)
            {
                return std::move(buf);
            }
        }
        return "";
    }
    std::wstring to_wide(std::string_view utf8_string)
    {
        int const size = ::MultiByteToWideChar(CP_UTF8, 0, utf8_string.data(), (int)utf8_string.length(), NULL, 0);
        if (size > 0)
        {
            std::wstring buf(size, L'\0');
            int const result = ::MultiByteToWideChar(CP_UTF8, 0, utf8_string.data(), (int)utf8_string.length(), buf.data(), size);
            if (result == size)
            {
                return std::move(buf);
            }
        }
        return L"";
    }
    std::string to_ansi(std::string_view utf8_string)
    {
        return std::move(to_ansi(to_wide(utf8_string)));
    }
    std::string to_ansi(std::wstring_view wide_string)
    {
        int const size = ::WideCharToMultiByte(CP_ACP, 0, wide_string.data(), (int)wide_string.length(), NULL, 0, NULL, NULL);
        if (size > 0)
        {
            std::string buf(size, '\0');
            int const result = ::WideCharToMultiByte(CP_ACP, 0, wide_string.data(), (int)wide_string.length(), buf.data(), size, NULL, NULL);
            if (result == size)
            {
                return std::move(buf);
            }
        }
        return "";
    }
}
