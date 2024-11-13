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

	static void defaultErrorCallback(std::string_view const& message) {
		std::cout << message << std::endl;
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

	void Configuration::setErrorCallback(std::function<void(std::string_view const&)> const& cb) {
		error_callback = cb;
	}

	bool Configuration::loadFromFile(std::string_view const& path) {
		std::string json_text;
		if (!readTextFile(path, json_text)) {
			return false;
		}

		auto const root = nlohmann::json::parse(json_text, nullptr, false);
		if (root.is_discarded()) {
			return false;
		}

		// legacy: compatible
		// TODO: remove

		if (root.contains("single_application_instance"sv)) {
			auto const& v = root.at("single_application_instance"sv);
			assert_type_is_boolean(v, "/single_application_instance"sv);
			if (!application.has_value()) {
				application.emplace();
			}
			application.value().single_instance.emplace(v.get<bool>());
		}
		if (root.contains("application_instance_id"sv)) {
			auto const& v = root.at("application_instance_id"sv);
			assert_type_is_string(v, "/application_instance_id"sv);
			if (!application.has_value()) {
				application.emplace();
			}
			application.value().uuid.emplace(v.get_ref<std::string const&>());
		}

		if (root.contains("debug_track_window_focus"sv)) {
			auto const& v = root.at("debug_track_window_focus"sv);
			assert_type_is_boolean(v, "/debug_track_window_focus"sv);
			if (!debug.has_value()) {
				debug.emplace();
			}
			debug.value().track_window_focus.emplace(v.get<bool>());
		}

		// include

		if (root.contains("include"sv)) {
			if (!allow_include) {
				error_callback("include is not allowed"sv);
				return false;
			}
			auto const& root_include = root.at("include"sv);
			assert_type_is_array(root_include, "/include"sv);
			include.reserve(root_include.size());
			for (size_t i = 0; i < root_include.size(); i += 1) {
				auto const& v = root_include[i];
				assert_type_is_object(v, std::format("/include/{}"sv, i));
				Include info;
				if (v.contains("path"sv)) {
					auto const& v_path = v.at("path"sv);
					assert_type_is_string(v_path, std::format("/include/{}/path"sv, i));
					info.path = v_path.get_ref<std::string const&>();
				}
				if (v.contains("optional"sv)) {
					auto const& v_optional = v.at("optional"sv);
					assert_type_is_boolean(v_optional, std::format("/include/{}/optional"sv, i));
					info.optional = (v_optional.get<bool>());
				}
				include.emplace_back(info);
			}
		}

		// debug

		if (root.contains("debug"sv)) {
			auto const& conf_debug = root.at("debug"sv);
			assert_type_is_object(conf_debug, "/debug"sv);
			auto& self_debug = debug.emplace();
			if (conf_debug.contains("track_window_focus"sv)) {
				auto const& v = conf_debug.at("track_window_focus"sv);
				assert_type_is_boolean(v, "/debug/track_window_focus"sv);
				self_debug.track_window_focus.emplace(v.get<bool>());
			}
		}

		// application

		if (root.contains("application"sv)) {
			auto const& root_app = root.at("application"sv);
			assert_type_is_object(root_app, "/application"sv);
			auto& self_app = application.emplace();

			if (root_app.contains("uuid"sv)) {
				auto const& app_uuid = root_app.at("uuid"sv);
				assert_type_is_string(app_uuid, "/application/uuid"sv);
				auto const& uuid = self_app.uuid.emplace(app_uuid.get_ref<std::string const&>());
				if (!is_uuid(uuid)) {
					error_callback(std::format("[/application/uuid] require uuid string, but obtain '{}'"sv, uuid));
					return false;
				}
			}
			if (root_app.contains("single_instance"sv)) {
				auto const& app_single_instance = root_app.at("single_instance"sv);
				assert_type_is_boolean(app_single_instance, "/application/single_instance"sv);
				[[maybe_unused]] auto const single_instance = self_app.single_instance.emplace(app_single_instance.get<bool>());
			}
		}

		// initialize

		if (root.contains("initialize"sv)) {
			assert_type_is_object(root.at("initialize"sv), "/initialize"sv);
			auto const& root_initialize = root.at("initialize"sv);
			initialize.emplace();

			// file system

			if (root_initialize.contains("file_systems"sv)) {
				assert_type_is_array(root_initialize.at("file_systems"sv), "/initialize/file_systems");
				auto const& init_fs = root_initialize.at("file_systems");
				auto& self_fs = initialize.value().file_systems;
				for (size_t index = 0; index < init_fs.size(); index += 1) {
					auto const& file_system = init_fs[index];
					assert_type_is_object(file_system, std::format("/initialize/file_systems/{}"sv, index));
					FileSystem info;
					if (file_system.contains("type"sv)) {
						assert_type_is_string(file_system.at("type"sv), std::format("/initialize/file_systems/{}/type", index));
						auto const& type = file_system.at("type"sv).get_ref<nlohmann::json::string_t const&>();
						if (type == "normal"sv) {
							info.type = FileSystemType::normal;
						}
						else if (type == "archive"sv) {
							info.type = FileSystemType::archive;
						}
						else {
							error_callback(std::format("[/initialize/file_systems/{}/type] unknown file system type '{}'"sv, index, type));
							return false;
						}
					}
					if (file_system.contains("path"sv) && file_system.at("path"sv).is_string()) {
						assert_type_is_string(file_system.at("path"sv), std::format("/initialize/file_systems/{}/type", index));
						info.path = file_system.at("path"sv).get_ref<nlohmann::json::string_t const&>();
					}
					self_fs.emplace_back(info);
				}
			}

			// graphics system

			if (root_initialize.contains("graphics_system"sv)) {
				assert_type_is_object(root_initialize.at("graphics_system"sv), "/initialize/graphics_system"sv);
				auto const& init_graphics = root_initialize.at("graphics_system"sv);
				auto& self_graphics = initialize.value().graphics_system.emplace();
				if (init_graphics.contains("preferred_device_name"sv)) {
					auto const& gs_preferred_device_name = init_graphics.at("preferred_device_name"sv);
					assert_type_is_string(gs_preferred_device_name, "/initialize/graphics_system/preferred_device_name"sv);
					self_graphics.preferred_device_name.emplace(gs_preferred_device_name.get_ref<std::string const&>());
				}
				if (init_graphics.contains("width"sv)) {
					assert_type_is_unsigned_integer(init_graphics.at("width"sv), "/initialize/graphics_system/width"sv);
					auto const width = self_graphics.width.emplace(init_graphics.at("width"sv).get<uint32_t>());
					if (width == 0) {
						error_callback("[/initialize/graphics_system/width] width must be greater than zero"sv);
						return false;
					}
				}
				if (init_graphics.contains("height"sv)) {
					assert_type_is_unsigned_integer(init_graphics.at("height"sv), "/initialize/graphics_system/height"sv);
					auto const height = self_graphics.height.emplace(init_graphics.at("height"sv).get<uint32_t>());
					if (height == 0) {
						error_callback("[/initialize/graphics_system/height] height must be greater than zero"sv);
						return false;
					}
				}
				if (init_graphics.contains("fullscreen"sv)) {
					assert_type_is_boolean(init_graphics.at("fullscreen"sv), "/initialize/graphics_system/fullscreen"sv);
					self_graphics.fullscreen.emplace(init_graphics.at("fullscreen"sv).get<bool>());
				}
				if (init_graphics.contains("vsync"sv)) {
					assert_type_is_boolean(init_graphics.at("vsync"sv), "/initialize/graphics_system/vsync"sv);
					self_graphics.vsync.emplace(init_graphics.at("vsync"sv).get<bool>());
				}
				//if (init_graphics.contains("display"sv)) {
				//	assert_type_is_object(init_graphics.at("display"sv), "/initialize/graphics_system/display"sv);
				//	auto const& init_graphics_display = init_graphics.at("display"sv);
				//	auto& self_display = self_graphics.display.emplace();
				//	// TODO
				//}
			}

			// audio system

			if (root_initialize.contains("audio_system"sv)) {
				assert_type_is_object(root_initialize.at("audio_system"sv), "/initialize/audio_system"sv);
				auto const& init_audio = root_initialize.at("audio_system"sv);
				auto& self_audio = initialize.value().audio_system.emplace();
				if (init_audio.contains("preferred_endpoint_name"sv)) {
					auto const& as_preferred_endpoint_name = init_audio.at("preferred_endpoint_name"sv);
					assert_type_is_string(as_preferred_endpoint_name, "/initialize/audio_system/preferred_endpoint_name"sv);
					self_audio.preferred_endpoint_name.emplace(as_preferred_endpoint_name.get_ref<std::string const&>());
				}
				if (init_audio.contains("sound_effect_volume"sv)) {
					assert_type_is_number(init_audio.at("sound_effect_volume"sv), "/initialize/audio_system/sound_effect_volume"sv);
					auto const volume = self_audio.sound_effect_volume.emplace(init_audio.at("sound_effect_volume"sv).get<float>());
					if (volume < 0.0f || volume > 1.0f) {
						error_callback("[/initialize/audio_system/sound_effect_volume] out of range [0.0, 1.0]"sv);
						return false;
					}
				}
				if (init_audio.contains("music_volume"sv)) {
					assert_type_is_number(init_audio.at("music_volume"sv), "/initialize/audio_system/music_volume"sv);
					auto const volume = self_audio.music_volume.emplace(init_audio.at("music_volume"sv).get<float>());
					if (volume < 0.0f || volume > 1.0f) {
						error_callback("[/initialize/audio_system/music_volume] out of range [0.0, 1.0]"sv);
						return false;
					}
				}
			}

			// application

			if (root_initialize.contains("application"sv)) {
				auto const& init_app = root_initialize.at("application"sv);
				assert_type_is_object(init_app, "/initialize/application"sv);
				auto& self_app = initialize.value().application.emplace();
				if (init_app.contains("frame_rate"sv)) {
					auto const& frame_rate = init_app.at("frame_rate"sv);
					assert_type_is_unsigned_integer(frame_rate, "/initialize/application/frame_rate"sv);
					self_app.frame_rate.emplace(frame_rate.get<uint32_t>());
				}
			}

			// window

			if (root_initialize.contains("window"sv)) {
				auto const& init_win = root_initialize.at("window"sv);
				assert_type_is_object(init_win, "/initialize/window"sv);
				auto& self_win = initialize.value().window.emplace();
				if (init_win.contains("title"sv)) {
					auto const& title = init_win.at("title"sv);
					assert_type_is_string(title, "/initialize/window/title"sv);
					self_win.title.emplace(title.get_ref<std::string const&>());
				}
				if (init_win.contains("cursor_visible"sv)) {
					auto const& cursor_visible = init_win.at("cursor_visible"sv);
					assert_type_is_boolean(cursor_visible, "/initialize/window/cursor_visible"sv);
					self_win.cursor_visible.emplace(cursor_visible.get<bool>());
				}
				if (init_win.contains("allow_window_corner"sv)) {
					auto const& allow_window_corner = init_win.at("allow_window_corner"sv);
					assert_type_is_boolean(allow_window_corner, "/initialize/window/allow_window_corner"sv);
					self_win.allow_window_corner.emplace(allow_window_corner.get<bool>());
				}
			}
		}

		return true;
	}

	Configuration::Configuration() : error_callback(defaultErrorCallback) {
	}
}

namespace core {
	class ConfigurationLoaderContext {
	public:
		static bool merge(ConfigurationLoader& loader, nlohmann::json const& root) {
			auto& messages = loader.messages;

			auto error_callback = [&](std::string_view const& message) {
				messages.emplace_back(message);
			};

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

			return true;
		}
		static bool load(ConfigurationLoader& loader, std::string_view const& path) {
			std::string json_text;
			if (!readTextFile(path, json_text)) {
				return false;
			}

			auto const root = nlohmann::json::parse(json_text, nullptr, false);
			if (root.is_discarded()) {
				return false;
			}

			return merge(loader, root);
		}
	};

	void ConfigurationLoader::merge(Configuration const& patch) {
		mergeOnly(patch);
		applyOnly();
	}

	void ConfigurationLoader::mergeOnly(Configuration const& patch) {
		if (patch.debug.has_value()) {
			// init self

			if (!configuration.debug.has_value()) {
				configuration.debug.emplace();
			}
			auto const& conf_debug = patch.debug.value();
			auto& self_debug = configuration.debug.value();

			// merge

			if (conf_debug.track_window_focus.has_value()) {
				self_debug.track_window_focus.emplace(conf_debug.track_window_focus.value());
			}
		}

		if (patch.application.has_value()) {
			// init self

			if (!configuration.application.has_value()) {
				configuration.application.emplace();
			}
			auto const& conf_app = patch.application.value();
			auto& self_app = configuration.application.value();

			// merge

			if (conf_app.uuid.has_value()) {
				self_app.uuid.emplace(conf_app.uuid.value());
			}
			if (conf_app.single_instance.has_value()) {
				self_app.single_instance.emplace(conf_app.single_instance.value());
			}
		}

		if (patch.initialize.has_value()) {
			// init self

			if (!configuration.initialize.has_value()) {
				configuration.initialize.emplace();
			}
			auto const& init = patch.initialize.value();
			auto& self = configuration.initialize.value();

			// file system

			if (!init.file_systems.empty()) {
				for (auto const& fs : init.file_systems) {
					self.file_systems.emplace_back(fs);
				}
			}

			// graphics system

			if (init.graphics_system.has_value()) {
				// init self

				if (!self.graphics_system.has_value()) {
					self.graphics_system.emplace();
				}
				auto const& init_graphics = init.graphics_system.value();
				auto& self_graphics = self.graphics_system.value();

				// merge

				if (init_graphics.preferred_device_name.has_value()) {
					self_graphics.preferred_device_name.emplace(init_graphics.preferred_device_name.value());
				}
				if (init_graphics.width.has_value()) {
					self_graphics.width.emplace(init_graphics.width.value());
				}
				if (init_graphics.height.has_value()) {
					self_graphics.height.emplace(init_graphics.height.value());
				}
				if (init_graphics.fullscreen.has_value()) {
					self_graphics.fullscreen.emplace(init_graphics.fullscreen.value());
				}
				if (init_graphics.vsync.has_value()) {
					self_graphics.vsync.emplace(init_graphics.vsync.value());
				}

				// TODO: display
			}

			// audio system

			if (init.audio_system.has_value()) {
				// init self

				if (!self.audio_system.has_value()) {
					self.audio_system.emplace();
				}
				auto const& init_audio = init.audio_system.value();
				auto& self_audio = self.audio_system.value();

				// merge

				if (init_audio.preferred_endpoint_name.has_value()) {
					self_audio.preferred_endpoint_name.emplace(init_audio.preferred_endpoint_name.value());
				}
				if (init_audio.sound_effect_volume.has_value()) {
					self_audio.sound_effect_volume.emplace(init_audio.sound_effect_volume.value());
				}
				if (init_audio.music_volume.has_value()) {
					self_audio.music_volume.emplace(init_audio.music_volume.value());
				}
			}

			// application

			if (init.application.has_value()) {
				// init self

				if (!self.application.has_value()) {
					self.application.emplace();
				}
				auto const& init_app = init.application.value();
				auto& self_app = self.application.value();

				// merge

				if (init_app.frame_rate.has_value()) {
					self_app.frame_rate.emplace(init_app.frame_rate.value());
				}
			}

			// window

			if (init.window.has_value()) {
				// init self

				if (!self.window.has_value()) {
					self.window.emplace();
				}
				auto const& init_win = init.window.value();
				auto& self_win = self.window.value();

				// merge

				if (init_win.title.has_value()) {
					self_win.title.emplace(init_win.title.value());
				}
				if (init_win.cursor_visible.has_value()) {
					self_win.cursor_visible.emplace(init_win.cursor_visible.value());
				}
				if (init_win.allow_window_corner.has_value()) {
					self_win.allow_window_corner.emplace(init_win.allow_window_corner.value());
				}
			}
		}
	}

	void ConfigurationLoader::applyOnly() {
		// apply

		if (configuration.debug.has_value()) {
			auto const& dbg = configuration.debug.value();
			if (dbg.track_window_focus.has_value()) {
				debug.setTrackWindowFocus(dbg.track_window_focus.value());
			}
		}

		if (configuration.application.has_value()) {
			auto const& app = configuration.application.value();
			if (app.uuid.has_value()) {
				application.setUuid(app.uuid.value());
			}
			if (app.single_instance.has_value()) {
				application.setSingleInstance(app.single_instance.value());
			}
		}

		if (configuration.initialize.has_value()) {
			auto const& init = configuration.initialize.value();
			if (!init.file_systems.empty()) {
				initialize.file_systems = init.file_systems;
			}
			if (init.graphics_system.has_value()) {
				auto const& graphics_system = init.graphics_system.value();
				if (graphics_system.preferred_device_name.has_value()) {
					initialize.graphics_system.setPreferredDeviceName(graphics_system.preferred_device_name.value());
				}
				if (graphics_system.width.has_value()) {
					initialize.graphics_system.setWidth(graphics_system.width.value());
				}
				if (graphics_system.height.has_value()) {
					initialize.graphics_system.setHeight(graphics_system.height.value());
				}
				if (graphics_system.fullscreen.has_value()) {
					initialize.graphics_system.setFullscreen(graphics_system.fullscreen.value());
				}
				if (graphics_system.vsync.has_value()) {
					initialize.graphics_system.setVsync(graphics_system.vsync.value());
				}
			}
			if (init.audio_system.has_value()) {
				auto const& audio_system = init.audio_system.value();
				if (audio_system.preferred_endpoint_name.has_value()) {
					initialize.audio_system.setPreferredEndpointName(audio_system.preferred_endpoint_name.value());
				}
				if (audio_system.sound_effect_volume.has_value()) {
					initialize.audio_system.setSoundEffectVolume(audio_system.sound_effect_volume.value());
				}
				if (audio_system.music_volume.has_value()) {
					initialize.audio_system.setMusicVolume(audio_system.music_volume.value());
				}
			}
			if (init.application.has_value()) {
				auto const& init_app = init.application.value();
				if (init_app.frame_rate.has_value()) {
					initialize.application.setFrameRate(init_app.frame_rate.value());
				}
			}
			if (init.window.has_value()) {
				auto const& init_win = init.window.value();
				if (init_win.title.has_value()) {
					initialize.window.setTitle(init_win.title.value());
				}
				if (init_win.cursor_visible.has_value()) {
					initialize.window.setCursorVisible(init_win.cursor_visible.value());
				}
				if (init_win.allow_window_corner.has_value()) {
					initialize.window.setAllowWindowCorner(init_win.allow_window_corner.value());
				}
			}
		}
	}

	bool ConfigurationLoader::loadFromFile(std::string_view const& path) {
		// load primary configuration

		configuration.setErrorCallback([&](std::string_view const& message) {
			messages.emplace_back(std::format("[{}] {}", path, message));
		});
		configuration.setAllowInclude(true); // only allow top level configuration
		std::vector<Configuration::Include> include;
		if (!configuration.loadFromFile(path)) {
			messages.emplace_back(std::format("[{}] load failed", path));
			return false;
		}
		for (auto const& it : configuration.include) {
			include.emplace_back(it);
		}
		configuration.include.clear();

		std::vector<std::string> paths;

		// load additional configurations

		while (!include.empty()) {
			auto const& first = include.front();

			std::error_code ec;
			if (first.optional && !std::filesystem::is_regular_file(to_u8string_view(first.path), ec)) {
				include.erase(include.begin()); // need to remove used element
				continue; // skip optional
			}

			Configuration patch;
			patch.setErrorCallback([&](std::string_view const& message) {
				messages.emplace_back(std::format("[{}] {}", first.path, message));
			});
			if (!patch.loadFromFile(first.path)) {
				messages.emplace_back(std::format("[{}] load failed", first.path));
				return false;
			}
			for (auto const& it : patch.include) {
				include.emplace_back(it);
			}

			merge(patch);

			paths.emplace_back(first.path);

			include.erase(include.begin()); // need to remove used element
		}

		// final check

		if (configuration.application) {
			auto const& app = configuration.application.value();
			if (app.single_instance && app.single_instance.value() && !app.uuid) {
				messages.emplace_back(std::format("[{}] single_instance require uuid string to be set", path));
			}
		}

		// apply

		applyOnly();

		// new

		using Level = Logging::Level;

		logging.console.setEnable(false);
		logging.console.setThreshold(Level::info);

		logging.file.setEnable(true);
		logging.file.setThreshold(Level::info);
		logging.file.setPath("");

		logging.rolling_file.setEnable(false);
		logging.rolling_file.setThreshold(Level::info);
		logging.rolling_file.setPath("");
		logging.rolling_file.setMaxHistory(10);

		if (!ConfigurationLoaderContext::load(*this, path)) {
			return false;
		}
		for (auto const& v : paths) {
			if (!ConfigurationLoaderContext::load(*this, v)) {
				return false;
			}
		}

		return true;
	}

	std::string ConfigurationLoader::getFormattedMessage() {
		std::string message;
		for (auto const& s : messages) {
			message.append(s);
			message.push_back('\n');
		}
		if (message.back() == '\n') {
			message.pop_back();
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
