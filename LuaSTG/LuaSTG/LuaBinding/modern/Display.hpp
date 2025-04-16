#pragma once
#include <string_view>
#include "lua.hpp"
#include "Core/Graphics/Window.hpp"

namespace luastg::binding {
	
	struct Display {

		static std::string_view class_name;

		[[maybe_unused]] core::Graphics::IDisplay* data{};

		static bool is(lua_State* L, int index);

		static Display* as(lua_State* L, int index);

		static Display* create(lua_State* L);

		static void registerClass(lua_State* L);

	};

}
