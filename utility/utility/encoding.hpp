#pragma once
#include <string>
#include <string_view>

namespace utility::encoding
{
    std::string to_utf8(std::wstring_view wide_string);
    std::wstring to_wide(std::string_view utf8_string);
}
