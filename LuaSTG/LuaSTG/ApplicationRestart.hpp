#pragma once
#include <vector>
#include <string>

namespace luastg {
	class ApplicationRestart {
	public:
		static void disable();
		static void enableWithCommandLineArguments(std::vector<std::string> const& args);
		static bool hasRestart();
		static void start();
	};
}
