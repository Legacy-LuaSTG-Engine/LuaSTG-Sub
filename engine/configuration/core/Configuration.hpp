#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>
#include <filesystem>

namespace core {
#define GetterSetterBoolean(_class_, _field_, _method_name_) \
	inline bool is##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ (bool const _field_##_ ) { _field_ = _field_##_ ; return *this; }

#define GetterSetterString(_class_, _field_, _method_name_) \
	inline bool has##_method_name_ () const noexcept { return !((_field_).empty()) ; }; \
	inline std::string const& get##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ (std::string_view const& _field_##_ ) { _field_ = _field_##_ ; return *this; }

#define GetterSetterPrimitive(_class_, _type_, _field_, _method_name_) \
	inline _type_ get##_method_name_ () const noexcept { return _field_ ; }; \
	inline _class_ & set##_method_name_ ( _type_ const _field_##_ ) { _field_ = _field_##_ ; return *this; }

	class ConfigurationLoader {
		friend class ConfigurationLoaderContext;
	public:
		class Debug {
		public:
			GetterSetterBoolean(Debug, track_window_focus, TrackWindowFocus);
		private:
			bool track_window_focus{ false }; // TODO: 应该归类到窗口配置中
		};
		class Application {
		public:
			GetterSetterString(Application, uuid, Uuid);
			GetterSetterBoolean(Application, single_instance, SingleInstance);
		private:
			std::string uuid;
			bool single_instance{ false };
		};
		class Logging {
			friend class ConfigurationLoader;
			friend class ConfigurationLoaderContext;
		public:
			enum class Level {
				debug,
				info,
				warn,
				error,
				fatal,
			};
			class Base {
			public:
				GetterSetterBoolean(Base, enable, Enable);
				GetterSetterPrimitive(Base, Level, threshold, Threshold);
			private:
				bool enable{ false };
				Level threshold{ Level::info };
			};
			class Debugger : public Base {
			};
			class Console : public Base {
			public:
				GetterSetterBoolean(Console, preserve, Preserve);
			private:
				bool preserve{ false };
			};
			class File : public Base {
			public:
				GetterSetterString(File, path, Path);
			private:
				std::string path;
			};
			class RollingFile : public File {
			public:
				GetterSetterPrimitive(RollingFile, uint32_t, max_history, MaxHistory);
			private:
				uint32_t max_history{ 10 };
			};
		public:
			inline Debugger const& getDebugger() const noexcept { return debugger; }
			inline Console const& getConsole() const noexcept { return console; }
			inline File const& getFile() const noexcept { return file; }
			inline RollingFile const& getRollingFile() const noexcept { return rolling_file; }
		private:
			Debugger debugger;
			Console console;
			File file;
			RollingFile rolling_file;
		};
		class FileSystem {
			friend class ConfigurationLoader;
			friend class ConfigurationLoaderContext;
		public:
			class ResourceFileSystem {
			public:
				enum class Type {
					directory,
					archive,
				};
			public:
				GetterSetterString(ResourceFileSystem, name, Name);
				GetterSetterString(ResourceFileSystem, path, Path);
				GetterSetterPrimitive(ResourceFileSystem, Type, type, Type);
			private:
				std::string name;
				std::string path;
				Type type{ Type::directory };
			};
		public:
			inline std::vector<ResourceFileSystem> const& getResources() const noexcept { return resources; }
			GetterSetterString(FileSystem, user, User);
		private:
			std::vector<ResourceFileSystem> resources;
			std::string user;
		};
		class Timing {
		public:
			GetterSetterPrimitive(Timing, uint32_t, frame_rate, FrameRate);
		private:
			uint32_t frame_rate{ 60 };
		};
		/* TODO*/ struct Display {
			std::string device_name;
			int32_t left{};
			int32_t top{};
			int32_t right{};
			int32_t bottom{};
		};
		class Window {
		public:
			GetterSetterString(Window, title, Title);
			GetterSetterBoolean(Window, cursor_visible, CursorVisible);
			GetterSetterBoolean(Window, allow_window_corner, AllowWindowCorner);
		private:
			std::string title;
			bool cursor_visible{ true };
			bool allow_window_corner{ true };
		};
		class GraphicsSystem {
		public:
			GetterSetterString(GraphicsSystem, preferred_device_name, PreferredDeviceName);
			GetterSetterPrimitive(GraphicsSystem, uint32_t, width, Width);
			GetterSetterPrimitive(GraphicsSystem, uint32_t, height, Height);
			GetterSetterBoolean(GraphicsSystem, fullscreen, Fullscreen);
			GetterSetterBoolean(GraphicsSystem, vsync, Vsync);
			GetterSetterBoolean(GraphicsSystem, allow_software_device, AllowSoftwareDevice);
			GetterSetterBoolean(GraphicsSystem, allow_exclusive_fullscreen, AllowExclusiveFullscreen);
			GetterSetterBoolean(GraphicsSystem, allow_modern_swap_chain, AllowModernSwapChain);
			GetterSetterBoolean(GraphicsSystem, allow_direct_composition, AllowDirectComposition);
		private:
			std::string preferred_device_name;
			uint32_t width{ 640u };
			uint32_t height{ 480u };
			bool fullscreen{};
			bool vsync{};
			bool allow_software_device{};
			bool allow_exclusive_fullscreen{ true };
			bool allow_modern_swap_chain{ true };
			bool allow_direct_composition{ true };
		};
		class AudioSystem {
		public:
			GetterSetterString(AudioSystem, preferred_endpoint_name, PreferredEndpointName);
			GetterSetterPrimitive(AudioSystem, float, sound_effect_volume, SoundEffectVolume);
			GetterSetterPrimitive(AudioSystem, float, music_volume, MusicVolume);
		private:
			std::string preferred_endpoint_name;
			float sound_effect_volume{ 1.0f };
			float music_volume{ 1.0f };
		};
	public:
		ConfigurationLoader();
		bool loadFromFile(std::string_view const& path);
		bool loadFromCommandLineArguments();
		inline std::vector<std::string> const& getMessages() const noexcept { return messages; }
		std::string getFormattedMessage();
		inline Debug const& getDebug() const noexcept { return debug; }
		inline Application const& getApplication() const noexcept { return application; }
		inline Logging const& getLogging() const noexcept { return logging; }
		inline FileSystem const& getFileSystem() const noexcept { return file_system; }
		inline Timing const& getTiming() const noexcept { return timing; }
		inline Window const& getWindow() const noexcept { return window; }
		inline GraphicsSystem const& getGraphicsSystem() const noexcept { return graphics_system; }
		inline AudioSystem const& getAudioSystem() const noexcept { return audio_system; }
	public:
		inline Window& getWindowRef() { return window; }
		inline GraphicsSystem& getGraphicsSystemRef() { return graphics_system; }
		inline AudioSystem& getAudioSystemRef() { return audio_system; }
	public:
		static bool exists(std::string_view const& path);
		static bool replaceAllPredefinedVariables(std::string_view const& input, std::string& output);
		static bool resolvePathWithPredefinedVariables(std::string_view const& input, std::filesystem::path& output, bool create_directories = false);
		static bool resolveFilePathWithPredefinedVariables(std::string_view const& input, std::filesystem::path& output, bool create_parent_directories = false);
		static ConfigurationLoader& getInstance();
	private:
		std::vector<std::string> messages;
		Debug debug;
		Application application;
		Logging logging;
		FileSystem file_system;
		Timing timing;
		Window window;
		GraphicsSystem graphics_system;
		AudioSystem audio_system;
	};

#undef GetterSetterBoolean
#undef GetterSetterString
#undef GetterSetterPrimitive
}
