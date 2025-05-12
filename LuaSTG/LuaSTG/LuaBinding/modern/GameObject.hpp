#pragma once
#include "GameObject/GameObject.hpp"
#include "lua.hpp"

namespace luastg::binding {

	struct GameObject {

		static std::string_view const class_name;

		static bool is(lua_State* vm, int index);

		static luastg::GameObject* as(lua_State* vm, int index);

		static int pushGameObjectTable(lua_State* vm);

		static void registerClass(lua_State* vm);

	};

}
