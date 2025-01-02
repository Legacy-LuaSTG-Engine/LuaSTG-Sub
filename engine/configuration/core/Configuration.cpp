#include "core/Configuration.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>
#include <ranges>
#include "nlohmann/json.hpp"

#define assert_type_is_boolean(JSON, PATH) \
		if (!((JSON).is_boolean())) { \
			error_callback(std::format("[{}] require boolean type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

#define assert_type_is_unsigned_integer(JSON, PATH) \
		if (!((JSON).is_number_unsigned())) { \
			error_callback(std::format("[{}] require unsigned integer type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

#define assert_type_is_number(JSON, PATH) \
		if (!((JSON).is_number())) { \
			error_callback(std::format("[{}] require number type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

#define assert_type_is_string(JSON, PATH) \
		if (!((JSON).is_string())) { \
			error_callback(std::format("[{}] require string type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

#define assert_type_is_array(JSON, PATH) \
		if (!((JSON).is_array())) { \
			error_callback(std::format("[{}] require array type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

#define assert_type_is_object(JSON, PATH) \
		if (!((JSON).is_object())) { \
			error_callback(std::format("[{}] require object type, but obtain {}"sv, PATH, getTypeName(JSON))); \
			return false; \
		}

using namespace std::string_view_literals;

namespace core {
	static std::u8string_view to_u8string_view(std::string_view const& sv) {
		static_assert(CHAR_BIT == 8 && sizeof(char) == 1 && sizeof(char) == sizeof(char8_t));
		return { reinterpret_cast<char8_t const*>(sv.data()), sv.size() };
	}

	static std::string to_string(std::u8string const& s) {
		static_assert(CHAR_BIT == 8 && sizeof(char) == 1 && sizeof(char) == sizeof(char8_t));
		return { reinterpret_cast<char const*>(s.c_str()), s.length() };
	}

	static bool is_uuid(std::string_view const& uuid) {
	#define is_uuid_char(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
	#define is_not_uuid_char(c) (!is_uuid_char(c))
		constexpr std::string_view format36{ "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"sv };
		switch (uuid.length()) {
		case 32:
			for (auto const c : uuid) {
				if (is_not_uuid_char(c)) {
					return false;
				}
			}
			return true;
		case 36:
			for (size_t i = 0; i < 36; i += 1) {
				auto const c = uuid[i];
				if (format36[i] == '-') {
					if (c != '-') {
						return false;
					}
				}
				else {
					if (is_not_uuid_char(c)) {
						return false;
					}
				}
			}
			return true;
		default:
			return false;
		}
	#undef is_uuid_char
	#undef is_not_uuid_char
	}

	static bool readTextFile(std::string_view const& path, std::string& str) {
		str.clear();
		std::error_code ec;
		std::filesystem::path fs_path(to_u8string_view(path));
		if (!std::filesystem::is_regular_file(fs_path, ec)) {
			return false;
		}
		std::ifstream json_file(fs_path, std::ifstream::in | std::ifstream::binary);
		if (!json_file.is_open()) {
			return false;
		}
		auto const end = json_file.seekg(0, std::ifstream::end).tellg();
		auto const begin = json_file.seekg(0, std::ifstream::beg).tellg();
		if (end == std::streamoff(-1) || begin == std::streamoff(-1) || begin > end) {
			return false;
		}
		auto const size = end - begin;
		str.resize(static_cast<size_t>(size));
		if (!json_file.read(str.data(), str.size())) {
			return false;
		}
		json_file.close();
		return true;
	}

	static std::string_view const getTypeName(nlohmann::json const& json) {
		if (json.is_object()) {
			return "object"sv;
		}
		if (json.is_array()) {
			return "array"sv;
		}
		if (json.is_string()) {
			return "string"sv;
		}
		if (json.is_number()) {
			return "number"sv;
		}
		if (json.is_boolean()) {
			return "boolean"sv;
		}
		if (json.is_null()) {
			return "null"sv;
		}
		if (json.is_binary()) {
			return "binary"sv;
		}
		return "unknown"sv;
	};
}

namespace core {
	class ConfigurationLoaderContext {
	public:
		struct Include {
			std::string path;
			bool optional{ false };
		};
	public:
		static bool merge(ConfigurationLoader& loader, std::vector<Include>* include_out, nlohmann::json const& root) {
			auto& messages = loader.messages;

			auto error_callback = [&](std::string_view const& message) {
				messages.emplace_back(message);
			};

			if (root.contains("include"sv)) {
				auto const& includes = root.at("include"sv);
				assert_type_is_array(includes, "/include"sv);
				if (!include_out) {
					error_callback("[/include] include is not allowed"sv);
					return false;
				}
				for (size_t i = 0; i < includes.size(); i += 1) {
					auto const& include = includes.at(i);
					assert_type_is_object(include, std::format("/include/{}"sv, i));
					Include inc;
					if (include.contains("path"sv)) {
						auto const& path = include.at("path"sv);
						assert_type_is_string(path, std::format("/include/{}/path"sv, i));
						inc.path = path.get_ref<std::string const&>();
					}
					if (include.contains("optional"sv)) {
						auto const& optional = include.at("optional"sv);
						assert_type_is_boolean(optional, std::format("/include/{}/optional"sv, i));
						inc.optional = optional.get<bool>();
					}
					if (include_out) {
						include_out->emplace_back(inc);
					}
				}
			}

			if (root.contains("debug"sv)) {
				auto const& debug = root.at("debug"sv);
				assert_type_is_object(debug, "/debug"sv);
				if (debug.contains("track_window_focus"sv)) {
					auto const& track_window_focus = debug.at("track_window_focus"sv);
					assert_type_is_boolean(track_window_focus, "/debug/track_window_focus"sv);
					loader.debug.setTrackWindowFocus(track_window_focus.get<bool>());
				}
			}

			if (root.contains("application"sv)) {
				auto const& application = root.at("application"sv);
				assert_type_is_object(application, "/application"sv);
				if (application.contains("uuid"sv)) {
					auto const& uuid = application.at("uuid"sv);
					assert_type_is_string(uuid, "/application/uuid"sv);
					auto const& s = uuid.get_ref<std::string const&>();
					if (!is_uuid(s)) {
						error_callback(std::format("[/application/uuid] require uuid string, but obtain '{}'"sv, s));
						return false;
					}
					loader.application.setUuid(uuid);
				}
				if (application.contains("single_instance"sv)) {
					auto const& single_instance = application.at("single_instance"sv);
					assert_type_is_boolean(single_instance, "/application/single_instance"sv);
					loader.application.setSingleInstance(single_instance.get<bool>());
				}
			}

			if (root.contains("logging"sv)) {
				using Level = ConfigurationLoader::Logging::Level;

			#define get_level(PATH) \
				auto const& s = threshold.get_ref<std::string const&>(); \
				Level level{ Level::info }; \
				if (s == "debug"sv) { level = Level::debug; } \
				else if (s == "info"sv) { level = Level::info; } \
				else if (s == "warn"sv) { level = Level::warn; } \
				else if (s == "error"sv) { level = Level::error; } \
				else if (s == "fatal"sv) { level = Level::fatal; } \
				else { error_callback(std::format("[" PATH "] unknown logging level '{}'"sv, s)); return false; }

				auto const& logging = root.at("logging"sv);
				assert_type_is_object(logging, "/logging"sv);
				if (logging.contains("debugger"sv)) {
					auto const& debugger = logging.at("debugger"sv);
					assert_type_is_object(debugger, "/logging/debugger"sv);
					if (debugger.contains("enable"sv)) {
						auto const& enable = debugger.at("enable"sv);
						assert_type_is_boolean(enable, "/logging/debugger/enable"sv);
						loader.logging.debugger.setEnable(enable.get<bool>());
					}
					if (debugger.contains("threshold"sv)) {
						auto const& threshold = debugger.at("threshold"sv);
						assert_type_is_string(threshold, "/logging/debugger/threshold"sv);
						get_level("/logging/debugger/threshold");
						loader.logging.debugger.setThreshold(level);
					}
				}
				if (logging.contains("console"sv)) {
					auto const& console = logging.at("console"sv);
					assert_type_is_object(console, "/logging/console"sv);
					if (console.contains("enable"sv)) {
						auto const& enable = console.at("enable"sv);
						assert_type_is_boolean(enable, "/logging/console/enable"sv);
						loader.logging.console.setEnable(enable.get<bool>());
					}
					if (console.contains("threshold"sv)) {
						auto const& threshold = console.at("threshold"sv);
						assert_type_is_string(threshold, "/logging/console/threshold"sv);
						get_level("/logging/console/threshold");
						loader.logging.console.setThreshold(level);
					}
				}
				if (logging.contains("file"sv)) {
					auto const& file = logging.at("file"sv);
					assert_type_is_object(file, "/logging/file"sv);
					if (file.contains("enable"sv)) {
						auto const& enable = file.at("enable"sv);
						assert_type_is_boolean(enable, "/logging/file/enable"sv);
						loader.logging.file.setEnable(enable.get<bool>());
					}
					if (file.contains("threshold"sv)) {
						auto const& threshold = file.at("threshold"sv);
						assert_type_is_string(threshold, "/logging/file/threshold"sv);
						get_level("/logging/file/threshold");
						loader.logging.file.setThreshold(level);
					}
					if (file.contains("path"sv)) {
						auto const& path = file.at("path"sv);
						assert_type_is_string(path, "/logging/file/path"sv);
						const auto& s = path.get_ref<std::string const&>();
						// TODO: validate file path format
						loader.logging.file.setPath(s);
					}
				}
				if (logging.contains("rolling_file"sv)) {
					auto const& rolling_file = logging.at("rolling_file"sv);
					assert_type_is_object(rolling_file, "/logging/rolling_file"sv);
					if (rolling_file.contains("enable"sv)) {
						auto const& enable = rolling_file.at("enable"sv);
						assert_type_is_boolean(enable, "/logging/rolling_file/enable"sv);
						loader.logging.rolling_file.setEnable(enable.get<bool>());
					}
					if (rolling_file.contains("threshold"sv)) {
						auto const& threshold = rolling_file.at("threshold"sv);
						assert_type_is_string(threshold, "/logging/rolling_file/threshold"sv);
						get_level("/logging/rolling_file/threshold");
						loader.logging.rolling_file.setThreshold(level);
					}
					if (rolling_file.contains("path"sv)) {
						auto const& path = rolling_file.at("path"sv);
						assert_type_is_string(path, "/logging/rolling_file/path"sv);
						const auto& s = path.get_ref<std::string const&>();
						// TODO: validate directory path format
						loader.logging.rolling_file.setPath(s);
					}
					if (rolling_file.contains("max_history"sv)) {
						auto const& max_history = rolling_file.at("max_history"sv);
						assert_type_is_unsigned_integer(max_history, "/logging/rolling_file/max_history"sv);
						auto const u32 = max_history.get<uint32_t>();
						// TODO: validate
						loader.logging.rolling_file.setMaxHistory(u32);
					}
				}

			#undef get_level
			}

			if (root.contains("file_system"sv)) {
				auto const& file_system = root.at("file_system"sv);
				assert_type_is_object(file_system, "/file_system"sv);
				if (file_system.contains("resources"sv)) {
					auto const& resources = file_system.at("resources"sv);
					assert_type_is_array(resources, "/file_system/resources"sv);
					for (size_t i = 0; i < resources.size(); i += 1) {
						auto const& resource = resources.at(i);
						assert_type_is_object(resource, std::format("/file_system/resources/{}"sv, i));
						ConfigurationLoader::FileSystem::ResourceFileSystem res;
						if (resource.contains("name"sv)) {
							auto const& name = resource.at("name"sv);
							assert_type_is_string(name, std::format("/file_system/resources/{}/name"sv, i));
							res.setName(name.get_ref<std::string const&>());
						}
						if (resource.contains("path"sv)) {
							auto const& path = resource.at("path"sv);
							assert_type_is_string(path, std::format("/file_system/resources/{}/path"sv, i));
							res.setPath(path.get_ref<std::string const&>());
						}
						if (resource.contains("type"sv)) {
							auto const& type = resource.at("type"sv);
							assert_type_is_string(type, std::format("/file_system/resources/{}/type"sv, i));
							auto const& s = type.get_ref<std::string const&>();
							using Type = ConfigurationLoader::FileSystem::ResourceFileSystem::Type;
							Type res_type{ Type::directory };
							if (s == "directory"sv) { res_type = Type::directory; }
							else if (s == "archive"sv) { res_type = Type::archive; }
							else { error_callback(std::format("[/file_system/resources/{}/type] unknown resource type '{}'"sv, i, s)); return false; }
							res.setType(res_type);
						}
						loader.file_system.resources.emplace_back(res);
					}
				}
				if (file_system.contains("user"sv)) {
					auto const& user = file_system.at("user"sv);
					assert_type_is_string(user, "/file_system/user"sv);
					loader.file_system.setUser(user.get_ref<std::string const&>());
				}
			}

			if (root.contains("timing"sv)) {
				auto const& timing = root.at("timing"sv);
				assert_type_is_object(timing, "/timing"sv);
				if (timing.contains("frame_rate"sv)) {
					auto const& frame_rate = timing.at("frame_rate"sv);
					assert_type_is_unsigned_integer(frame_rate, "/timing/frame_rate"sv);
					loader.timing.setFrameRate(frame_rate.get<uint32_t>());
				}
			}

			if (root.contains("window"sv)) {
				auto const& window = root.at("window"sv);
				assert_type_is_object(window, "/window"sv);
				if (window.contains("title"sv)) {
					auto const& title = window.at("title"sv);
					assert_type_is_string(title, "/window/title"sv);
					loader.window.setTitle(title.get_ref<std::string const&>());
				}
				if (window.contains("cursor_visible"sv)) {
					auto const& cursor_visible = window.at("cursor_visible"sv);
					assert_type_is_boolean(cursor_visible, "/window/cursor_visible"sv);
					loader.window.setCursorVisible(cursor_visible.get<bool>());
				}
				if (window.contains("allow_window_corner"sv)) {
					auto const& allow_window_corner = window.at("allow_window_corner"sv);
					assert_type_is_boolean(allow_window_corner, "/window/allow_window_corner"sv);
					loader.window.setAllowWindowCorner(allow_window_corner.get<bool>());
				}
				if (window.contains("allow_title_bar_auto_hide"sv)) {
					auto const& allow_title_bar_auto_hide = window.at("allow_title_bar_auto_hide"sv);
					assert_type_is_boolean(allow_title_bar_auto_hide, "/window/allow_title_bar_auto_hide"sv);
					loader.window.setAllowTitleBarAutoHide(allow_title_bar_auto_hide.get<bool>());
				}
			}

			if (root.contains("graphics_system"sv)) {
				auto const& graphics_system = root.at("graphics_system"sv);
				assert_type_is_object(graphics_system, "/graphics_system"sv);
				if (graphics_system.contains("preferred_device_name"sv)) {
					auto const& preferred_device_name = graphics_system.at("preferred_device_name"sv);
					assert_type_is_string(preferred_device_name, "/graphics_system/preferred_device_name"sv);
					loader.graphics_system.setPreferredDeviceName(preferred_device_name.get_ref<std::string const&>());
				}
				if (graphics_system.contains("width"sv)) {
					auto const& width = graphics_system.at("width"sv);
					assert_type_is_unsigned_integer(width, "/graphics_system/width"sv);
					auto const v = width.get<uint32_t>();
					if (width == 0) {
						error_callback("[/graphics_system/width] width must be greater than zero"sv);
						return false;
					}
					loader.graphics_system.setWidth(v);
				}
				if (graphics_system.contains("height"sv)) {
					auto const& height = graphics_system.at("height"sv);
					assert_type_is_unsigned_integer(height, "/graphics_system/height"sv);
					auto const v = height.get<uint32_t>();
					if (height == 0) {
						error_callback("[/graphics_system/height] width must be greater than zero"sv);
						return false;
					}
					loader.graphics_system.setHeight(v);
				}
				if (graphics_system.contains("fullscreen"sv)) {
					auto const& fullscreen = graphics_system.at("fullscreen"sv);
					assert_type_is_boolean(fullscreen, "/graphics_system/fullscreen"sv);
					loader.graphics_system.setFullscreen(fullscreen.get<bool>());
				}
				if (graphics_system.contains("vsync"sv)) {
					auto const& vsync = graphics_system.at("vsync"sv);
					assert_type_is_boolean(vsync, "/graphics_system/vsync"sv);
					loader.graphics_system.setVsync(vsync.get<bool>());
				}
				if (graphics_system.contains("allow_software_device"sv)) {
					auto const& allow_software_device = graphics_system.at("allow_software_device"sv);
					assert_type_is_boolean(allow_software_device, "/graphics_system/allow_software_device"sv);
					loader.graphics_system.setAllowSoftwareDevice(allow_software_device.get<bool>());
				}
				if (graphics_system.contains("allow_exclusive_fullscreen"sv)) {
					auto const& allow_exclusive_fullscreen = graphics_system.at("allow_exclusive_fullscreen"sv);
					assert_type_is_boolean(allow_exclusive_fullscreen, "/graphics_system/allow_exclusive_fullscreen"sv);
					loader.graphics_system.setAllowExclusiveFullscreen(allow_exclusive_fullscreen.get<bool>());
				}
				if (graphics_system.contains("allow_modern_swap_chain"sv)) {
					auto const& allow_modern_swap_chain = graphics_system.at("allow_modern_swap_chain"sv);
					assert_type_is_boolean(allow_modern_swap_chain, "/graphics_system/allow_modern_swap_chain"sv);
					loader.graphics_system.setAllowModernSwapChain(allow_modern_swap_chain.get<bool>());
				}
				if (graphics_system.contains("allow_direct_composition"sv)) {
					auto const& allow_direct_composition = graphics_system.at("allow_direct_composition"sv);
					assert_type_is_boolean(allow_direct_composition, "/graphics_system/allow_direct_composition"sv);
					loader.graphics_system.setAllowDirectComposition(allow_direct_composition.get<bool>());
				}
				// TODO: display
			}

			if (root.contains("audio_system"sv)) {
				auto const& audio_system = root.at("audio_system"sv);
				assert_type_is_object(audio_system, "/audio_system"sv);
				if (audio_system.contains("preferred_endpoint_name"sv)) {
					auto const& preferred_endpoint_name = audio_system.at("preferred_endpoint_name"sv);
					assert_type_is_string(preferred_endpoint_name, "/audio_system/preferred_endpoint_name"sv);
					loader.audio_system.setPreferredEndpointName(preferred_endpoint_name.get_ref<std::string const&>());
				}
				if (audio_system.contains("sound_effect_volume"sv)) {
					auto const& sound_effect_volume = audio_system.at("sound_effect_volume"sv);
					assert_type_is_number(sound_effect_volume, "/audio_system/sound_effect_volume"sv);
					auto const v = sound_effect_volume.get<float>();
					if (v < 0.0f || v > 1.0f) {
						error_callback("[/audio_system/sound_effect_volume] out of range [0.0, 1.0]"sv);
						return false;
					}
					loader.audio_system.setSoundEffectVolume(v);
				}
				if (audio_system.contains("music_volume"sv)) {
					auto const& music_volume = audio_system.at("music_volume"sv);
					assert_type_is_number(music_volume, "/audio_system/music_volume"sv);
					auto const v = music_volume.get<float>();
					if (v < 0.0f || v > 1.0f) {
						error_callback("[/audio_system/music_volume] out of range [0.0, 1.0]"sv);
						return false;
					}
					loader.audio_system.setMusicVolume(v);
				}
			}

			return true;
		}
		static bool load(ConfigurationLoader& loader, std::vector<Include>* include_out, std::string_view const& path) {
			std::string json_text;
			if (!readTextFile(path, json_text)) {
				return false;
			}

			auto const root = nlohmann::json::parse(json_text, nullptr, false);
			if (root.is_discarded()) {
				return false;
			}

			return merge(loader, include_out, root);
		}
	};

	ConfigurationLoader::ConfigurationLoader() {
		logging.debugger.setEnable(true);
		logging.file.setEnable(true);
	}

	bool ConfigurationLoader::loadFromFile(std::string_view const& path) {
		// load

		std::vector<ConfigurationLoaderContext::Include> includes;
		if (!ConfigurationLoaderContext::load(*this, &includes, path)) {
			return false;
		}
		for (auto const& include : includes) {
			std::filesystem::path fs_path;
			if (!resolveFilePathWithPredefinedVariables(include.path, fs_path)) {
				if (include.optional) {
					continue;
				}
				else {
					messages.emplace_back(std::format("resolve '{}' failed", include.path));
					return false;
				}
			}
			auto const resolved_path = to_string(fs_path.lexically_normal().generic_u8string());
			if (!exists(resolved_path)) {
				if (include.optional) {
					continue;
				}
				else {
					messages.emplace_back(std::format("'{}' not found", include.path));
					return false;
				}
			}
			if (!ConfigurationLoaderContext::load(*this, nullptr, resolved_path)) {
				return false;
			}
		}

		// check

		if (application.isSingleInstance() && !application.hasUuid()) {
			messages.emplace_back(std::format("[{}] single_instance require uuid string to be set", path));
			return false;
		}

		return true;
	}
	
	std::string ConfigurationLoader::getFormattedMessage() {
		std::string message;
		if (!messages.empty()) {
			for (auto const& s : messages) {
				message.append(s);
				message.push_back('\n');
			}
			if (message.ends_with('\n')) {
				message.pop_back();
			}
		}
		return message;
	}

	bool ConfigurationLoader::exists(std::string_view const& path) {
		std::error_code ec;
		std::filesystem::path fs_path(to_u8string_view(path));
		return std::filesystem::is_regular_file(fs_path, ec);
	}

	ConfigurationLoader& ConfigurationLoader::getInstance() {
		static ConfigurationLoader instance;
		return instance;
	}
}
