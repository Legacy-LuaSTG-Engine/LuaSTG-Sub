#pragma once
#include <vector>
#include <string>

namespace platform
{
	class CommandLine
	{
	public:
		static std::vector<std::string> get();
	};
}
