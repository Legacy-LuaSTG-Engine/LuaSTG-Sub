#pragma once
#include <string>

namespace slow {
    bool utf8_utf16(const std::string& src, std::wstring& dest);
    bool utf16_utf8(const std::wstring& src, std::string& dest);
    
    bool ansi_utf16(const std::string& src, std::wstring& dest);
    bool utf16_ansi(const std::wstring& src, std::string& dest);
    
    bool utf8_ansi(const std::string& src, std::string& dest);
    bool ansi_utf8(const std::string& src, std::string& dest);
};
