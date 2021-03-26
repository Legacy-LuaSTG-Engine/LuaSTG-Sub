#include "AppFrame.h"
#include "Utility.h"
#include "SteamAPI.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int)
{
	#ifdef LDEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
	#endif
	
	int ret = 0;
	if (LuaSTGPlus::SteamAPI::Init())
	{
		LuaSTGPlus::CoInitializeScope co;
		if (co())
		{
			if (LAPP.Init())
			{
				LAPP.Run();
				LAPP.Shutdown();
				ret = 0;
			}
			else
			{
				MessageBoxW(
					0,
					L"引擎未能成功初始化，查看日志文件可以获得更多信息。",
					L"引擎初始化失败",
					MB_ICONERROR | MB_OK);
				ret = -1;
			}
		}
		else
		{
			MessageBoxW(
				0,
				L"未能正常初始化COM组件库，请尝试重新启动此应用程序。",
				L"引擎初始化失败",
				MB_ICONERROR | MB_OK);
			ret = -1;
		}
		LuaSTGPlus::SteamAPI::Shutdown();
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}
