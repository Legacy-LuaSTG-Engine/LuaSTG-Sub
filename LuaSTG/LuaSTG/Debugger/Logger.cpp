#include "Debugger/Logger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/wincolor_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "Platform/HResultChecker.hpp"
#include "core/Configuration.hpp"
#include "utf8.hpp"

namespace {
	std::filesystem::path rolling_file_root;
	bool openWin32Console();
	void closeWin32Console();
	std::string generateRollingFileName();
	spdlog::level::level_enum mapLevel(core::ConfigurationLoader::Logging::Level const level) {
		switch (level) {
		default:  // NOLINT(clang-diagnostic-covered-switch-default)
			return (spdlog::level::trace);
		case core::ConfigurationLoader::Logging::Level::debug:
			return (spdlog::level::debug);
		case core::ConfigurationLoader::Logging::Level::info:
			return (spdlog::level::info);
		case core::ConfigurationLoader::Logging::Level::warn:
			return (spdlog::level::warn);
		case core::ConfigurationLoader::Logging::Level::error:
			return (spdlog::level::err);
		case core::ConfigurationLoader::Logging::Level::fatal:
			return (spdlog::level::critical);
		}
	}
	void writeMessage(std::string_view const message) {
		spdlog::error("[luastg] {}", message);
	}
}

namespace luastg {
	void Logger::create() {
		auto const& config = core::ConfigurationLoader::getInstance().getLogging();

		std::vector<spdlog::sink_ptr> sinks;

	#ifndef NDEBUG
		if (auto const& logging_debugger = config.getDebugger(); logging_debugger.isEnable()) {
			auto sink = std::make_shared<spdlog::sinks::windebug_sink_mt>();
			sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
			sink->set_level(mapLevel(logging_debugger.getThreshold()));
			sinks.emplace_back(sink);
		}
	#endif

	#ifdef USING_CONSOLE_OUTPUT
		if (auto const& logging_console = config.getConsole(); logging_console.isEnable() && openWin32Console()) {
			auto sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
			sink->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
			sink->set_level(mapLevel(logging_console.getThreshold()));
			sinks.emplace_back(sink);
		}
	#endif

		if (auto const& logging_file = config.getFile(); logging_file.isEnable()) {
			std::filesystem::path file_path;
			if (logging_file.hasPath()) {
				core::ConfigurationLoader::resolveFilePathWithPredefinedVariables(logging_file.getPath(), file_path, true);
			}
			else {
				file_path = (L"" LUASTG_LOG_FILE);
			}
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path.generic_wstring(), true);
			sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
			sink->set_level(mapLevel(logging_file.getThreshold()));
			sinks.emplace_back(sink);
		}

		if (auto const& logging_rolling_file = config.getRollingFile(); logging_rolling_file.isEnable()) {
			std::filesystem::path path;
			if (logging_rolling_file.hasPath()) {
				std::filesystem::path directory;
				core::ConfigurationLoader::resolvePathWithPredefinedVariables(logging_rolling_file.getPath(), directory, true);
				path = directory / utf8::to_wstring(generateRollingFileName());
				rolling_file_root = directory;
			}
			else {
				std::filesystem::path directory(L"logs/"); // TODO: MAGIC VALUE
				path = directory / utf8::to_wstring(generateRollingFileName());
				rolling_file_root = directory;
			}
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.wstring(), true);
			sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
			sink->set_level(mapLevel(logging_rolling_file.getThreshold()));
			sinks.emplace_back(sink);
		}

		auto const logger = std::make_shared<spdlog::logger>("luastg", sinks.begin(), sinks.end());
		logger->set_level(spdlog::level::trace);
		//logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%L] %v");
		//logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L]%$ %v");
		//logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%L] [%n]%$ %v");
		logger->flush_on(spdlog::level::info);

		spdlog::set_default_logger(logger);

		Platform::HResultChecker::SetPrintCallback(&writeMessage);
	}
	void Logger::destroy() {
		auto const& config = core::ConfigurationLoader::getInstance().getLogging();

		if (auto const logger = spdlog::get("luastg")) {
			logger->flush();
		}

		Platform::HResultChecker::SetPrintCallback();

		spdlog::drop_all();
		spdlog::shutdown();

	#ifdef USING_CONSOLE_OUTPUT
		closeWin32Console();
	#endif

		if (auto const& logging_rolling_file = config.getRollingFile(); logging_rolling_file.isEnable()) {
			std::error_code ec;
			std::vector<std::filesystem::path> logs;
			for (auto& it : std::filesystem::directory_iterator(rolling_file_root, ec)) {
				if (it.is_regular_file(ec) && it.path().extension() == u8".log") {
					logs.push_back(it.path());
				}
			}
			if (logs.size() > logging_rolling_file.getMaxHistory()) {
				std::sort(logs.begin(), logs.end(), [](std::filesystem::path const& a, std::filesystem::path const& b) -> bool {
					return a.filename().generic_u8string() < b.filename().generic_u8string();
				});
				size_t const remove_count = logs.size() - logging_rolling_file.getMaxHistory();
				for (size_t idx = 0; idx < remove_count; idx += 1) {
					std::filesystem::remove(logs[idx], ec);
				}
			}
		}
	}
};

#include "Platform/CleanWindows.hpp"

namespace {
	bool g_alloc_console{ false };
	bool openWin32Console() {
		auto const& logging_console = core::ConfigurationLoader::getInstance().getLogging().getConsole();
		if (!logging_console.isEnable()) {
			return false;
		}
		if (!AllocConsole()) {
			return false;
		}
		g_alloc_console = true;
		HWND const window = GetConsoleWindow();
		HMENU const menu = GetSystemMenu(window, FALSE);
		RemoveMenu(menu, SC_CLOSE, MF_BYCOMMAND);
		SetWindowTextW(window, L"" LUASTG_INFO);
		ShowWindow(window, SW_MAXIMIZE);
		SetConsoleOutputCP(CP_UTF8);
		return true;
	}
	void closeWin32Console() {
		auto const& logging_console = core::ConfigurationLoader::getInstance().getLogging().getConsole();
		if (g_alloc_console) {
			if (logging_console.isPreserve()) {
				constexpr std::wstring_view exit_message(L"按 ESC 关闭引擎日志窗口 | Press ESC to close the engine log window\n");
				WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), exit_message.data(), static_cast<DWORD>(exit_message.length()), nullptr, nullptr);
				while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0x8000) {
					Sleep(1);
				}
			}
			FreeConsole();
		}
	}
	std::string generateRollingFileName() {
		SYSTEMTIME current_time{};
		GetLocalTime(&current_time);

		std::array<char, 32> buffer{};
		int const length = std::snprintf(
			buffer.data(), buffer.size(),
			"%04u-%02u-%02u-%02u-%02u-%02u-%03u",
			current_time.wYear,
			current_time.wMonth,
			current_time.wDay,
			current_time.wHour,
			current_time.wMinute,
			current_time.wSecond,
			current_time.wMilliseconds);
		assert(length > 0);

		std::string path;
		path.append("engine-");
		path.append(std::string_view(buffer.data(), static_cast<size_t>(length)));
		path.append(".log");
		return path;
	}
}
