#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <Windows.h>

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

enum LogLevel
{
    Verbose,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
};

template<size_t N = 1024>
inline void OutputLog(LogLevel lv, const char* fmt, ...)
{
    static_assert(N > 6, "size N must larger than 6");
    char buffer[N] = { '[', '?', ']', ' ', '\n', '\0' };
    switch (lv)
    {
        case LogLevel::Verbose: buffer[1] = 'V'; break;
        case LogLevel::Debug  : buffer[1] = 'D'; break;
        case LogLevel::Info   : buffer[1] = 'I'; break;
        case LogLevel::Warn   : buffer[1] = 'W'; break;
        case LogLevel::Error  : buffer[1] = 'E'; break;
        case LogLevel::Fatal  : buffer[1] = 'F'; break;
    }
    va_list args;
    va_start(args, fmt);
    const auto x = vsnprintf(buffer + 4, N - 6, fmt, args);
    va_end(args);
    if (x >= 0)
    {
        buffer[4 + x] = '\n';
        buffer[5 + x] = '\0';
    }
    OutputDebugStringA(buffer);
    printf(buffer);
}

template<size_t N = 1024>
inline void OutputLog(LogLevel lv, const wchar_t* fmt, ...)
{
    static_assert(N > 5, "size N must larger than 6");
    wchar_t buffer[N] = { L'[', L'?', L']', L' ', L'\n', L'\0' };
    switch (lv)
    {
        case LogLevel::Verbose: buffer[1] = L'V'; break;
        case LogLevel::Debug  : buffer[1] = L'D'; break;
        case LogLevel::Info   : buffer[1] = L'I'; break;
        case LogLevel::Warn   : buffer[1] = L'W'; break;
        case LogLevel::Error  : buffer[1] = L'E'; break;
        case LogLevel::Fatal  : buffer[1] = L'F'; break;
    }
    va_list args;
    va_start(args, fmt);
    const auto x = vswprintf(buffer + 4, N - 6, fmt, args);
    va_end(args);
    if (x >= 0)
    {
        buffer[4 + x] = L'\n';
        buffer[5 + x] = L'\0';
    }
    OutputDebugStringW(buffer);
    wprintf(buffer);
}
