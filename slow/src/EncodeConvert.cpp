#include "EncodeConvert.hpp"
#include <Windows.h>

namespace slow {
    inline bool multi_wide(UINT encode, const std::string& src, std::wstring& dest) {
        try {
            if (src.length() == 0) {
                return true;
            }
            int result = ::MultiByteToWideChar(encode, 0, src.c_str(), (int)src.length(), nullptr, 0);
            if (result > 0) {
                dest.resize((size_t)result);
                result = ::MultiByteToWideChar(encode, 0, src.c_str(), (int)src.length(), (LPWSTR)dest.data(), result);
                if (result != 0) {
                    return true;
                }
            }
        }
        catch(...) {}
        return false;
    }
    inline bool wide_multi(UINT encode, const std::wstring& src, std::string& dest) {
        try {
            if (src.length() == 0) {
                return true;
            }
            int result = ::WideCharToMultiByte(encode, 0, src.c_str(), (int)src.length(), nullptr, 0, nullptr, nullptr);
            if (result > 0) {
                dest.resize((size_t)result);
                result = ::WideCharToMultiByte(encode, 0, src.c_str(), (int)src.length(), (LPSTR)dest.data(), result, nullptr, nullptr);
                if (result != 0) {
                    return true;
                }
            }
        }
        catch(...) {}
        return false;
    }
    
    bool utf8_utf16(const std::string& src, std::wstring& dest) {
        return multi_wide(CP_UTF8, src, dest);
    }
    bool utf16_utf8(const std::wstring& src, std::string& dest) {
        return wide_multi(CP_UTF8, src, dest);
    }
    
    bool ansi_utf16(const std::string& src, std::wstring& dest) {
        return multi_wide(CP_ACP, src, dest);
    }
    bool utf16_ansi(const std::wstring& src, std::string& dest) {
        return wide_multi(CP_ACP, src, dest);
    }
    
    bool utf8_ansi(const std::string& src, std::string& dest) {
        std::wstring buffer;
        if (utf8_utf16(src, buffer)) {
            if (utf16_ansi(buffer, dest)) {
                return true;
            }
        }
        return false;
    }
    bool ansi_utf8(const std::string& src, std::string& dest) {
        std::wstring buffer;
        if (ansi_utf16(src, buffer)) {
            if (utf16_utf8(buffer, dest)) {
                return true;
            }
        }
        return false;
    }
};
