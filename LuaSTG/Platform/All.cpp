#include "Platform/Shared.hpp"

#include "Platform/DesktopWindowManager.hpp"

namespace Platform
{
	class ScopeRegKey
	{
	private:
		HKEY hKey{ NULL };
	public:
		HKEY Get() const noexcept { return hKey; }
		HKEY* GetAddressOf() noexcept { return &hKey; }
		void Close() { if (hKey) RegCloseKey(hKey); hKey = NULL; }
	public:
		ScopeRegKey() = default;
		~ScopeRegKey() { Close(); }
	};

	BOOL DesktopWindowManager::IsOverlayTestModeExists()
	{
		LSTATUS ls = ERROR_SUCCESS;

		ScopeRegKey hKey;
		ls = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\Dwm", 0, KEY_READ, hKey.GetAddressOf());
		if (ERROR_SUCCESS != ls) return FALSE;
		
		// 提醒：这个值读取不出来，一直返回 ERROR_FILE_NOT_FOUND
		// 但是同一个键下的其他值是能读取的，不知道傻逼微软搞了什么寄吧东西（😅）
		DWORD dwValue = 0;
		DWORD dwValueSize = 4;
		DWORD dwType = 0;
		ls = RegQueryValueExW(hKey.Get(), L"OverlayTestMode", NULL, &dwType, (BYTE*)&dwValue, &dwValueSize);
		if (ERROR_SUCCESS != ls) return FALSE;
		if (REG_DWORD != dwType) return FALSE;

		return 5 == dwValue;
	}
}
