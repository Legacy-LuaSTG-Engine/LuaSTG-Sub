#pragma once
#include "lua.hpp"
#include "core/Graphics/Sprite.hpp"

namespace luastg::binding {
	struct Sprite {
		static std::string_view const class_name;

		[[maybe_unused]] core::Graphics::ISprite* data{};

		static bool is(lua_State* vm, int index);
		static Sprite* as(lua_State* vm, int index);
		static Sprite* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
