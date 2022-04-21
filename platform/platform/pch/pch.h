#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#ifdef WINVER
#undef WINVER
#endif
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#include <sdkddkver.h>

#include <cassert>
#include <tuple>
#include <vector>
#include <string>
#include <string_view>

#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <ShellScalingApi.h>
#include <Shobjidl.h>
#include <Knownfolders.h>

namespace platform
{
	inline std::string to_utf8(std::wstring_view const& src)
	{
		int const size = WideCharToMultiByte(CP_UTF8, 0, src.data(), (int)src.length(), NULL, 0, NULL, NULL);
		if (size > 0)
		{
			std::string buf(size, '\0');
			int const result = WideCharToMultiByte(CP_UTF8, 0, src.data(), (int)src.length(), buf.data(), size, NULL, NULL);
			if (result == size)
			{
				return buf;
			}
		}
		return "";
	}
	inline std::wstring to_wide(std::string_view const& src)
	{
		int const size = MultiByteToWideChar(CP_UTF8, 0, src.data(), (int)src.length(), NULL, 0);
		if (size > 0)
		{
			std::wstring buf(size, L'\0');
			int const result = MultiByteToWideChar(CP_UTF8, 0, src.data(), (int)src.length(), buf.data(), size);
			if (result == size)
			{
				return buf;
			}
		}
		return L"";
	}
}
