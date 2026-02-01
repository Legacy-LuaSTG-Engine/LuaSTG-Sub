#include "Display.hpp"
#include "Window.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include "windows/WindowsVersion.hpp"

static void pushSize(lua_State* L, lua_Number const width, lua_Number const height) {
	lua::stack_t S(L);
	auto const index = S.create_map(2);
	S.set_map_value(index, "width", width);
	S.set_map_value(index, "height", height);
}

static int compat_SetSplash(lua_State* L)
{
	LAPP.SetSplash(lua_toboolean(L, 1));
	return 0;
}
static int compat_SetTitle(lua_State* L)
{
	LAPP.SetTitle(luaL_checkstring(L, 1));
	return 0;
}

namespace luastg::binding {

	std::string_view Window::class_name{ "lstg.Window" };

	struct WindowBinding : public Window {

		// meta methods

		static int __gc(lua_State* L) {
			auto self = as(L, 1);
			if (self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		static int __eq(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			if (is(L, 2)) {
				auto other = as(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		// instance methods

		static int setTitle(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const text = S.get_value<std::string_view>(2);
			self->data->setTitleText(text);
			return 0;
		}

		static int getClientAreaSize(lua_State* L) {
			auto self = as(L, 1);
			auto const size = self->data->_getCurrentSize();
			pushSize(L, size.x, size.y);
			return 1;
		}

		static int getStyle(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const style = self->data->getFrameStyle();
			S.push_value(static_cast<int32_t>(style));
			return 1;
		}

		static int getDisplayScale(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			S.push_value(self->data->getDPIScaling());
			return 1;
		}

		static int setWindowed(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const width = S.get_value<uint32_t>(2);
			auto const height = S.get_value<uint32_t>(3);
			auto const size = core::Vector2U(width, height);
			auto style = self->data->getFrameStyle();
			if (S.is_number(4)) {
				style = static_cast<core::WindowFrameStyle>(S.get_value<int32_t>(4));
			}
			if (Display::is(L, 5)) {
				auto display = Display::as(L, 5);
				self->data->setWindowMode(size, style, display->data);
			}
			else {
				self->data->setWindowMode(size, style);
			}
			return 0;
		}

		static int setFullscreen(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			if (Display::is(L, 2)) {
				auto display = Display::as(L, 2);
				self->data->setFullScreenMode(display->data);
			}
			else {
				self->data->setFullScreenMode();
			}
			return 0;
		}

		static int getCursorVisibility(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			S.push_value(self->data->getCursor() != core::WindowCursor::None);
			return 1;
		}

		static int setCursorVisibility(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const visible = S.get_value<bool>(2);
			self->data->setCursor(visible ? core::WindowCursor::Arrow : core::WindowCursor::None);
			return 0;
		}

		// extension

		static int queryInterface(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(2);
			if (name == Window_InputMethodExtension::class_name) {
				auto ext = Window_InputMethodExtension::create(L);
				ext->data = self->data;
				ext->data->retain();
				return 1;
			}
			if (name == Window_TextInputExtension::class_name) {
				auto ext = Window_TextInputExtension::create(L);
				ext->data = self->data;
				ext->data->retain();
				return 1;
			}
			if (name == Window_Windows11Extension::class_name) {
				if (Platform::WindowsVersion::Is11()) {
					auto ext = Window_Windows11Extension::create(L);
					ext->data = self->data;
					ext->data->retain();
					return 1;
				}
			}
			S.push_value(std::nullopt);
			return 1;
		}

		// static methods

		static int getMain(lua_State* L) {
			lua::stack_t S(L);
			auto self = create(L);
			self->data = LAPP.getWindow();
			self->data->retain();
			return 1;
		}

	};

	bool Window::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	Window* Window::as(lua_State* L, int index) {
		return static_cast<Window*>(luaL_checkudata(L, index, class_name.data()));
	}

	Window* Window::create(lua_State* L) {
		lua::stack_t S(L);
		auto self = S.create_userdata<Window>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}

	void Window::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// lstg.Window.FrameStyle

		{
			auto const e = S.create_module("lstg.Window.FrameStyle");
			S.set_map_value(e, "borderless", static_cast<int32_t>(core::WindowFrameStyle::None));
			S.set_map_value(e, "fixed", static_cast<int32_t>(core::WindowFrameStyle::Fixed));
			S.set_map_value(e, "normal", static_cast<int32_t>(core::WindowFrameStyle::Normal));
		}
		
		// method

		auto const method_table = S.create_module(class_name);
		S.set_map_value(method_table, "setTitle", &WindowBinding::setTitle);
		S.set_map_value(method_table, "getClientAreaSize", &WindowBinding::getClientAreaSize);
		S.set_map_value(method_table, "getStyle", &WindowBinding::getStyle);
		S.set_map_value(method_table, "getDisplayScale", &WindowBinding::getDisplayScale);
		S.set_map_value(method_table, "setWindowed", &WindowBinding::setWindowed);
		S.set_map_value(method_table, "setFullscreen", &WindowBinding::setFullscreen);
		S.set_map_value(method_table, "getCursorVisibility", &WindowBinding::getCursorVisibility);
		S.set_map_value(method_table, "setCursorVisibility", &WindowBinding::setCursorVisibility);
		S.set_map_value(method_table, "queryInterface", &WindowBinding::queryInterface);
		S.set_map_value(method_table, "getMain", &WindowBinding::getMain);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &WindowBinding::__gc);
		S.set_map_value(metatable, "__tostring", &WindowBinding::__tostring);
		S.set_map_value(metatable, "__eq", &WindowBinding::__eq);
		S.set_map_value(metatable, "__index", method_table);

		// register

		auto const module_table = S.push_module("lstg");
		S.set_map_value(module_table, "SetSplash", &compat_SetSplash);
		S.set_map_value(module_table, "SetTitle", &compat_SetTitle);
		//S.set_map_value(module_table, "Window", method_table);
	}

}

namespace luastg::binding {

	std::string_view Window_InputMethodExtension::class_name{ "lstg.Window.InputMethodExtension" };

	struct ImeExtBinding : public Window_InputMethodExtension {

		// meta methods

		static int __gc(lua_State* L) {
			auto self = as(L, 1);
			if (self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		// instance methods

		static int isInputMethodEnabled(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			S.push_value(self->data->getIMEState());
			return 1;
		}

		static int setInputMethodEnabled(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const enabled = S.get_value<bool>(2);
			self->data->setIMEState(enabled);
			return 0;
		}

		static int setInputMethodPosition(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const x = S.get_value<int32_t>(2);
			auto const y = S.get_value<int32_t>(3);
			self->data->setInputMethodPosition(core::Vector2I(x, y));
			return 0;
		}

	};

	bool Window_InputMethodExtension::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	Window_InputMethodExtension* Window_InputMethodExtension::as(lua_State* L, int index) {
		return static_cast<Window_InputMethodExtension*>(luaL_checkudata(L, index, class_name.data()));
	}

	Window_InputMethodExtension* Window_InputMethodExtension::create(lua_State* L) {
		lua::stack_t S(L);
		auto self = S.create_userdata<Window_InputMethodExtension>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}

	void Window_InputMethodExtension::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// method

		auto const method_table = S.create_module(class_name);
		S.set_map_value(method_table, "isInputMethodEnabled", &ImeExtBinding::isInputMethodEnabled);
		S.set_map_value(method_table, "setInputMethodEnabled", &ImeExtBinding::setInputMethodEnabled);
		S.set_map_value(method_table, "setInputMethodPosition", &ImeExtBinding::setInputMethodPosition);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &ImeExtBinding::__gc);
		S.set_map_value(metatable, "__tostring", &ImeExtBinding::__tostring);
		S.set_map_value(metatable, "__index", method_table);
	}

}

namespace luastg::binding {

	std::string_view Window_TextInputExtension::class_name{ "lstg.Window.TextInputExtension" };

	struct TextInputExtBinding : public Window_TextInputExtension {

		// meta methods

		static int __gc(lua_State* L) {
			auto self = as(L, 1);
			if (self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		// instance methods

		static int isEnabled(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			S.push_value(self->data->textInput_isEnabled());
			return 1;
		}

		static int setEnabled(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const enabled = S.get_value<bool>(2);
			self->data->textInput_setEnabled(enabled);
			return 0;
		}

		static int toString(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const buffer = self->data->textInput_getBuffer();
			S.push_value(buffer);
			return 1;
		}

		static int clear(lua_State* L) {
			auto self = as(L, 1);
			self->data->textInput_clearBuffer();
			return 0;
		}

		static int getCursorPosition(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const position = self->data->textInput_getCursorPosition();
			S.push_value(position);
			return 1;
		}

		static int setCursorPosition(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const position = S.get_value<uint32_t>(2);
			self->data->textInput_setCursorPosition(position);
			return 0;
		}

		static int addCursorPosition(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const offset = S.get_value<int32_t>(2);
			self->data->textInput_addCursorPosition(offset);
			return 0;
		}

		static int insert(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			if (S.is_string(3)) {
				auto const idx = S.get_value<uint32_t>(2);
				auto const str = S.get_value<std::string_view>(3);
				self->data->textInput_insertBufferRange(idx, str);
			}
			else {
				auto const str = S.get_value<std::string_view>(2);
				self->data->textInput_insertBufferRange(self->data->textInput_getCursorPosition(), str);
			}
			return 0;
		}

		static int remove(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const idx = S.get_value<uint32_t>(2, self->data->textInput_getCursorPosition());
			auto const cnt = S.get_value<uint32_t>(3, 1);
			self->data->textInput_removeBufferRange(idx, cnt);
			return 0;
		}

		static int backspace(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			auto const count = S.get_value<uint32_t>(2, 1);
			self->data->textInput_backspace(count);
			return 0;
		}

	};

	bool Window_TextInputExtension::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	Window_TextInputExtension* Window_TextInputExtension::as(lua_State* L, int index) {
		return static_cast<Window_TextInputExtension*>(luaL_checkudata(L, index, class_name.data()));
	}

	Window_TextInputExtension* Window_TextInputExtension::create(lua_State* L) {
		lua::stack_t S(L);
		auto self = S.create_userdata<Window_TextInputExtension>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}

	void Window_TextInputExtension::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// method

		auto const method_table = S.create_module(class_name);
		S.set_map_value(method_table, "isEnabled", &TextInputExtBinding::isEnabled);
		S.set_map_value(method_table, "setEnabled", &TextInputExtBinding::setEnabled);
		S.set_map_value(method_table, "toString", &TextInputExtBinding::toString);
		S.set_map_value(method_table, "clear", &TextInputExtBinding::clear);
		S.set_map_value(method_table, "getCursorPosition", &TextInputExtBinding::getCursorPosition);
		S.set_map_value(method_table, "setCursorPosition", &TextInputExtBinding::setCursorPosition);
		S.set_map_value(method_table, "addCursorPosition", &TextInputExtBinding::addCursorPosition);
		S.set_map_value(method_table, "insert", &TextInputExtBinding::insert);
		S.set_map_value(method_table, "remove", &TextInputExtBinding::remove);
		S.set_map_value(method_table, "backspace", &TextInputExtBinding::backspace);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &TextInputExtBinding::__gc);
		S.set_map_value(metatable, "__tostring", &TextInputExtBinding::__tostring);
		S.set_map_value(metatable, "__index", method_table);
	}

}

namespace luastg::binding {

	std::string_view Window_Windows11Extension::class_name{ "lstg.Window.Windows11Extension" };

	struct Win11ExtBinding : public Window_Windows11Extension {

		// meta methods

		static int __gc(lua_State* L) {
			auto self = as(L, 1);
			if (self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}

		static int __tostring(lua_State* L) {
			lua::stack_t S(L);
			[[maybe_unused]] auto self = as(L, 1);
			S.push_value(class_name);
			return 1;
		}

		static int __eq(lua_State* L) {
			lua::stack_t S(L);
			auto self = as(L, 1);
			if (is(L, 2)) {
				auto other = as(L, 2);
				S.push_value(self->data == other->data);
			}
			else {
				S.push_value(false);
			}
			return 1;
		}

		// instance methods

		static int setWindowCornerPreference(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const allow = S.get_value<bool>(2);
			self->data->setWindowCornerPreference(allow);
			return 0;
		}

		static int setTitleBarAutoHidePreference(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const allow = S.get_value<bool>(2);
			self->data->setTitleBarAutoHidePreference(allow);
			return 0;
		}

	};

	bool Window_Windows11Extension::is(lua_State* L, int index) {
		return nullptr != luaL_testudata(L, index, class_name.data());
	}

	Window_Windows11Extension* Window_Windows11Extension::as(lua_State* L, int index) {
		return static_cast<Window_Windows11Extension*>(luaL_checkudata(L, index, class_name.data()));
	}

	Window_Windows11Extension* Window_Windows11Extension::create(lua_State* L) {
		lua::stack_t S(L);
		auto self = S.create_userdata<Window_Windows11Extension>();
		auto const self_index = S.index_of_top();
		S.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}

	void Window_Windows11Extension::registerClass(lua_State* L) {
		[[maybe_unused]] lua::stack_balancer_t SB(L);
		lua::stack_t S(L);

		// method

		auto const method_table = S.create_module(class_name);
		S.set_map_value(method_table, "setWindowCornerPreference", &Win11ExtBinding::setWindowCornerPreference);
		S.set_map_value(method_table, "setTitleBarAutoHidePreference", &Win11ExtBinding::setTitleBarAutoHidePreference);

		// metatable

		auto const metatable = S.create_metatable(class_name);
		S.set_map_value(metatable, "__gc", &Win11ExtBinding::__gc);
		S.set_map_value(metatable, "__tostring", &Win11ExtBinding::__tostring);
		S.set_map_value(metatable, "__eq", &Win11ExtBinding::__eq);
		S.set_map_value(metatable, "__index", method_table);
	}

}
