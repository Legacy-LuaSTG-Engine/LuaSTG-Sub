#pragma once
#include <string>
#include <string_view>

namespace Core
{
	struct InitializeConfigure
	{
		std::string engine_cache_directory;

		void reset();
		bool load(std::string_view const source) noexcept;
		bool save(std::string_view const source, std::string& buffer) noexcept;
		bool loadFromFile(std::string_view const path) noexcept;
		bool saveToFile(std::string_view const path) noexcept;

		static bool parserDirectory(std::string_view const path, std::string& buffer, bool create_directories) noexcept;
		static bool parserFilePath(std::string_view const path, std::string& buffer, bool create_parent_directories) noexcept;
	};
}
