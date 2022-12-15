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

	static BOOL _IsOverlayTestModeExists(DWORD option)
	{
		// 在 MPO 启用且支持工作的情况下，DWM 会专门分配一个 plane 来独立呈现
		// 但是 MPO 似乎因为各种问题给用户带来了困扰，NVIDIA 甚至专门给出了禁用方法
		// https://nvidia.custhelp.com/app/answers/detail/a_id/5157/~/after-updating-to-nvidia-game-ready-driver-461.09-or-newer%2C-some-desktop-apps
		// 禁用 MPO 后，DirectComposition 的延迟会大大增加，因为 DWM 会始终控制合成
		// 为了处理这个特例，我们需要检测该注册表项

		LSTATUS ls = ERROR_SUCCESS;

		ScopeRegKey hKey;
		ls = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\Dwm", 0, KEY_READ | option, hKey.GetAddressOf());
		if (ERROR_SUCCESS != ls) return FALSE;

		DWORD dwValue = 0;
		DWORD dwValueSize = 4;
		DWORD dwType = 0;
		ls = RegQueryValueExW(hKey.Get(), L"OverlayTestMode", NULL, &dwType, (BYTE*)&dwValue, &dwValueSize);
		if (ERROR_SUCCESS != ls) return FALSE;
		if (REG_DWORD != dwType) return FALSE;

		return 5 == dwValue;
	}

	BOOL DesktopWindowManager::IsOverlayTestModeExists()
	{
		if (_IsOverlayTestModeExists(0x0))
		{
			return TRUE; // 理论上在 64 位程序下会触发这个
		}

		if (_IsOverlayTestModeExists(KEY_WOW64_32KEY))
		{
			return TRUE; // 理论上不会触发，但是以防万一，省的微软妈又死了
		}

		if (_IsOverlayTestModeExists(KEY_WOW64_64KEY))
		{
			return TRUE; // 理论上在 32 位程序下会触发这个
		}

		return FALSE;
	}
}
