#include "Display.hpp"
#include "Window.hpp"
#include "lua_utility.hpp"
#include "AppFrame.h"
#include "Platform/WindowsVersion.hpp"

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

namespace LuaSTG::Sub::LuaBinding {

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
			auto const size = Core::Vector2U(width, height);
			auto style = self->data->getFrameStyle();
			if (S.is_number(4)) {
				style = static_cast<Core::Graphics::WindowFrameStyle>(S.get_value<int32_t>(4));
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
			S.push_value(self->data->getCursor() != Core::Graphics::WindowCursor::None);
			return 1;
		}

		static int setCursorVisibility(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const visible = S.get_value<bool>(2);
			self->data->setCursor(visible ? Core::Graphics::WindowCursor::Arrow : Core::Graphics::WindowCursor::None);
			return 0;
		}

		// extension

		static int queryInterface(lua_State* L) {
			auto self = as(L, 1);
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(2);
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
			self->data = LuaSTGPlus::AppFrame::GetInstance().GetAppModel()->getWindow();
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
			auto const e = S.push_module("lstg.Window.FrameStyle");
			S.set_map_value(e, "borderless", static_cast<int32_t>(Core::Graphics::WindowFrameStyle::None));
			S.set_map_value(e, "fixed", static_cast<int32_t>(Core::Graphics::WindowFrameStyle::Fixed));
			S.set_map_value(e, "normal", static_cast<int32_t>(Core::Graphics::WindowFrameStyle::Normal));
		}
		
		// method

		auto const method_table = S.push_module(class_name);
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

namespace LuaSTG::Sub::LuaBinding {

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

		auto const method_table = S.push_module(class_name);
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
