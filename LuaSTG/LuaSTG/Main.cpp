#include "Platform/MessageBox.hpp"
#include "Debugger/Logger.hpp"
#include "SteamAPI/SteamAPI.hpp"
#include "Utility/Utility.h"
#include "AppFrame.h"
#include "RuntimeCheck.hpp"

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	if (!LuaSTG::CheckUserRuntime())
	{
		return EXIT_FAILURE;
	}

	LuaSTGPlus::CoInitializeScope com_runtime;
	if (!com_runtime())
	{
		Platform::MessageBox::Error(LUASTG_INFO,
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
			Platform::MessageBox::Error(LUASTG_INFO,
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

#include "Platform/CleanWindows.hpp"

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	return main();
}
