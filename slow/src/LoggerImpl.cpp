#include "slow/Logger.hpp"
#include <cstdarg>
#include <iostream>
#include <Windows.h>

namespace slow {
    static char utf8_buffer[4096];
    static wchar_t utf16_buffer[2048];
    
    constexpr size_t utf8_len = sizeof(utf8_buffer) / sizeof(char);
    constexpr size_t utf16_len = sizeof(utf16_buffer) / sizeof(wchar_t);
    constexpr size_t utf8_len_2 = utf8_len - 2;
    constexpr size_t utf16_len_2 = utf16_len - 2;
    
    void DebugOutput(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int cnt = vsnprintf(utf8_buffer, utf8_len_2, fmt, args);
        if (cnt >= 0) {
            int need = MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, cnt, nullptr, 0);
            if (need >= 0 && need <= utf16_len_2) {
                int write = MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, cnt, utf16_buffer, need);
                if (write >= 0) {
                    utf16_buffer[write] = L'\n';
                    utf16_buffer[write + 1] = L'\0';
                    OutputDebugStringW(utf16_buffer);
                }
            }
        }
        va_end(args);
    }
    
    void DebugOutput(const wchar_t* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int cnt = vswprintf(utf16_buffer, utf16_len_2, fmt, args);
        if (cnt >= 0) {
            utf16_buffer[cnt] = L'\n';
            utf16_buffer[cnt + 1] = L'\0';
            OutputDebugStringW(utf16_buffer);
        }
        va_end(args);
    }
};
