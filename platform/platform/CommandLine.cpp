#include "CommandLine.hpp"

namespace platform
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
					std::wstring_view argw(argv[i]);
					int const size = WideCharToMultiByte(CP_UTF8, 0, argw.data(), (int)argw.length(), NULL, 0, NULL, NULL);
					if (size > 0)
					{
						std::string buf(size, '\0');
						int const result = WideCharToMultiByte(CP_UTF8, 0, argw.data(), (int)argw.length(), buf.data(), size, NULL, NULL);
						if (size > 0)
						{
							args.emplace_back(buf);
						}
					}
				}
				LocalFree(argv);
			}
		}
		return args;
	}
}
