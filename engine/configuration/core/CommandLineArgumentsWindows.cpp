#include "core/CommandLineArguments.hpp"
#include <vector>
#include <string>
#include <mutex>
#include "utf8.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	std::recursive_mutex s_lock;
	std::vector<std::string> s_args;
	constexpr auto s_empty{ ""sv };
}

namespace core {
	void CommandLineArguments::clear() {
		std::lock_guard const lock{ s_lock };
		s_args.clear();
	}
	void CommandLineArguments::add(std::string_view const arg) {
		std::lock_guard const lock{ s_lock };
		s_args.emplace_back(arg);
	}

	std::string_view CommandLineArguments::at(size_t const n) {
		std::lock_guard const lock{ s_lock };
		if (n < s_args.size()) {
			return s_args[n];
		}
		return s_empty;
	}
	size_t CommandLineArguments::size() {
		std::lock_guard const lock{ s_lock };
		return s_args.size();
	}
	std::vector<std::string> CommandLineArguments::copy() {
		std::lock_guard const lock{ s_lock };
		return { s_args };
	}
}

#include <windows.h>

namespace {
	void parse(int const argc, WCHAR const* const* const argv) {
		s_args.resize(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i += 1) {
			s_args[i].assign(utf8::to_string(argv[i]));
		}
	}
}

namespace core {
	void CommandLineArguments::initialize() {
		std::lock_guard const lock{ s_lock };
		int argc{};
		if (const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc); argv != nullptr) {
			parse(argc, argv);
			LocalFree(static_cast<HLOCAL>(argv));
		}
	}
}
