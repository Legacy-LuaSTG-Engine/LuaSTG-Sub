#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#include <cassert>
#include <tuple>
#include <array>
#include <vector>
#include <string>
#include <string_view>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX

#include <Windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <ShellScalingApi.h>
#include <Shobjidl.h>
#include <Knownfolders.h>
#include <VersionHelpers.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

VERSIONHELPERAPI
IsWindows10BuildOrGreater(DWORD build)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
	osvi.dwBuildNumber = build;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

VERSIONHELPERAPI
IsWindows11OrGreater()
{
	return IsWindows10BuildOrGreater(22000);
}

namespace Microsoft::WRL::Wrappers
{
	using ThreadHandle = HandleT<HandleTraits::HANDLENullTraits>;
	using SemaphoreHandle = HandleT<HandleTraits::HANDLENullTraits>;
}

// Now, let's FUCK Windows.h

#undef CreateWindow
#undef DefWindowProc
#undef DrawText
#undef MessageBox

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dcomp.h>
#include <dwrite_3.h>
#include <dwmapi.h>

#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef NOSERVICE
#undef NOMCX

// helper

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
