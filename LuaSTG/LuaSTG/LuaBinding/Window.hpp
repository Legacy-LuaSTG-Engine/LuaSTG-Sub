#pragma once
#include <string_view>
#include "lua.hpp"
#include "Core/Graphics/Window.hpp"

namespace LuaSTG::Sub::LuaBinding {

	struct Window {

		static std::string_view class_name;

		[[maybe_unused]] Core::Graphics::IWindow* data{};

		static bool is(lua_State* L, int index);

		static Window* as(lua_State* L, int index);

		static Window* create(lua_State* L);

		static void registerClass(lua_State* L);

	};

	struct Window_Windows11Extension {

		static std::string_view class_name;

		[[maybe_unused]] Core::Graphics::IWindow* data{};

		static bool is(lua_State* L, int index);

		static Window_Windows11Extension* as(lua_State* L, int index);

		static Window_Windows11Extension* create(lua_State* L);

		static void registerClass(lua_State* L);

	};

}
