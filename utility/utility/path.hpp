#pragma once
#include <string>

namespace utility::path
{
    void to_slash(std::string& utf8_string);
    void to_slash(std::wstring& wide_string);
}
