#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>

namespace core {
	struct Configuration {
	public:
		struct Include {
			std::string path;
			bool optional{ false };
		};

		std::vector<Include> include;

		enum class FileSystemType {
			normal,
			archive,
		};

		struct FileSystem {
			FileSystemType type{ FileSystemType::normal };
			std::string path;
		};

		struct Display {
			std::string device_name;
			int32_t left{};
			int32_t top{};
			int32_t right{};
			int32_t bottom{};
		};

		struct GraphicsSystem {
			std::optional<uint32_t> width;
			std::optional<uint32_t> height;
			std::optional<bool> fullscreen;
			std::optional<bool> vsync;
			std::optional<Display> display;
		};

		struct AudioSystem {
			std::optional<float> sound_effect_volume;
			std::optional<float> music_volume;
		};

		struct Initialize {
			std::vector<FileSystem> file_systems;

			std::optional<GraphicsSystem> graphics_system;

			std::optional<AudioSystem> audio_system;
		};

		std::optional<Initialize> initialize;
	private:
		std::function<void(std::string_view const&)> error_callback;

	public:
		void setErrorCallback(std::function<void(std::string_view const&)> const& cb);

		bool loadFromFile(std::string_view const& path);

		Configuration();
	};

	class ConfigurationLoader {
	public:
		bool loadFromFile(std::string_view const& path);
	public:
		static ConfigurationLoader& getInstance();
	private:
		std::vector<std::string> messages;
		Configuration configuration;
	};
}
