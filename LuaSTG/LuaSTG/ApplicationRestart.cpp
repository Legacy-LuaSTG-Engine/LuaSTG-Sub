#include "ApplicationRestart.hpp"
#include <atomic>
#include "core/Logger.hpp"
#include "core/CommandLineArguments.hpp"
#include "utf8.hpp"
#include <windows.h>

namespace {
	std::atomic_bool s_restart;
	std::wstring getModuleFileName() {
		std::vector<WCHAR> buffer;
		for (size_t i = 256; i <= 65536; i *= 2) {
			buffer.resize(std::max<size_t>(MAX_PATH, i));
			std::ranges::fill(buffer, L'\0');
			SetLastError(0);
			const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
			if (length == 0) {
				return {};
			}
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				continue;
			}
			return { buffer.data(), length };
		}
		return {};
	}
}

namespace luastg {
	void ApplicationRestart::disable() {
		s_restart.store(false);
	}
	void ApplicationRestart::enableWithCommandLineArguments(std::vector<std::string> const& args) {
		s_restart.store(true);
		core::CommandLineArguments::assign(args);
	}
	bool ApplicationRestart::hasRestart() {
		return s_restart.load();
	}
	void ApplicationRestart::start() {
		if (!s_restart.load()) {
			return;
		}

		auto const path{ getModuleFileName() };
		if (path.empty()) {
			return;
		}

		std::wstring cmd;
		cmd.push_back('\"');
		cmd.append(path);
		cmd.push_back('\"');
		cmd.push_back(' ');

		auto const args{ core::CommandLineArguments::copy() };
		for (size_t i = 1 /* skip executable path */; i < args.size(); i += 1) {
			cmd.push_back(L'\"');
			cmd.append(utf8::to_wstring(args[i])); // TODO: FUCK MICROSOFT
			cmd.push_back(L'\"');
			cmd.push_back(L' ');
		}

		STARTUPINFOW info{};
		info.cb = sizeof(info);
		PROCESS_INFORMATION output{};

		const BOOL result = CreateProcessW(
			path.c_str(), // module
			cmd.data(), // command line
			nullptr, // process attr
			nullptr, // thread attr
			FALSE, // inherit handles
			0, // creation flags
			nullptr, // environment
			nullptr, // current directory
			&info,
			&output
		);
		if (!result) {
			core::Logger::error("[luastg::ApplicationRestart] start failed: CreateProcessW failed");
		}

		if (output.hProcess != nullptr) {
			CloseHandle(output.hProcess);
		}
		if (output.hThread != nullptr) {
			CloseHandle(output.hThread);
		}
	}
}
