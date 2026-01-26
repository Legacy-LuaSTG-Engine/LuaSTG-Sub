#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"
#include "core/Display.hpp"

namespace core {
	struct IWindowEventListener {
		virtual void onWindowCreate() {};
		virtual void onWindowDestroy() {};

		virtual void onWindowActive() {};
		virtual void onWindowInactive() {};

		virtual void onWindowSize(Vector2U size) { (void)size; };
		virtual void onWindowFullscreenStateChange(bool state) { (void)state; }
		virtual void onWindowDpiChange() {};

		virtual void onWindowClose() {};

		virtual void onDeviceChange() {};

		struct NativeWindowMessageResult {
			intptr_t result;
			bool should_return;

			NativeWindowMessageResult() : result(0), should_return(false) {}
			NativeWindowMessageResult(intptr_t v, bool b) : result(v), should_return(b) {}
		};

		virtual NativeWindowMessageResult onNativeWindowMessage(void*, uint32_t, uintptr_t, intptr_t) { return {}; };
	};

	enum class WindowFrameStyle {
		None,
		Fixed,
		Normal,
	};

	enum class WindowLayer {
		Bottom,
		Normal,
		Top,
		TopMost,
	};

	enum class WindowCursor {
		None,

		Arrow,
		Hand,

		Cross,
		TextInput,

		Resize,
		ResizeEW,
		ResizeNS,
		ResizeNESW,
		ResizeNWSE,

		NotAllowed,
		Wait,
	};

	CORE_INTERFACE IWindow : IReferenceCounted {
		virtual void addEventListener(IWindowEventListener* e) = 0;
		virtual void removeEventListener(IWindowEventListener* e) = 0;

		virtual void* getNativeHandle() = 0;

		virtual void setIMEState(bool enable) = 0;
		virtual bool getIMEState() = 0;

		// vvvvvvvv BEGIN WIP

		virtual void setInputMethodPosition(Vector2I position) = 0;

		virtual bool       textInput_isEnabled() = 0;
		virtual void       textInput_setEnabled(bool enabled) = 0;
		virtual StringView textInput_getBuffer() = 0;
		virtual void       textInput_clearBuffer() = 0;
		virtual uint32_t   textInput_getCursorPosition() = 0;
		virtual void       textInput_setCursorPosition(uint32_t code_point_index) = 0;
		virtual void       textInput_addCursorPosition(int32_t offset_by_code_point) = 0;
		virtual void       textInput_removeBufferRange(uint32_t code_point_index, uint32_t code_point_count) = 0;
		virtual void       textInput_insertBufferRange(uint32_t code_point_index, StringView str) = 0;
		virtual void       textInput_backspace(uint32_t code_point_count) = 0;

		// ^^^^^^^^ END WIP

		virtual void setTitleText(StringView str) = 0;
		virtual StringView getTitleText() = 0;

		virtual bool setFrameStyle(WindowFrameStyle style) = 0;
		virtual WindowFrameStyle getFrameStyle() = 0;

		virtual Vector2U getSize() = 0; // TODO: history problem
		virtual Vector2U _getCurrentSize() = 0; // TODO: history problem
		virtual bool setSize(Vector2U v) = 0;
		virtual bool setLayer(WindowLayer layer) = 0;

		virtual uint32_t getDPI() = 0;
		virtual float getDPIScaling() = 0;

		virtual void setWindowMode(Vector2U size, WindowFrameStyle style = WindowFrameStyle::Normal, IDisplay* display = nullptr) = 0;
		virtual void setFullScreenMode(IDisplay* display = nullptr) = 0;
		virtual void setCentered(bool show, IDisplay* display = nullptr) = 0;

		virtual void setCustomSizeMoveEnable(bool v) = 0;
		virtual void setCustomMinimizeButtonRect(RectI v) = 0;
		virtual void setCustomCloseButtonRect(RectI v) = 0;
		virtual void setCustomMoveButtonRect(RectI v) = 0;

		virtual bool setCursor(WindowCursor type) = 0;
		virtual WindowCursor getCursor() = 0;

		// Windows 11
		virtual void setWindowCornerPreference(bool allow) = 0;
		virtual void setTitleBarAutoHidePreference(bool allow) = 0;

		static bool create(IWindow** pp_window);
		static bool create(Vector2U size, StringView title_text, WindowFrameStyle style, bool show, IWindow** pp_window);
	};

    CORE_INTERFACE_ID(IWindow, "9432a56d-e3d2-5173-b313-a9581b373155")
}
