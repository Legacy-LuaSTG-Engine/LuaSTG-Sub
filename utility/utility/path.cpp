#include "utility/path.hpp"

namespace utility::path
{
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
}
