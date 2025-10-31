#pragma once
#include <string_view>

namespace core {
	class ShellIntegration {
	public:
		static bool openFile(std::string_view path);
		static bool openDirectory(std::string_view path);
		static bool openUrl(std::string_view url);
	};
}
