#include "RuntimeCheck.hpp"
#include "Platform/MessageBox.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/CleanWindows.hpp"
#include <shellapi.h>

namespace LuaSTG
{
	static void OpenWebsiteWindows7EndOfSupport()
	{
		ShellExecuteW(NULL, NULL, L"https://www.microsoft.com/en-us/windows/end-of-support", NULL, NULL, SW_SHOWNORMAL);
	}
	static void OpenWebsiteDownloadWindows7SP1()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932", NULL, NULL, SW_SHOWNORMAL);
	}
	static void OpenWebsiteDownloadPlatformUpdateForWindows7()
	{
		ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838", NULL, NULL, SW_SHOWNORMAL);
	}
	bool CheckUserRuntime()
	{
		if (!Platform::WindowsVersion::Is7SP1WithPlatformUpdate())
		{
			if (!Platform::WindowsVersion::Is7SP1())
			{
				if (!Platform::WindowsVersion::Is7())
				{
					// 连 Windows 7 都没达到
					Platform::MessageBox::Error(LUASTG_INFO,
						"Minimum system requirements are not met.\n"
						"Satisfied:\n"
						"    N/A\n"
						"Not Satisfied:\n"
						"    Windows 7\n"
						"    Windows 7 Service Pack 1 (KB976932)\n"
						"    Platform Update for Windows 7 (KB2670838)\n"
						"\n"
						"未达到最低系统要求。\n"
						"已满足的条件：\n"
						"    N/A\n"
						"未满足的条件：\n"
						"    Windows 7\n"
						"    Windows 7 Service Pack 1 (KB976932)\n"
						"    Platform Update for Windows 7 (KB2670838)");
					(void)&OpenWebsiteWindows7EndOfSupport;
				}
				else
				{
					// 没装 Windows 7 SP1
					bool const ok = Platform::MessageBox::Warning(LUASTG_INFO,
						"Minimum system requirements are not met.\n"
						"Satisfied:\n"
						"    Windows 7\n"
						"Not Satisfied:\n"
						"    Windows 7 Service Pack 1 (KB976932)\n"
						"    Platform Update for Windows 7 (KB2670838)\n"
						"Click the \"OK\" button to open Microsoft's official website to download the update.\n"
						"Click the \"Cancel\" button to exit.\n"
						"\n"
						"未达到最低系统要求。\n"
						"已满足的条件：\n"
						"    Windows 7\n"
						"未满足的条件：\n"
						"    Windows 7 Service Pack 1 (KB976932)\n"
						"    Platform Update for Windows 7 (KB2670838)\n"
						"点击“确定”打开微软官方网站下载更新。\n"
						"点击“取消”关闭程序。");
					if (ok)
					{
						OpenWebsiteDownloadWindows7SP1();
						Sleep(1000);
						OpenWebsiteDownloadPlatformUpdateForWindows7();
					}
				}
			}
			else
			{
				// 没装平台更新
				bool const ok = Platform::MessageBox::Warning(LUASTG_INFO,
					"Minimum system requirements are not met.\n"
					"Satisfied:\n"
					"    Windows 7\n"
					"    Windows 7 Service Pack 1 (KB976932)\n"
					"Not Satisfied:\n"
					"    Platform Update for Windows 7 (KB2670838)\n"
					"Click the \"OK\" button to open Microsoft's official website to download the update.\n"
					"Click the \"Cancel\" button to exit.\n"
					"\n"
					"未达到最低系统要求。\n"
					"已满足的条件：\n"
					"    Windows 7\n"
					"    Windows 7 Service Pack 1 (KB976932)\n"
					"未满足的条件：\n"
					"    Platform Update for Windows 7 (KB2670838)\n"
					"点击“确定”打开微软官方网站下载更新。\n"
					"点击“取消”关闭程序。");
				if (ok)
				{
					OpenWebsiteDownloadPlatformUpdateForWindows7();
				}
			}
			return false;
		}
		return true;
	}
}
