#include "utility/path.hpp"
#include "utility/encoding.hpp"

namespace utility::path
{
    bool is_separator(char c) { return c == '/' || c == '\\'; }
    bool is_separator(wchar_t c) { return c == L'/' || c == L'\\'; }
    void to_slash(std::string& utf8_string)
    {
        for (auto& c : utf8_string)
        {
            if (c == '\\') c = '/';
        }
    }
    void to_slash(std::wstring& wide_string)
    {
        for (auto& c : wide_string)
        {
            if (c == L'\\') c = L'/';
        }
    }
    void to_backslash(std::string& utf8_string)
    {
        for (auto& c : utf8_string)
        {
            if (c == '/') c = '\\';
        }
    }
    void to_backslash(std::wstring& wide_string)
    {
        for (auto& c : wide_string)
        {
            if (c == L'/') c = L'\\';
        }
    }
    void merge_separator(std::string& utf8_string)
    {
        size_t const string_size = utf8_string.size();
        bool slash_section = false;
        size_t j = 0;
        for (size_t i = 0; i < string_size; i += 1)
        {
            if (!slash_section)
            {
                if (is_separator(utf8_string[i]))
                {
                    slash_section = true;
                }
                if (j != i)
                {
                    utf8_string[j] = utf8_string[i];
                }
                j += 1;
            }
            else
            {
                if (!is_separator(utf8_string[i]))
                {
                    slash_section = false;
                    if (j != i)
                    {
                        utf8_string[j] = utf8_string[i];
                    }
                    j += 1;
                }
            }
        }
        utf8_string.resize(j);
    }
    void merge_separator(std::wstring& wide_string)
    {
        size_t const string_size = wide_string.size();
        bool slash_section = false;
        size_t j = 0;
        for (size_t i = 0; i < string_size; i += 1)
        {
            if (!slash_section)
            {
                if (is_separator(wide_string[i]))
                {
                    slash_section = true;
                }
                if (j != i)
                {
                    wide_string[j] = wide_string[i];
                }
                j += 1;
            }
            else
            {
                if (!is_separator(wide_string[i]))
                {
                    slash_section = false;
                    if (j != i)
                    {
                        wide_string[j] = wide_string[i];
                    }
                    j += 1;
                }
            }
        }
        wide_string.resize(j);
    }
    bool compare(std::string_view p1, std::string_view p2)
    {
        if (p1.size() != p2.size()) return false;
        for (size_t i = 0; i < p1.size(); i += 1)
        {
            if (is_separator(p1[i]) && is_separator(p2[i]))
            {
                // ok
            }
            else if (p1[i] == p2[i])
            {
                // ok
            }
            else
            {
                return false; // not equal
            }
        }
        return true;
    }
    bool compare(std::wstring_view p1, std::wstring_view p2)
    {
        if (p1.size() != p2.size()) return false;
        for (size_t i = 0; i < p1.size(); i += 1)
        {
            if (is_separator(p1[i]) && is_separator(p2[i]))
            {
                // ok
            }
            else if (p1[i] == p2[i])
            {
                // ok
            }
            else
            {
                return false; // not equal
            }
        }
        return true;
    }
    bool compare(std::string_view utf8_path, std::wstring_view wide_path)
    {
        std::wstring p1(std::move(encoding::to_wide(utf8_path)));
        return compare(p1, wide_path);
    }
}
