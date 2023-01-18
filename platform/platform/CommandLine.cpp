#include "CommandLine.hpp"

namespace Platform
{
	std::vector<std::string> CommandLine::get()
	{
		std::vector<std::string> args;
		if (LPWSTR pCmdLine = GetCommandLineW())
		{
			int argc = 0;
			if (LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc))
			{
				args.reserve(argc);
				for (int i = 0; i < argc; i += 1)
				{
					args.emplace_back(to_utf8(argv[i]));
				}
				LocalFree(argv);
			}
		}
		return args;
	}
}
