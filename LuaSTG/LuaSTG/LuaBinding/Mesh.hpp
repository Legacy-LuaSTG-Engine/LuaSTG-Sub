#pragma once
#include "lua.hpp"
#include "Core/Graphics/Mesh.hpp"

namespace LuaSTG::Sub::LuaBinding {
	struct Mesh {
		static std::string_view class_name;

		[[maybe_unused]] Core::Graphics::IMesh* data{};

		static bool is(lua_State* vm, int index);
		static Mesh* as(lua_State* vm, int index);
		static Mesh* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
