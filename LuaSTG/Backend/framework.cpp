#include "Backend/framework.hpp"
#include <cstdio>
#include <cstring>

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

HRESULT HResultCheck::operator=(HRESULT v)
{
    hr = v;
    if (FAILED(hr))
    {
        WCHAR buffer[256] = {};
        swprintf_s(buffer, L"line: %d\n", line);

        OutputDebugStringW(L"file: ");
        OutputDebugStringW(file);
        OutputDebugStringW(L"\n");

        OutputDebugStringW(buffer);

        debugPrintHRESULT(hr, message);
    }
    return hr;
}
HResultCheck& HResultCheck::get(wchar_t const* file, int line, wchar_t const* message)
{
    static HResultCheck v;
    v.file = file;
    v.line = line;
    v.message = message;
    return v;
}

bool HResultToBool::operator=(HRESULT v)
{
    hr = v;
    return SUCCEEDED(v);
}
HResultToBool& HResultToBool::get()
{
    static HResultToBool v;
    return v;
}
