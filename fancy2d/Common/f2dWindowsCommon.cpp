#include "Common/f2dWindowsCommon.h"
#include <cstdio>
#include <cstring>
#include "spdlog/spdlog.h"

HRESULT HResultCheck::operator=(HRESULT v)
{
    hr = v;
    if (FAILED(hr))
    {
        char buffer[512] = {};
        DWORD result = ::FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            hr,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            buffer,
            512,
            NULL);
#ifdef _DEBUG
        spdlog::error("[fancy2d] 文件：'{}' 第 {} 行：(HRESULT = 0x{:08X}) {}", cfile, line, static_cast<unsigned long>(hr), buffer);
#else
        spdlog::error("[fancy2d] (HRESULT = 0x{:08X}) {}", buffer);
#endif
    }
    return hr;
}
HResultCheck& HResultCheck::get(char const* cfile, wchar_t const* file, int line, wchar_t const* message)
{
    static HResultCheck v;
    v.cfile = cfile;
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
