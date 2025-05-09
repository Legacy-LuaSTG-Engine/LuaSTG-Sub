#include "Main.h"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include "Platform/MessageBox.hpp"
#include "Platform/ApplicationSingleInstance.hpp"
#include "Debugger/Logger.hpp"
#include "SteamAPI/SteamAPI.hpp"
#include "Utility/Utility.h"
#include "AppFrame.h"
#include "RuntimeCheck.hpp"
#include "core/Configuration.hpp"
#include <chrono>

int luastg::main() {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif
	auto const t1 = std::chrono::high_resolution_clock::now();

	// STAGE 1: initialize COM

	CoInitializeScope com_runtime;
	if (!com_runtime()) {
		Platform::MessageBox::Error(LUASTG_INFO,
			"引擎初始化失败。\n"
			"未能正常初始化COM组件库，请尝试重新启动此应用程序。");
		return EXIT_FAILURE;
	}

	// STAGE 2: load application configurations

	auto& config_loader = core::ConfigurationLoader::getInstance();
	if (core::ConfigurationLoader::exists(LUASTG_CONFIGURATION_FILE) && !config_loader.loadFromFile(LUASTG_CONFIGURATION_FILE)) {
		Platform::MessageBox::Error(LUASTG_INFO, config_loader.getFormattedMessage());
		return EXIT_FAILURE;
	}
	config_loader.loadFromCommandLineArguments();

	// STAGE 3: configure single instance

	Platform::ApplicationSingleInstance single_instance(LUASTG_INFO);
	if (auto const& config_app = config_loader.getApplication(); config_app.isSingleInstance()) {
		single_instance.Initialize(config_app.getUuid());
	}

	// STAGE 4: check runtime

	if (!checkEngineRuntimeRequirement()) {
		return EXIT_FAILURE;
	}

	// STAGE 5: start

	Logger::create();

	auto const t2 = std::chrono::high_resolution_clock::now();
	spdlog::info("Duration before logging system: {}s", double((t2 - t1).count()) / 1000000000.0);

	int result = EXIT_SUCCESS;
	if (SteamAPI::Init())
	{
		if (LAPP.Init())
		{
			auto const t3 = std::chrono::high_resolution_clock::now();
			spdlog::info("Duration of initialization: {}s", double((t3 - t2).count()) / 1000000000.0);

			LAPP.Run();
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
		LAPP.Shutdown();
		SteamAPI::Shutdown();
	}
	else
	{
		result = EXIT_FAILURE;
	}

	Logger::destroy();

#ifndef NDEBUG
	core::implement::ReferenceCountedDebugger::reportLeak();
#endif

	return result;
}
