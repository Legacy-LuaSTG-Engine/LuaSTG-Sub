#include "core/Configuration.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>
#include "nlohmann/json.hpp"

using namespace std::string_view_literals;

namespace core {
	static std::u8string_view to_u8string_view(std::string_view const& sv) {
		static_assert(CHAR_BIT == 8 && sizeof(char) == 1 && sizeof(char) == sizeof(char8_t));
		return { reinterpret_cast<char8_t const*>(sv.data()), sv.size() };
	}

	static void defaultErrorCallback(std::string_view const& message) {
		std::cout << message << std::endl;
	}

	static bool readTextFile(std::string_view const& path, std::string& str) {
		str.clear();
		std::error_code ec;
		static_assert(CHAR_BIT == 8 && sizeof(char) == 1 && sizeof(char) == sizeof(char8_t));
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

		// include

		if (root.contains("include"sv)) {
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
		}

	#undef assert_type_is_boolean
	#undef assert_type_is_unsigned_integer
	#undef assert_type_is_number
	#undef assert_type_is_string
	#undef assert_type_is_array
	#undef assert_type_is_object

		return true;
	}

	Configuration::Configuration() : error_callback(defaultErrorCallback) {
	}
}

namespace core {
	bool ConfigurationLoader::loadFromFile(std::string_view const& path) {
		// load primary configuration

		configuration.setErrorCallback([&](std::string_view const& message) {
			messages.emplace_back(std::format("[{}] {}", path, message));
		});
		std::vector<Configuration::Include> include;
		if (!configuration.loadFromFile(path)) {
			messages.emplace_back(std::format("[{}] load failed", path));
			return false;
		}
		for (auto const& it : configuration.include) {
			include.emplace_back(it);
		}
		configuration.include.clear();

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

					if (init_audio.sound_effect_volume.has_value()) {
						self_audio.sound_effect_volume.emplace(init_audio.sound_effect_volume.value());
					}
					if (init_audio.music_volume.has_value()) {
						self_audio.music_volume.emplace(init_audio.music_volume.value());
					}
				}
			}

			include.erase(include.begin()); // need to remove used element
		}

		return true;
	}

	ConfigurationLoader& ConfigurationLoader::getInstance() {
		static ConfigurationLoader instance;
		return instance;
	}
}
