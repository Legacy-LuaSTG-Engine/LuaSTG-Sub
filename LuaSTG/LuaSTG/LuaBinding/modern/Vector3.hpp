#pragma once
#include "lua.hpp"
#include "core/Vector3.hpp"

namespace luastg::binding {
	struct Vector3 {
		static std::string_view class_name;

		core::Vector3<lua_Number> data;

		static bool is(lua_State* vm, int index);
		static Vector3* as(lua_State* vm, int index);
		static Vector3* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
