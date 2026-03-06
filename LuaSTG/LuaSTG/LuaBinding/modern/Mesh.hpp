#pragma once
#include "lua.hpp"
#include "core/Graphics/Mesh.hpp"

namespace luastg::binding {
	struct Mesh {
		static std::string_view class_name;

		[[maybe_unused]] core::Graphics::IMesh* data{};

		static bool is(lua_State* vm, int index);
		static Mesh* as(lua_State* vm, int index);
		static Mesh* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
