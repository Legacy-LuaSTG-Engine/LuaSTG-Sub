#pragma once

namespace Platform
{
	class WindowTheme
	{
	public:
		static BOOL IsSystemDarkModeEnabled();
		static BOOL ShouldApplicationEnableDarkMode();
		static BOOL SetDarkMode(HWND hWnd, BOOL bEnable, BOOL bFocus);
		static BOOL UpdateColorMode(HWND hWnd, BOOL bFocus);
	};
}
