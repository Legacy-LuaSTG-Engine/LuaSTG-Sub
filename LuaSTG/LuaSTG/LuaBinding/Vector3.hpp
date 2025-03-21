#pragma once
#include "lua.hpp"
#include "Core/Type.hpp"

namespace LuaSTG::Sub::LuaBinding {
	struct Vector3 {
		static std::string_view class_name;

		Core::Vector3<lua_Number> data;

		static bool is(lua_State* vm, int index);
		static Vector3* as(lua_State* vm, int index);
		static Vector3* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
