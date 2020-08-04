#include "AppFrame.h"
#include "RuntimeCheck.hpp"
#include "Utility.h"
#include "SteamAPI.hpp"

using namespace std;
using namespace LuaSTGPlus;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
#ifdef LDEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	int ret = 0;
	if (SteamAPI_Init()) {
		CoScope co;
		if (co()) {
			if (CheckRuntime()) {
				if (LAPP.Init()) {
					LAPP.Run();
					LAPP.Shutdown();
					ret = 0;
				}
				else {
					MessageBoxW(
						0,
						L"引擎未能成功初始化，查看日志文件可以获得更多信息。",
						L"引擎初始化失败",
						MB_ICONERROR | MB_OK);
					ret = -1;
				}
			}
			else {
				MessageBoxW(
					0,
					L"检测到运行库缺失，请前往微软官网下载安装 DirectX End-User Runtime，\n"
					L"和 Microsoft Visual C++ Redistributable for Visual Studio 2019 x86。",
					L"引擎初始化失败",
					MB_ICONERROR | MB_OK);
				ret = -1;
			}
		}
		else {
			MessageBoxW(
				0,
				L"未能正常初始化COM组件库，请尝试重新启动此应用程序。",
				L"引擎初始化失败",
				MB_ICONERROR | MB_OK);
			ret = -1;
		}
		SteamAPI_Shutdown();
	}
	else {
		ret = 1;
	}

	return ret;
}
