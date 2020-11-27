#pragma once
#include <cstdarg>
#include <string>
#include <Windows.h>

// Log Helper
inline void DebugLog(const char* fmt, ...)
{
    char buffer[1024];
    buffer[1023] = 0;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 1023, fmt, args);
    va_end(args);
    OutputDebugStringA(buffer);
}
inline void DebugLog(const wchar_t* fmt, ...)
{
    wchar_t buffer[1024];
    buffer[1023] = 0;
    va_list args;
    va_start(args, fmt);
    vswprintf(buffer, 1023, fmt, args);
    va_end(args);
    OutputDebugStringW(buffer);
}
