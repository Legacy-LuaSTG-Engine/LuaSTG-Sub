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

		struct InitApplication {
			std::optional<uint32_t> frame_rate;
		};

		struct InitWindow {
			std::optional<std::string> title;
			std::optional<bool> cursor_visible;
			std::optional<bool> allow_window_corner;
		};

		struct Initialize {
			std::vector<FileSystem> file_systems;

			std::optional<GraphicsSystem> graphics_system;

			std::optional<AudioSystem> audio_system;

			std::optional<InitApplication> application;

			std::optional<InitWindow> window;
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

#define GetterSetterBoolean(_class_, _field_, _method_name_) \
	inline bool is##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ (bool const _field_##_ ) { _field_ = _field_##_ ; return *this; }

#define GetterSetterString(_class_, _field_, _method_name_) \
	inline bool has##_method_name_ () const noexcept { return !((_field_).empty()) ; }; \
	inline std::string const& get##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ (std::string const& _field_##_ ) { _field_ = _field_##_ ; return *this; }

#define GetterSetterPrimitive(_class_, _type_, _field_, _method_name_) \
	inline _type_ get##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ ( _type_ const _field_##_ ) { _field_ = _field_##_ ; return *this; }

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
		class GraphicsSystem {
		public:
			inline bool hasPreferredDeviceName() const noexcept { return !preferred_device_name.empty(); }
			inline std::string const& getPreferredDeviceName() const noexcept { return preferred_device_name; }
			inline GraphicsSystem& setPreferredDeviceName(std::string const& preferred_device_name_) {
				preferred_device_name = preferred_device_name_;
				return *this;
			}
			inline uint32_t getWidth() const noexcept { return width; }
			inline GraphicsSystem& setWidth(uint32_t const width_) {
				width = width_;
				return *this;
			}
			inline uint32_t getHeight() const noexcept { return height; }
			inline GraphicsSystem& setHeight(uint32_t const height_) {
				height = height_;
				return *this;
			}
			inline bool isFullscreen() const noexcept { return fullscreen; }
			inline GraphicsSystem& setFullscreen(bool const fullscreen_) {
				fullscreen = fullscreen_;
				return *this;
			}
			inline bool isVsync() const noexcept { return vsync; }
			inline GraphicsSystem& setVsync(bool const vsync_) {
				vsync = vsync_;
				return *this;
			}
		private:
			std::string preferred_device_name;
			uint32_t width{ 640 };
			uint32_t height{ 480 };
			bool fullscreen{};
			bool vsync{};
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
		class InitApplication {
		public:
			GetterSetterPrimitive(InitApplication, uint32_t, frame_rate, FrameRate);
		private:
			uint32_t frame_rate{ 60 };
		};
		class InitWindow {
		public:
			GetterSetterString(InitWindow, title, Title);
			GetterSetterBoolean(InitWindow, cursor_visible, CursorVisible);
			GetterSetterBoolean(InitWindow, allow_window_corner, AllowWindowCorner);
		private:
			std::string title;
			bool cursor_visible{ true };
			bool allow_window_corner{ true };
		};
		class Initialize {
			friend class ConfigurationLoader;
		public:
			inline std::vector<Configuration::FileSystem> const& getFileSystems() const noexcept { return file_systems; }
			inline GraphicsSystem const& getGraphicsSystem() const noexcept { return graphics_system; }
			inline AudioSystem const& getAudioSystem() const noexcept { return audio_system; }
			inline InitApplication const& getApplication() const noexcept { return application; }
			inline InitWindow const& getWindow() const noexcept { return window; }
		private:
			std::vector<Configuration::FileSystem> file_systems;
			GraphicsSystem graphics_system;
			AudioSystem audio_system;
			InitApplication application;
			InitWindow window;
		};
	public:
		void merge(Configuration const& config);
		bool loadFromFile(std::string_view const& path);
		inline std::vector<std::string> const& getMessages() const noexcept { return messages; }
		std::string getFormattedMessage();
		inline Debug const& getDebug() const noexcept { return debug; }
		inline Application const& getApplication() const noexcept { return application; }
		inline Initialize const& getInitialize() const noexcept { return initialize; }
	public:
		static ConfigurationLoader& getInstance();
	private:
		void mergeOnly(Configuration const& config);
		void applyOnly();
	private:
		std::vector<std::string> messages;
		Configuration configuration;
		Debug debug;
		Initialize initialize;
		Application application;
	};

#undef GetterSetterBoolean
#undef GetterSetterString
#undef GetterSetterPrimitive
}
