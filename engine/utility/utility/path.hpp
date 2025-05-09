#pragma once
#include <string>
#include <string_view>

namespace utility::path
{
    bool is_separator(char c);
    bool is_separator(wchar_t c);
    void to_slash(std::string& utf8_string);
    void to_slash(std::wstring& wide_string);
    void to_backslash(std::string& utf8_string);
    void to_backslash(std::wstring& wide_string);
    void merge_separator(std::string& utf8_string);
    void merge_separator(std::wstring& wide_string);
    bool compare(std::string_view p1, std::string_view p2);
    bool compare(std::wstring_view p1, std::wstring_view p2);
    bool compare(std::string_view utf8_path, std::wstring_view wide_path);
}
