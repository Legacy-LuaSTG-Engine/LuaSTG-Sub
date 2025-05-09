#include "core/Configuration.hpp"
#include <array>
#include <vector>
#include <optional>
#include <format>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
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

	static std::optional<bool> to_boolean(std::string_view const& s) {
		if (s == "true"sv) {
			return { true };
		}
		else if (s == "false"sv) {
			return { false };
		}
		else {
			return std::nullopt;
		}
	}

	template<typename T>
	static std::optional<T> to_unsigned_integer(std::string_view const& s) {
		static_assert(std::is_unsigned<T>::value);
		T value{};
		auto const result = std::from_chars(s.data(), s.data() + s.size(), value);
		if (result.ec == std::errc{}) {
			return { value };
		}
		else {
			return std::nullopt;
		}
	}

	static bool getCommandLineArguments(std::vector<std::string>& args) {
		PWSTR command_line = GetCommandLineW();
		if (!command_line) {
			return false;
		}

		int argc = 0;
		PWSTR* argv = CommandLineToArgvW(command_line, &argc);
		if (!argv) {
			return false;
		}

		args.resize(static_cast<size_t>(argc));
		for (int i = 0; i < argc; i += 1) {
			args[i].assign(to_string(argv[i]));
		}

		LocalFree(argv);
		return true;
	}

	bool ConfigurationLoader::loadFromCommandLineArguments() {
		std::vector<std::string> args;
		if (!getCommandLineArguments(args)) {
			return false;
		}
		auto write_arg_error = [this](std::string_view const& raw_arg) -> void {
			messages.emplace_back(std::format("invalid command line argument '{}'", raw_arg));
		};
		auto write_message = [this](std::string_view const& raw_arg, std::string_view const& message) -> void {
			messages.emplace_back(std::format("invalid command line argument '{}': {}", raw_arg, message));
		};
		for (size_t i = 0; i < args.size(); i += 1) {
			std::string_view const raw_arg(args[i]);
			std::string_view arg(args[i]);

			constexpr auto double_dash = "--"sv;
			constexpr auto separator = "."sv;
			constexpr auto assignment = "="sv;

			if (!arg.starts_with(double_dash)) {
				continue;
			}
			arg = arg.substr(double_dash.size());

		#define access_parent_field(VAR, EXP)			\
			constexpr auto key_##VAR = "" #VAR ""sv;	\
			if (arg.starts_with(key_##VAR)) {			\
				arg = arg.substr(key_##VAR.size());		\
				if (arg.starts_with(separator)) {		\
					arg = arg.substr(separator.size());	\
					EXP;								\
				}										\
			}

		#define access_field(VAR, EXP)					\
			constexpr auto key_##VAR = "" #VAR ""sv;	\
			if (arg.starts_with(key_##VAR)) {			\
				arg = arg.substr(key_##VAR.size());		\
				if (arg.starts_with(assignment)) {		\
					arg = arg.substr(assignment.size());\
					EXP;								\
				}										\
			}

			access_parent_field(graphics_system,
								{
									access_field(preferred_device_name,
										if (!arg.empty()) {
											graphics_system.setPreferredDeviceName(arg);
										});
									access_field(width,
										if (auto const value = to_unsigned_integer<uint32_t>(arg); value) {
											if (value.value() == 0) {
												write_message(raw_arg, "width must greater than 0"sv);
												return false;
											}
											graphics_system.setWidth(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(height,
										if (auto const value = to_unsigned_integer<uint32_t>(arg); value) {
											if (value.value() == 0) {
												write_message(raw_arg, "height must greater than 0"sv);
												return false;
											}
											graphics_system.setHeight(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(fullscreen,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setFullscreen(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(vsync,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setVsync(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(allow_software_device,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setAllowSoftwareDevice(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});

									access_field(allow_exclusive_fullscreen,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setAllowExclusiveFullscreen(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(allow_modern_swap_chain,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setAllowModernSwapChain(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
									access_field(allow_direct_composition,
										if (auto const value = to_boolean(arg); value) {
											graphics_system.setAllowDirectComposition(value.value());
										}
										else {
											write_arg_error(raw_arg);
											return false;
										});
								});

		#undef access_parent_field
		#undef access_field
		}
		return true;
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

		if (str) {
			output.assign(to_string(str));
		}
		CoTaskMemFree(str);
		return true;
	}

	enum class PredefinedVariableType {
		folder,
		temporary_folder,
	};

	struct PredefinedVariable {
		std::string_view mark;
		PredefinedVariableType type;
		std::string_view default_value;
		KNOWNFOLDERID id;
	};

	static bool getTempPathDynamic(std::string& output) {
		std::vector<WCHAR> buffer;
		for (DWORD size = 256; size <= 0xffffu; size *= 2) {
			buffer.resize(256, L'\0');
			DWORD const result = GetTempPathW(size, buffer.data());
			if (result == 0) {
				return false;
			}
			if (result > size) {
				continue;
			}
			output.assign(to_string(std::wstring_view(buffer.data(), result)));
			return true;
		}
		return false;
	}
	static bool getTempPath(std::string& output) {
		constexpr DWORD size = MAX_PATH + 1;
		WCHAR buffer[size]{};
		DWORD const result = GetTempPathW(size, buffer);
		if (result == 0) {
			return false;
		}
		if (result > size) {
			return getTempPathDynamic(output);
		}
		output.assign(to_string(std::wstring_view(buffer, result)));
		return true;
	}
	static auto v = FOLDERID_LocalAppData;

	constexpr GUID guid(
		uint32_t const a, uint16_t const b, uint16_t const c,
		uint8_t const d0, uint8_t const d1, uint8_t const d2, uint8_t const d3, uint8_t const d4, uint8_t const d5, uint8_t const d6, uint8_t const d7
	) {
		return { a, b, c, {d0, d1, d2, d3, d4, d5, d6, d7} };
	}

	// ReSharper disable CommentTypo
	static constexpr std::array predefined_variables{
		PredefinedVariable{
			.mark{"${user:roaming_app_data}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"AppData"sv},
			.id{guid(0x3EB685DB, 0x65F9, 0x4CF6, 0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D)}, // FOLDERID_RoamingAppData
		},
		PredefinedVariable{
			.mark{"${user:local_app_data}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"LocalAppData"sv},
			.id{guid(0xF1B32785, 0x6FBA, 0x4FCF, 0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91)}, // FOLDERID_LocalAppData
		},
		PredefinedVariable{
			.mark{"${user:local_app_data_low}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"LocalAppDataLow"sv},
			.id{guid(0xA520A1A4, 0x1780, 0x4FF6, 0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16)}, // FOLDERID_LocalAppDataLow
		},
		PredefinedVariable{
			.mark{"${user:saved_games}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"SavedGames"sv},
			.id{guid(0x4c5c32ff, 0xbb9d, 0x43b0, 0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4)}, // FOLDERID_SavedGames
		},
		// compatibility
		PredefinedVariable{
			.mark{"${AppData}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"AppData"sv},
			.id{guid(0x3EB685DB, 0x65F9, 0x4CF6, 0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D)},
		},
		PredefinedVariable{
			.mark{"${LocalAppData}"sv},
			.type = PredefinedVariableType::folder,
			.default_value{"LocalAppData"sv},
			.id{guid(0xF1B32785, 0x6FBA, 0x4FCF, 0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91)},
		},
		PredefinedVariable{
			.mark{"${Temp}"sv},
			.type = PredefinedVariableType::temporary_folder,
			.default_value{"Temp"sv},
			.id{},
		},
	};
	// ReSharper restore CommentTypo

	bool ConfigurationLoader::replaceAllPredefinedVariables(std::string_view const& input, std::string& output) {
		std::string buffer(input);
		for (auto const& predefined_variable : predefined_variables) {
			if (auto const i = buffer.find(predefined_variable.mark); i != std::string::npos) {
				if (std::string folder; predefined_variable.type == PredefinedVariableType::temporary_folder && getTempPath(folder)) {
					buffer.replace(i, predefined_variable.mark.length(), folder);
				}
				else if (getKnownDirectory(predefined_variable.id, folder)) {
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
				if (std::string folder; predefined_variable.type == PredefinedVariableType::temporary_folder && getTempPath(folder)) {
					buffer.replace(0, predefined_variable.mark.length(), folder);
				}
				else if (getKnownDirectory(predefined_variable.id, folder)) {
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
			if (std::error_code ec; !std::filesystem::is_directory(path, ec)) {
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
				if (std::string folder; predefined_variable.type == PredefinedVariableType::temporary_folder && getTempPath(folder)) {
					buffer.replace(0, predefined_variable.mark.length(), folder);
				}
				else if (getKnownDirectory(predefined_variable.id, folder)) {
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
			if (std::error_code ec; !std::filesystem::is_directory(parent_path, ec)) {
				std::filesystem::create_directories(parent_path, ec);
			}
		}
		output = path;
		return true;
	}
}
