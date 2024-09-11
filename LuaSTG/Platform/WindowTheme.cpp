#include "Shared.hpp"
#include "WindowTheme.hpp"

namespace Platform
{
	// https://github.com/microsoft/WindowsAppSDK/issues/41#

	// optional Mica support (but have bug)
	//   DWORD const DWMWA_MICA_EFFFECT = 0x405;
	//   BOOL mica_effect = TRUE;
	//   HRESULT hr = loader().api_DwmSetWindowAttribute(hWnd, DWMWA_MICA_EFFFECT, &mica_effect, sizeof(mica_effect));

	struct DwmapiLoader
	{
		HMODULE dll_dwmapi = NULL;
		decltype(::DwmSetWindowAttribute)* api_DwmSetWindowAttribute = NULL;
		decltype(::DwmGetWindowAttribute)* api_DwmGetWindowAttribute = NULL;
		decltype(::DwmEnableBlurBehindWindow)* api_DwmEnableBlurBehindWindow = NULL;
		decltype(::DwmExtendFrameIntoClientArea)* api_DwmExtendFrameIntoClientArea = NULL;
		DwmapiLoader()
		{
			dll_dwmapi = LoadLibraryW(L"dwmapi.dll");
			if (dll_dwmapi)
			{
				api_DwmSetWindowAttribute = (decltype(::DwmSetWindowAttribute)*)GetProcAddress(dll_dwmapi, "DwmSetWindowAttribute");
				api_DwmGetWindowAttribute = (decltype(::DwmGetWindowAttribute)*)GetProcAddress(dll_dwmapi, "DwmGetWindowAttribute");
				api_DwmEnableBlurBehindWindow = (decltype(::DwmEnableBlurBehindWindow)*)GetProcAddress(dll_dwmapi, "DwmEnableBlurBehindWindow");
				api_DwmExtendFrameIntoClientArea = (decltype(::DwmExtendFrameIntoClientArea)*)GetProcAddress(dll_dwmapi, "DwmExtendFrameIntoClientArea");
			}
		}
		~DwmapiLoader()
		{
			if (dll_dwmapi)
			{
				FreeLibrary(dll_dwmapi);
			}
		}
	};
	static DwmapiLoader& loader()
	{
		static DwmapiLoader instance;
		return instance;
	}

	BOOL WindowTheme::IsSystemDarkModeEnabled()
	{
		HKEY hKey = NULL;
		if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey))
		{
			DWORD dwValue = 0;
			DWORD dwValueSize = 4;
			DWORD dwType = 0;
			if (ERROR_SUCCESS == RegQueryValueExW(hKey, L"SystemUsesLightTheme", NULL, &dwType, (BYTE*)&dwValue, &dwValueSize))
			{
				RegCloseKey(hKey);
				return dwValue == 0;
			}
			else
			{
				RegCloseKey(hKey);
				return FALSE;
			}
		}
		return FALSE;
	}
	BOOL WindowTheme::ShouldApplicationEnableDarkMode()
	{
		auto const key_path{ L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize" };
		auto const value_name{ L"AppsUseLightTheme" };
		DWORD type{};
		DWORD value{};
		DWORD size{ sizeof(DWORD) };
		if (ERROR_SUCCESS != RegGetValueW(HKEY_CURRENT_USER, key_path, value_name, RRF_RT_REG_DWORD, &type, &value, &size)) {
			size = DWORD{ sizeof(DWORD) };
			if (ERROR_SUCCESS != RegGetValueW(HKEY_CURRENT_USER, key_path, value_name, RRF_RT_REG_DWORD | KEY_WOW64_64KEY, &type, &value, &size)) {
				size = DWORD{ sizeof(DWORD) };
				if (ERROR_SUCCESS != RegGetValueW(HKEY_CURRENT_USER, key_path, value_name, RRF_RT_REG_DWORD | KEY_WOW64_32KEY, &type, &value, &size)) {
					return false;
				}
			}
		}
		if (type == REG_DWORD && size == sizeof(DWORD)) {
			return value == 0;
		}
		return false;
	}
	BOOL WindowTheme::SetDarkMode(HWND hWnd, BOOL bEnable, BOOL bFocus)
	{
		if (loader().api_DwmSetWindowAttribute)
		{
			// Method 1 (Native but not good?): DWMWA_USE_IMMERSIVE_DARK_MODE
			
			std::ignore = bFocus;
			BOOL dark_mode = bEnable;
			HRESULT hr = loader().api_DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));
			return SUCCEEDED(hr);
			
			// Method 2: DWMWA_CAPTION_COLOR
			
			//COLORREF color = bEnable ? (bFocus ? 0x00202020 : 0x002B2B2B) : 0x00FFFFFF; // 0x002E2E2E
			//HRESULT hr = loader().api_DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));
			//return SUCCEEDED(hr);
		}
		return FALSE;
	}
	BOOL WindowTheme::UpdateColorMode(HWND hWnd, BOOL bFocus)
	{
		return SetDarkMode(hWnd, ShouldApplicationEnableDarkMode(), bFocus);
	}
}
