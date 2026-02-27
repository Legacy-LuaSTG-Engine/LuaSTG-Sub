#pragma once
#include "core/Texture2D.hpp"
#include "lua.hpp"
#include <string_view>

namespace luastg::binding {
	struct Video {
		static const std::string_view class_name;

		core::ITexture2D* data{ nullptr };

		static bool is(lua_State* vm, int index);
		static Video* as(lua_State* vm, int index);
		static Video* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
