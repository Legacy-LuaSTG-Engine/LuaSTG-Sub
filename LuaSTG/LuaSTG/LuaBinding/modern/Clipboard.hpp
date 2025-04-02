#pragma once
#include "lua.hpp"

namespace luastg::binding {

	struct Clipboard {

		static std::string_view const class_name;

		static void registerClass(lua_State* L);

	};

}
