#include "Platform/Shared.hpp"

#include "Platform/CommandLineArguments.hpp"

namespace Platform
{
	inline void wcstr_to_utf8(WCHAR const* wstr, std::string& u8str)
	{
		int const length = static_cast<int>(wcslen(wstr));
		if (length <= 0)
		{
			return; // empty string
		}
		int const count = WideCharToMultiByte(CP_UTF8, 0, wstr, length, NULL, 0, NULL, NULL);
		if (count <= 0)
		{
			return; // error
		}
		u8str.resize(static_cast<size_t>(count));
		int const write = WideCharToMultiByte(CP_UTF8, 0, wstr, length, u8str.data(), count, NULL, NULL);
		if (write != count)
		{
			u8str.clear();
			return; // error
		}
	}

	bool CommandLineArguments::Update()
	{
		m_args.clear();

		LPWSTR pCmdLine = GetCommandLineW();
		if (!pCmdLine)
		{
			return false;
		}

		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc);
		if (!argv)
		{
			return false;
		}

		m_args.resize(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i += 1)
		{
			wcstr_to_utf8(argv[i], m_args[i]);
		}
	
		LocalFree(argv);

		return true;
	}
	bool CommandLineArguments::GetArguments(std::vector<std::string_view>& list)
	{
		if (m_args.empty())
		{
			if (!Update())
			{
				return false;
			}
		}

		list.resize(m_args.size());
		for (size_t i = 0; i < m_args.size(); i += 1)
		{
			list[i] = m_args[i];
		}

		return true;
	}
	bool CommandLineArguments::IsOptionExist(std::string_view option)
	{
		if (m_args.empty())
		{
			if (!Update())
			{
				return false;
			}
		}

		for (auto const& v : m_args)
		{
			if (v == option)
			{
				return true;
			}
		}

		return false;
	}

	CommandLineArguments::CommandLineArguments()
	{
	}
	CommandLineArguments::~CommandLineArguments()
	{
	}

	CommandLineArguments& CommandLineArguments::Get()
	{
		static CommandLineArguments instace;
		return instace;
	}
}

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
