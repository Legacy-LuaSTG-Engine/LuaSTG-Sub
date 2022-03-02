#include "HighDPI.hpp"
#include <cassert>
#include <tuple>
#include <ShellScalingApi.h>

namespace platform
{
	namespace
	{
		struct Loader
		{
			BOOL is_enable;
			HMODULE dll_user32;
			HMODULE dll_shcore;
			decltype(::SetProcessDPIAware)* api_SetProcessDPIAware; // Windows Vista
			decltype(::SetProcessDpiAwareness)* api_SetProcessDpiAwareness; // Windows 8.1
			decltype(::SetProcessDpiAwarenessContext)* api_SetProcessDpiAwarenessContext; // Windows 10 1703
			decltype(::EnableNonClientDpiScaling)* api_EnableNonClientDpiScaling; // Windows 10 1607
			decltype(::GetDpiForMonitor)* api_GetDpiForMonitor; // Windows 8.1
			decltype(::AdjustWindowRectExForDpi)* api_AdjustWindowRectExForDpi; // Windows 10 1607
			decltype(::GetDpiForWindow)* api_GetDpiForWindow; // Windows 10 1607
			decltype(::GetDpiForSystem)* api_GetDpiForSystem; // Windows 10 1607
			Loader()
				: is_enable(FALSE)
				, dll_user32(NULL)
				, dll_shcore(NULL)
				, api_SetProcessDPIAware(NULL)
				, api_SetProcessDpiAwareness(NULL)
				, api_SetProcessDpiAwarenessContext(NULL)
				, api_EnableNonClientDpiScaling(NULL)
				, api_GetDpiForMonitor(NULL)
				, api_AdjustWindowRectExForDpi(NULL)
				, api_GetDpiForWindow(NULL)
				, api_GetDpiForSystem(NULL)
			{
				dll_user32 = ::LoadLibraryW(L"user32.dll");
				dll_shcore = ::LoadLibraryW(L"SHCore.dll");
				if (dll_user32)
				{
					api_SetProcessDPIAware = (decltype(api_SetProcessDPIAware))::GetProcAddress(dll_user32, "SetProcessDPIAware");
					api_SetProcessDpiAwarenessContext = (decltype(api_SetProcessDpiAwarenessContext))::GetProcAddress(dll_user32, "SetProcessDpiAwarenessContext");
					api_EnableNonClientDpiScaling = (decltype(api_EnableNonClientDpiScaling))::GetProcAddress(dll_user32, "EnableNonClientDpiScaling");
					api_AdjustWindowRectExForDpi = (decltype(api_AdjustWindowRectExForDpi))::GetProcAddress(dll_user32, "AdjustWindowRectExForDpi");
					api_GetDpiForWindow = (decltype(api_GetDpiForWindow))::GetProcAddress(dll_user32, "GetDpiForWindow");
					api_GetDpiForSystem = (decltype(api_GetDpiForSystem))::GetProcAddress(dll_user32, "GetDpiForSystem");
				}
				if (dll_shcore)
				{
					api_SetProcessDpiAwareness = (decltype(api_SetProcessDpiAwareness))::GetProcAddress(dll_shcore, "SetProcessDpiAwareness");
					api_GetDpiForMonitor = (decltype(api_GetDpiForMonitor))::GetProcAddress(dll_shcore, "GetDpiForMonitor");
				}
			}
			~Loader()
			{
				if (dll_user32) ::FreeLibrary(dll_user32);
				if (dll_shcore) ::FreeLibrary(dll_shcore);
			}
		};
		static Loader& loader()
		{
			static Loader api_;
			return api_;
		}
	}
	void HighDPI::enable()
	{
		if (loader().api_SetProcessDpiAwarenessContext)
		{
			BOOL ret_ = loader().api_SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
			if (ret_ || (ERROR_ACCESS_DENIED == GetLastError()))
			{
				loader().is_enable = TRUE;
				return;
			}
		}
		if (loader().api_SetProcessDpiAwareness)
		{
			HRESULT hr_ = loader().api_SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			if (SUCCEEDED(hr_) || (E_ACCESSDENIED == hr_))
			{
				loader().is_enable = TRUE;
				return;
			}
		}
		if (loader().api_SetProcessDPIAware)
		{
			if (loader().api_SetProcessDPIAware())
			{
				loader().is_enable = TRUE;
				return;
			}
		}
	}
	BOOL HighDPI::EnableNonClientDpiScaling(HWND hWnd)
	{
		if (loader().api_EnableNonClientDpiScaling)
		{
			return loader().api_EnableNonClientDpiScaling(hWnd);
		}
		return FALSE;
	}
	BOOL HighDPI::AdjustWindowRectExForDpi(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi)
	{
		if (loader().api_AdjustWindowRectExForDpi)
		{
			return loader().api_AdjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
		}
		return ::AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
	}
	UINT HighDPI::GetDpiForSystem()
	{
		if (loader().api_GetDpiForSystem)
		{
			return loader().api_GetDpiForSystem();
		}
		if (HDC hdc_ = ::GetDC(NULL))
		{
			const int dpix_ = ::GetDeviceCaps(hdc_, LOGPIXELSX);
			const int dpiy_ = ::GetDeviceCaps(hdc_, LOGPIXELSY);
			assert(dpix_ == dpiy_);
			std::ignore = dpiy_;
			::ReleaseDC(NULL, hdc_);
			return UINT(dpix_); // dpix_ should always equal to dpiy_ ?
		}
		return USER_DEFAULT_SCREEN_DPI;
	}
	UINT HighDPI::GetDpiForMonitor(HMONITOR hMonitor)
	{
		if (loader().api_GetDpiForMonitor)
		{
			UINT dpix_ = USER_DEFAULT_SCREEN_DPI;
			UINT dpiy_ = USER_DEFAULT_SCREEN_DPI;
			if (SUCCEEDED(loader().api_GetDpiForMonitor(hMonitor, MDT_DEFAULT, &dpix_, &dpiy_)))
			{
				assert(dpix_ == dpiy_);
				std::ignore = dpiy_;
				return dpix_; // dpix_ should always equal to dpiy_ ?
			}
		}
		return GetDpiForSystem();
	}
	UINT HighDPI::GetDpiForWindow(HWND hWnd)
	{
		if (loader().api_GetDpiForWindow)
		{
			return loader().api_GetDpiForWindow(hWnd);
		}
		HMONITOR monitor_ = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		assert(monitor_ != NULL); // tell me WHY?
		return GetDpiForMonitor(monitor_);
	}
	FLOAT HighDPI::GetDpiScalingForSystem()
	{
		return (FLOAT)GetDpiForSystem() / (FLOAT)USER_DEFAULT_SCREEN_DPI;
	}
	FLOAT HighDPI::GetDpiScalingForMonitor(HMONITOR hMonitor)
	{
		return (FLOAT)GetDpiForMonitor(hMonitor) / (FLOAT)USER_DEFAULT_SCREEN_DPI;
	}
	FLOAT HighDPI::GetDpiScalingForWindow(HWND hWnd)
	{
		return (FLOAT)GetDpiForWindow(hWnd) / (FLOAT)USER_DEFAULT_SCREEN_DPI;
	}
}
