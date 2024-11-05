#include "Core/InitializeConfigure.hpp"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "utf8.hpp"
#include "Platform/KnownDirectory.hpp"

namespace Core
{
	constexpr int const json_indent = 2;

	inline void to_json(nlohmann::json& j, InitializeConfigure const& p)
	{
	#define SET(name) j[#name] = p.##name;

		SET(target_graphics_device);

		SET(canvas_width);
		SET(canvas_height);

		SET(fullscreen_enable);
		SET(vsync_enable);

		SET(log_file_enable);
		SET(log_file_path);
		SET(persistent_log_file_enable);
		SET(persistent_log_file_directory);
		SET(persistent_log_file_max_count);
		SET(engine_cache_directory);

	#undef SET
	}
	inline void from_json(nlohmann::json const& j, InitializeConfigure& p)
	{
	#define GET(name) if (j.contains(#name)) { j.at(#name).get_to(p.##name); }

		GET(target_graphics_device);

		GET(canvas_width);
		GET(canvas_height);

		GET(fullscreen_enable);
		GET(vsync_enable);

		GET(log_file_enable);
		GET(log_file_path);
		GET(persistent_log_file_enable);
		GET(persistent_log_file_directory);
		GET(persistent_log_file_max_count);
		GET(engine_cache_directory);

	#undef GET
	}

	inline bool from_file(nlohmann::json& j, std::string_view const path)
	{
		std::wstring wpath(utf8::to_wstring(path));
		std::error_code ec;
		if (!std::filesystem::is_regular_file(wpath, ec))
		{
			return false;
		}
		std::ifstream file(wpath, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}
		file >> j;
		return true;
	}
	inline bool to_file(nlohmann::json const& j, std::string_view const path)
	{
		std::wstring wpath(utf8::to_wstring(path));
		std::ofstream file(wpath, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file.is_open())
		{
			return false;
		}
		file << std::setw(json_indent) << j;
		return true;
	}

	void InitializeConfigure::reset()
	{
		target_graphics_device.clear();

		canvas_width = 640;
		canvas_height = 480;

		fullscreen_enable = false;
		vsync_enable = false;

		log_file_enable = true;
		log_file_path = "engine.log";
		persistent_log_file_enable = false;
		persistent_log_file_directory = "logs/";
		persistent_log_file_max_count = 100;
		engine_cache_directory.clear();
	}
	bool InitializeConfigure::load(std::string_view const source) noexcept
	{
		try
		{
			nlohmann::json json = nlohmann::json::parse(source);
			from_json(json, *this);
			return true;
		}
		catch (std::exception const&)
		{
			reset();
			return false;
		}
	}
	bool InitializeConfigure::save(std::string_view const source, std::string& buffer) noexcept
	{
		try
		{
			nlohmann::json json = nlohmann::json::parse(source);
			to_json(json, *this);
			buffer = std::move(json.dump(json_indent));
			return true;
		}
		catch (std::exception const&)
		{
			return false;
		}
	}
	bool InitializeConfigure::loadFromFile(std::string_view const path) noexcept
	{
		try
		{
			nlohmann::json json;
			if (!from_file(json, path)) return false;
			from_json(json, *this);
			return true;
		}
		catch (std::exception const&)
		{
			reset();
			return false;
		}
	}
	bool InitializeConfigure::saveToFile(std::string_view const path) noexcept
	{
		try
		{
			nlohmann::json json;
			if (!from_file(json, path)) return false;
			to_json(json, *this);
			if (!to_file(json, path)) return false;
			return true;
		}
		catch (std::exception const&)
		{
			return false;
		}
	}

	inline bool parser_path(std::string_view const path, std::string& buffer)
	{
		std::string_view const mark_app_data("${AppData}");
		std::string_view const mark_local_app_data("${LocalAppData}");
		std::string_view const mark_temp("${Temp}");

		std::string_view path_view(path);
		buffer.clear();

		if (path_view.starts_with(mark_app_data))
		{
			std::string path_buffer;
			Platform::KnownDirectory::getRoamingAppData(path_buffer);
			if (path_buffer.empty())
			{
				path_buffer = "AppData";
			}
			buffer.append(path_buffer);
			path_view = path_view.substr(mark_app_data.size());
		}
		else if (path_view.starts_with(mark_local_app_data))
		{
			std::string path_buffer;
			Platform::KnownDirectory::getLocalAppData(path_buffer);
			if (path_buffer.empty())
			{
				path_buffer = "LocalAppData";
			}
			buffer.append(path_buffer);
			path_view = path_view.substr(mark_local_app_data.size());
		}
		else if (path_view.starts_with(mark_temp))
		{
			std::array<wchar_t, MAX_PATH + 1> wpath_buffer{};
			DWORD const length = GetTempPathW(MAX_PATH + 1, wpath_buffer.data());
			std::wstring_view const wpath_buffer_view(wpath_buffer.data(), length);
			std::string path_buffer(utf8::to_string(wpath_buffer_view));
			if (path_buffer.empty())
			{
				path_buffer = "Temp";
			}
			buffer.append(path_buffer);
			path_view = path_view.substr(mark_temp.size());
		}

		buffer.append(path_view);
		return true;
	}

	bool InitializeConfigure::parserDirectory(std::string_view const path, std::string& buffer__, bool create_directories) noexcept
	{
		std::string buffer;
		if (!parser_path(path, buffer)) return false;

		std::filesystem::path fs_path(utf8::to_wstring(buffer), std::filesystem::path::generic_format);
		if (create_directories)
		{
			std::error_code ec;
			if (!std::filesystem::is_directory(fs_path, ec))
			{
				std::filesystem::create_directories(fs_path, ec);
			}
		}

		buffer__ = utf8::to_string(fs_path.wstring());
		return true;
	}
	bool InitializeConfigure::parserFilePath(std::string_view const path, std::string& buffer__, bool create_parent_directories) noexcept
	{
		std::string buffer;
		if (!parser_path(path, buffer)) return false;

		std::filesystem::path fs_path(utf8::to_wstring(buffer), std::filesystem::path::generic_format);
		if (create_parent_directories && fs_path.has_parent_path())
		{
			std::filesystem::path fs_parent_path(fs_path.parent_path());
			std::error_code ec;
			if (!std::filesystem::is_directory(fs_parent_path, ec))
			{
				std::filesystem::create_directories(fs_parent_path, ec);
			}
		}

		buffer__ = utf8::to_string(fs_path.wstring());
		return true;
	}
}
