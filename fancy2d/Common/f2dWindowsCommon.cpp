#include "Common/f2dWindowsCommon.h"
#include <cstdio>

void debugPrintHRESULT(HRESULT hr, const wchar_t* message) noexcept
{
    WCHAR buffer[256] = {};
    DWORD result = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        buffer,
        256,
        NULL);
    OutputDebugStringW(message);
    OutputDebugStringW(L"\n");
    OutputDebugStringW(buffer);
    OutputDebugStringW(L"\n");
}
