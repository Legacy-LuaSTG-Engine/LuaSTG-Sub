#pragma once
#include "lua.hpp"
#include "core/Vector2.hpp"

namespace luastg::binding {
	struct Vector2 {
		static std::string_view class_name;

		core::Vector2<lua_Number> data;

		static bool is(lua_State* vm, int index);
		static Vector2* as(lua_State* vm, int index);
		static Vector2* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
