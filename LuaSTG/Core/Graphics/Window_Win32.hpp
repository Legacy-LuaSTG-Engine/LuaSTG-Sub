#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/ApplicationModel.hpp"
#include "Core/Graphics/Window.hpp"
#include "Platform/WindowSizeMoveController.hpp"
#include "Platform/RuntimeLoader/DesktopWindowManager.hpp"
#include "Platform/ImmersiveTitleBarController.hpp"

namespace core::Graphics
{
	class Display_Win32 : public implement::ReferenceCounted<IDisplay> {
	private:
		HMONITOR win32_monitor{};
	public:
		void* getNativeHandle();
		void getFriendlyName(IImmutableString** output);
		Vector2U getSize();
		Vector2I getPosition();
		RectI getRect();
		Vector2U getWorkAreaSize();
		Vector2I getWorkAreaPosition();
		RectI getWorkAreaRect();
		bool isPrimary();
		float getDisplayScale();
	public:
		Display_Win32(HMONITOR monitor);
		~Display_Win32();
	};

	struct SetWindowedModeParameters {
		Vector2U size;
		WindowFrameStyle style{ WindowFrameStyle::None };
		IDisplay* display{};
	};

	class Window_Win32 : public implement::ReferenceCounted<IWindow>
	{
	private:
		WNDCLASSEXW win32_window_class{ sizeof(WNDCLASSEXW) };
		ATOM win32_window_class_atom{ 0 };
		static LRESULT CALLBACK win32_window_callback(HWND window, UINT message, WPARAM arg1, LPARAM arg2);

		HWND win32_window{};

		BOOL win32_window_ime_enable{ FALSE };

		UINT win32_window_width{ 640 };
		UINT win32_window_height{ 480 };
		UINT win32_window_dpi{ USER_DEFAULT_SCREEN_DPI };

		INT_PTR win32_window_icon_id{ /* IDI_APPICON THIS IS A HACK */ 101 };

		std::string win32_window_text{ "LuaSTG Sub" };
		std::array<wchar_t, 512> win32_window_text_w{};

		WindowCursor m_cursor{ WindowCursor::Arrow };
		HCURSOR win32_window_cursor{};

		WindowFrameStyle m_framestyle{ WindowFrameStyle::Normal };
		DWORD win32_window_style{ WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX) };
		DWORD win32_window_style_ex{ 0 };
		WINDOWPLACEMENT m_last_window_placement{};
		BOOL m_alt_down{ FALSE };
		BOOL m_fullscreen_mode{ FALSE };
		BOOL m_ignore_size_message{ FALSE }; // 在 SetWindowLongPtr 修改窗口样式时，可以忽略 WM_SIZE
		BOOL m_allow_windows_11_window_corner{ TRUE };

		BOOL win32_window_is_sizemove{ FALSE };
		BOOL win32_window_is_menu_loop{ FALSE };
		BOOL win32_window_want_track_focus{ FALSE };
		bool enable_track_window_focus{ false };
		bool auto_hide_title_bar{ false };

		Platform::WindowSizeMoveController m_sizemove;
		platform::windows::ImmersiveTitleBarController m_title_bar_controller;
		Platform::RuntimeLoader::DesktopWindowManager dwmapi_loader;

		LRESULT onMessage(HWND window, UINT message, WPARAM arg1, LPARAM arg2);

		bool createWindowClass();
		void destroyWindowClass();
		bool createWindow();
		void destroyWindow();

		IApplicationModel* m_framework{};

	public:
		// 内部方法

		HWND GetWindow() { return win32_window; }
		platform::windows::ImmersiveTitleBarController& getTitleBarController() { return m_title_bar_controller; }

		void convertTitleText();

		RectI getRect();
		bool setRect(RectI v);
		RectI getClientRect();
		bool setClientRect(RectI v);
		uint32_t getDPI();
		bool _recreateWindow();
		bool recreateWindow();
		void _toggleFullScreenMode();
		void _setWindowMode(SetWindowedModeParameters* parameters, bool ignore_size);
		void _setFullScreenMode(IDisplay* display);

		void implSetApplicationModel(IApplicationModel* p_framework) { m_framework = p_framework; }

	private:

		enum class EventType
		{
			WindowCreate,
			WindowDestroy,

			WindowActive,
			WindowInactive,

			WindowClose,

			WindowSize,
			WindowFullscreenStateChange,
			WindowDpiChanged,

			NativeWindowMessage,

			DeviceChange,
		};
		union EventData
		{
			Vector2U window_size;
			bool window_fullscreen_state;
		};
		bool m_is_dispatch_event{ false };
		std::vector<IWindowEventListener*> m_eventobj;
		std::vector<IWindowEventListener*> m_eventobj_late;
		void dispatchEvent(EventType t, EventData d = {});

	public:

		void addEventListener(IWindowEventListener* e) override;
		void removeEventListener(IWindowEventListener* e) override;

	private:

		std::u32string m_text_input_buffer;
		std::u8string m_text_input_buffer_u8;
		uint32_t m_text_input_cursor{};
		char16_t m_text_input_last_high_surrogate{};
		bool m_text_input_enabled{ false };

		void       textInput_updateBuffer();
		void       textInput_addChar32(char32_t code);
		void       textInput_handleChar32(char32_t code);
		void       textInput_handleChar16(char16_t code);

	public:

		// IWindow

		bool       textInput_isEnabled() override;
		void       textInput_setEnabled(bool enabled) override;
		StringView textInput_getBuffer() override;
		void       textInput_clearBuffer() override;
		uint32_t   textInput_getCursorPosition() override;
		void       textInput_setCursorPosition(uint32_t code_point_index) override;
		void       textInput_addCursorPosition(int32_t offset_by_code_point) override;
		void       textInput_removeBufferRange(uint32_t code_point_index, uint32_t code_point_count) override;
		void       textInput_insertBufferRange(uint32_t code_point_index, StringView str) override;
		void       textInput_backspace(uint32_t code_point_count) override;

		void* getNativeHandle() override;

		void setIMEState(bool enable) override;
		bool getIMEState() override;
		void setInputMethodPosition(Vector2I position) override;

		void setTitleText(StringView str) override;
		StringView getTitleText() override;

		bool setFrameStyle(WindowFrameStyle style) override;
		WindowFrameStyle getFrameStyle() override;

		Vector2U getSize() override;
		Vector2U _getCurrentSize() override;
		bool setSize(Vector2U v) override;
		bool setLayer(WindowLayer layer) override;

		float getDPIScaling() override;

		void setWindowMode(Vector2U size, WindowFrameStyle style, IDisplay* display) override;
		void setFullScreenMode(IDisplay* display) override;
		void setCentered(bool show, IDisplay* display) override;

		void setCustomSizeMoveEnable(bool v) override;
		void setCustomMinimizeButtonRect(RectI v) override;
		void setCustomCloseButtonRect(RectI v) override;
		void setCustomMoveButtonRect(RectI v) override;

		bool setCursor(WindowCursor type) override;
		WindowCursor getCursor() override;

		void setWindowCornerPreference(bool allow) override; // Windows 11
		void setTitleBarAutoHidePreference(bool allow) override; // Windows 11

		// Window/Win32

		Window_Win32();
		~Window_Win32() override;

		static bool create(Window_Win32** pp_window);
		static bool create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, Window_Win32** pp_window);
	};
}
