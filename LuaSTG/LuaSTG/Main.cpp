#include "platform/MessageBox.hpp"
#include "platform/WindowsVersion.hpp"
#include "Debugger/Logger.hpp"
#include "SteamAPI/SteamAPI.hpp"
#include "Utility/Utility.h"
#include "AppFrame.h"

void openWebsiteWindows7EndOfSupport();
void openWebsiteDownloadWindows7SP1();
void openWebsiteDownloadPlatformUpdateForWindows7();

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	if (!platform::WindowsVersion::Is7SP1WithPlatformUpdate())
	{
		if (!platform::WindowsVersion::Is7SP1())
		{
			if (!platform::WindowsVersion::Is7())
			{
				// 连 Windows 7 都没达到
				platform::MessageBox::Error(LUASTG_INFO,
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
			}
			else
			{
				// 没装 Windows 7 SP1
				bool const ok = platform::MessageBox::Warning(LUASTG_INFO,
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
					openWebsiteDownloadWindows7SP1();
					openWebsiteDownloadPlatformUpdateForWindows7();
				}
			}
		}
		else
		{
			// 没装平台更新
			bool const ok = platform::MessageBox::Warning(LUASTG_INFO,
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
				openWebsiteDownloadPlatformUpdateForWindows7();
			}
		}
		return EXIT_FAILURE;
	}

	LuaSTGPlus::CoInitializeScope com_runtime;
	if (!com_runtime())
	{
		platform::MessageBox::Error(LUASTG_INFO,
			"引擎初始化失败。\n"
			"未能正常初始化COM组件库，请尝试重新启动此应用程序。");
		return EXIT_FAILURE;
	}

	LuaSTG::Debugger::Logger::create();

	int result = EXIT_SUCCESS;
	if (LuaSTG::SteamAPI::Init())
	{
		if (LAPP.Init())
		{
			LAPP.Run();
			LAPP.Shutdown();
			result = EXIT_SUCCESS;
		}
		else
		{
			platform::MessageBox::Error(LUASTG_INFO,
				"引擎初始化失败。\n"
				"查看日志文件（engine.log，可以用记事本打开）可以获得更多信息。\n"
				"请尝试重新启动此应用程序，或者联系开发人员。");
			result = EXIT_FAILURE;
		}
		LuaSTG::SteamAPI::Shutdown();
	}
	else
	{
		result = EXIT_FAILURE;
	}

	LuaSTG::Debugger::Logger::destroy();

	return result;
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

void openWebsiteWindows7EndOfSupport()
{
	ShellExecuteW(NULL, NULL, L"https://www.microsoft.com/en-us/windows/end-of-support", NULL, NULL, SW_SHOWNORMAL);
}
void openWebsiteDownloadWindows7SP1()
{
	ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB976932", NULL, NULL, SW_SHOWNORMAL);
}
void openWebsiteDownloadPlatformUpdateForWindows7()
{
	ShellExecuteW(NULL, NULL, L"https://www.catalog.update.microsoft.com/Search.aspx?q=KB2670838", NULL, NULL, SW_SHOWNORMAL);
}

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	return main();
}
