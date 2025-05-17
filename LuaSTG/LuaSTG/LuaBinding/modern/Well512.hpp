#pragma once
#include "lua.hpp"
#include "Utility/well512.hpp"

namespace luastg::binding {
	struct Well512 {
		static std::string_view const class_name;

		random::well512 well512;

		static bool is(lua_State* vm, int index);
		static random::well512* as(lua_State* vm, int index);
		static random::well512* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
