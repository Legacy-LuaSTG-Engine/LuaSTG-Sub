#pragma once
#include <vector>
#include <string>

namespace Platform
{
	class CommandLine
	{
	public:
		static std::vector<std::string> get();
	};
}
