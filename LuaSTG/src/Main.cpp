#include "AppFrame.h"
#include "Utility.h"
#include "SteamAPI/SteamAPI.hpp"
#include "LLogger.hpp"

#define NOMINMAX
#include <Windows.h>

_Use_decl_anno_impl_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR pCmdLine, int)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	#ifdef LDEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
	#endif
	
	LuaSTGPlus::CoInitializeScope com;
	if (!com())
	{
		::MessageBoxW(
				0,
				L"未能正常初始化COM组件库，请尝试重新启动此应用程序。",
				L"引擎初始化失败",
				MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}
	
	slow::openLogger();
	
	int ret = 0;
	if (LuaSTGPlus::SteamAPI::Init())
	{
		if (LAPP.Init())
		{
			LAPP.Run();
			LAPP.Shutdown();
			ret = EXIT_SUCCESS;
		}
		else
		{
			MessageBoxW(
				NULL,
				L"引擎未能成功初始化：\n"
				L"请尝试重新启动此应用程序，或者联系开发人员。\n"
				L"在日志文件（engine.log，可以用记事本打开）中可以获得更多信息。",
				L"引擎初始化失败",
				MB_ICONERROR | MB_OK);
			ret = EXIT_FAILURE;
		}
	}
	else
	{
		ret = EXIT_FAILURE;
	}
	
	slow::closeLogger();
	
	return ret;
}
