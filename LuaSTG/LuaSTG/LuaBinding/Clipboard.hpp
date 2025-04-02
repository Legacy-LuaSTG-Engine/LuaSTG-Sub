#pragma once
#include "lua.hpp"

namespace LuaSTG::Sub::LuaBinding {

	struct Clipboard {

		static std::string_view const class_name;

		static void registerClass(lua_State* L);

	};

}
