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

		struct Application {
			std::optional<std::string> uuid;
			std::optional<bool> single_instance;
		};

		std::optional<Application> application;

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
			std::optional<std::string> preferred_device_name;
			std::optional<uint32_t> width;
			std::optional<uint32_t> height;
			std::optional<bool> fullscreen;
			std::optional<bool> vsync;
			std::optional<Display> display;
		};

		struct AudioSystem {
			std::optional<std::string> preferred_endpoint_name;
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

		bool allow_include{ false };

	public:
		void setErrorCallback(std::function<void(std::string_view const&)> const& cb);

		inline void setAllowInclude(bool const v) { allow_include = v; }

		bool loadFromFile(std::string_view const& path);

		Configuration();
	};

	class ConfigurationLoader {
	public:
		class Application {
		public:
			inline Application& setUuid(std::string const& uuid_) {
				uuid = uuid_;
				return *this;
			}
			inline Application& setSingleInstance(bool const single_instance_) {
				single_instance = single_instance_;
				return *this;
			}
			inline bool hasUuid() const noexcept { return !uuid.empty(); }
			inline std::string const& getUuid() const noexcept { return uuid; }
			inline bool isSingleInstance() const noexcept { return single_instance; }
		private:
			std::string uuid;
			bool single_instance{ false };
		};
	public:
		bool loadFromFile(std::string_view const& path);
		inline std::vector<std::string> const& getMessages() const noexcept { return messages; }
		std::string getFormattedMessage();
		inline Application const& getApplication() { return application; }
	public:
		static ConfigurationLoader& getInstance();
	private:
		std::vector<std::string> messages;
		Configuration configuration;
		Application application;
	};
}
