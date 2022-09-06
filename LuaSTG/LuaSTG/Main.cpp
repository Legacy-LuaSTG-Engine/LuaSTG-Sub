#include "Debugger/Logger.hpp"
#include "SteamAPI/SteamAPI.hpp"
#include "Utility/Utility.h"
#include "AppFrame.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	LuaSTGPlus::CoInitializeScope com_runtime;
	if (!com_runtime())
	{
		MessageBoxW(
			NULL,
			L"引擎初始化失败。\n"
			L"未能正常初始化COM组件库，请尝试重新启动此应用程序。",
			L"" LUASTG_INFO,
			MB_ICONERROR | MB_OK);
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
			MessageBoxW(
				NULL,
				L"引擎初始化失败。\n"
				L"请尝试重新启动此应用程序，或者联系开发人员。\n"
				L"在日志文件（engine.log，可以用记事本打开）中可以获得更多信息。",
				L"" LUASTG_INFO,
				MB_ICONERROR | MB_OK);
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
