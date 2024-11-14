#include "core/Configuration.hpp"
#include <array>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shobjidl.h>
#include <Knownfolders.h>
#include <wrl/client.h>

using namespace std::string_view_literals;

namespace core {
	static std::string to_string(std::wstring_view const& wstr) {
		std::string str;
		int const length = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
		if (length <= 0) return str;
		str.resize(static_cast<size_t>(length));
		int const result = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), str.data(), length, nullptr, nullptr);
		if (length != result) str.clear();
		return str;
	}

	static std::u8string_view to_u8string_view(std::string const& sv) {
		static_assert(CHAR_BIT == 8 && sizeof(char) == 1 && sizeof(char) == sizeof(char8_t));
		return { reinterpret_cast<char8_t const*>(sv.c_str()), sv.length() };
	}

	static bool getKnownDirectory(KNOWNFOLDERID const& id, std::string& output) {
		HRESULT hr{};

		Microsoft::WRL::ComPtr<IKnownFolderManager> manager;
		hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager));
		if (FAILED(hr)) return false;

		Microsoft::WRL::ComPtr<IKnownFolder> folder;
		hr = manager->GetFolder(id, &folder);
		if (FAILED(hr)) return false;

		PWSTR str{};
		hr = folder->GetPath(0, &str);
		if (FAILED(hr)) return false;

		output.assign(to_string(str));
		CoTaskMemFree(str);
		return true;
	}

	struct PredefinedVariables {
		std::string_view mark;
		std::string_view default_value;
		KNOWNFOLDERID id;
	};

	static constexpr std::array<PredefinedVariables, 4> predefined_variables{
		PredefinedVariables{"${user:roaming_app_data}"sv, "AppData"sv, /* FOLDERID_RoamingAppData */ KNOWNFOLDERID{0x3EB685DB, 0x65F9, 0x4CF6, {0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D}}},
		PredefinedVariables{"${user:local_app_data}"sv, "LocalAppData"sv, /* FOLDERID_LocalAppData */ KNOWNFOLDERID{0xF1B32785, 0x6FBA, 0x4FCF, {0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91}}},
		PredefinedVariables{"${user:local_app_data_low}"sv, "LocalAppDataLow"sv, /* FOLDERID_LocalAppDataLow */ KNOWNFOLDERID{0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16}}},
		PredefinedVariables{"${user:saved_games}"sv, "SavedGames"sv, /* FOLDERID_SavedGames */ KNOWNFOLDERID{0x4c5c32ff, 0xbb9d, 0x43b0, {0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4}}},
	};

	bool ConfigurationLoader::replaceAllPredefinedVariables(std::string_view const& input, std::string& output) {
		std::string buffer(input);
		for (auto const& predefined_variable : predefined_variables) {
			if (auto const i = buffer.find(predefined_variable.mark); i != std::string::npos) {
				std::string folder;
				if (getKnownDirectory(predefined_variable.id, folder)) {
					buffer.replace(i, predefined_variable.mark.length(), folder);
				}
				else {
					buffer.replace(i, predefined_variable.mark.length(), predefined_variable.default_value);
				}
			}
		}
		output.assign(std::move(buffer));
		return true;
	}

	bool ConfigurationLoader::resolvePathWithPredefinedVariables(std::string_view const& input, std::filesystem::path& output, bool const create_directories) {
		std::string buffer(input);
		for (auto const& predefined_variable : predefined_variables) {
			if (buffer.starts_with(predefined_variable.mark)) {
				std::string folder;
				if (getKnownDirectory(predefined_variable.id, folder)) {
					buffer.replace(0, predefined_variable.mark.length(), folder);
				}
				else {
					buffer.replace(0, predefined_variable.mark.length(), predefined_variable.default_value);
				}
				break;
			}
		}
		std::filesystem::path path(to_u8string_view(buffer));
		path = path.lexically_normal();
		if (create_directories) {
			std::error_code ec;
			if (!std::filesystem::is_directory(path, ec)) {
				std::filesystem::create_directories(path, ec);
			}
		}
		output = path;
		return true;
	}

	bool ConfigurationLoader::resolveFilePathWithPredefinedVariables(std::string_view const& input, std::filesystem::path& output, bool const create_parent_directories) {
		std::string buffer(input);
		for (auto const& predefined_variable : predefined_variables) {
			if (buffer.starts_with(predefined_variable.mark)) {
				std::string folder;
				if (getKnownDirectory(predefined_variable.id, folder)) {
					buffer.replace(0, predefined_variable.mark.length(), folder);
				}
				else {
					buffer.replace(0, predefined_variable.mark.length(), predefined_variable.default_value);
				}
				break;
			}
		}
		std::filesystem::path path(to_u8string_view(buffer));
		path = path.lexically_normal();
		if (create_parent_directories && path.has_parent_path()) {
			auto const parent_path = path.parent_path();
			std::error_code ec;
			if (!std::filesystem::is_directory(parent_path, ec)) {
				std::filesystem::create_directories(parent_path, ec);
			}
		}
		output = path;
		return true;
	}

	/*
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
	//*/
}
