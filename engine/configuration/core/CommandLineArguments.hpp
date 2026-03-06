#pragma once
#include <string_view>
#include <vector>
#include <string>

namespace core {
	class CommandLineArguments {
	public:
		// initialize

		static void initialize();

		// modify

		static void clear();
		static void add(std::string_view arg);
		static void assign(std::vector<std::string> const& args);

		// access

		static std::string_view at(size_t n);
		static size_t size();
		static std::vector<std::string> copy();
	};
}
