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

		struct Debug {
			std::optional<bool> track_window_focus;
		};

		std::optional<Debug> debug;

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
		class Debug {
		public:
			inline Debug& setTrackWindowFocus(bool const track_window_focus_) {
				track_window_focus = track_window_focus_;
				return *this;
			}
			inline bool isTrackWindowFocus() const noexcept { return track_window_focus; }
		private:
			bool track_window_focus{ false };
		};
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
		class AudioSystem {
		public:
			inline bool hasPreferredEndpointName() const noexcept { return !preferred_endpoint_name.empty(); }
			inline std::string const& getPreferredEndpointName() const noexcept { return preferred_endpoint_name; }
			inline AudioSystem& setPreferredEndpointName(std::string const& preferred_endpoint_name_) {
				preferred_endpoint_name = preferred_endpoint_name_;
				return *this;
			}
			inline float getSoundEffectVolume() const noexcept { return sound_effect_volume; }
			inline AudioSystem& setSoundEffectVolume(float const sound_effect_volume_) {
				sound_effect_volume = sound_effect_volume_;
				return *this;
			}
			inline float getMusicVolume() const noexcept { return music_volume; }
			inline AudioSystem& setMusicVolume(float const music_volume_) {
				music_volume = music_volume_;
				return *this;
			}
		private:
			std::string preferred_endpoint_name;
			float sound_effect_volume{ 1.0 };
			float music_volume{ 1.0f };
		};
		class Initialize {
			friend class ConfigurationLoader;
		public:
			inline AudioSystem const& getAudioSystem() const noexcept { return audio_system; }
		private:
			AudioSystem audio_system;
		};
	public:
		bool loadFromFile(std::string_view const& path);
		inline std::vector<std::string> const& getMessages() const noexcept { return messages; }
		std::string getFormattedMessage();
		inline Debug const& getDebug() const noexcept { return debug; }
		inline Application const& getApplication() const noexcept { return application; }
		inline Initialize const& getInitialize() const noexcept { return initialize; }
	public:
		static ConfigurationLoader& getInstance();
	private:
		std::vector<std::string> messages;
		Configuration configuration;
		Debug debug;
		Initialize initialize;
		Application application;
	};
}
