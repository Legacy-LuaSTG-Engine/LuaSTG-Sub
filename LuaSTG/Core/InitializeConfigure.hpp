#pragma once
#include <string>
#include <string_view>

namespace Core
{
	struct InitializeConfigure
	{
		std::string target_graphics_device;

		int canvas_width = 640;
		int canvas_height = 480;
		bool fullscreen_enable = false;
		bool vsync_enable = false;

		bool log_file_enable = true;
		std::string log_file_path = "engine.log";
		bool persistent_log_file_enable = false;
		std::string persistent_log_file_directory = "logs/";
		int persistent_log_file_max_count = 100;
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
