#pragma once

namespace Platform
{
	class HighDPI
	{
	public:
		static void Enable();
		static BOOL EnableNonClientDpiScaling(HWND hWnd);
		static BOOL AdjustWindowRectExForDpi(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
		static UINT GetDpiForSystem();
		static UINT GetDpiForMonitor(HMONITOR hMonitor);
		static UINT GetDpiForWindow(HWND hWnd);
		static FLOAT GetDpiScalingForSystem();
		static FLOAT GetDpiScalingForMonitor(HMONITOR hMonitor);
		static FLOAT GetDpiScalingForWindow(HWND hWnd);
		template<typename T>
		static inline T ScalingByDpi(T value, UINT dpi)
		{
			return (T)::MulDiv((int)value, (int)dpi, USER_DEFAULT_SCREEN_DPI);
		}
		static inline float ScalingFromDpi(UINT dpi)
		{
			return (float)dpi / (float)USER_DEFAULT_SCREEN_DPI;
		}
	};
}
