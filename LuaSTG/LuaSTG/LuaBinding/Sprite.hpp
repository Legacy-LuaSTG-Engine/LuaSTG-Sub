#pragma once
#include "lua.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTG::Sub::LuaBinding {
	struct Sprite {
		static std::string_view const class_name;

		[[maybe_unused]] Core::Graphics::ISprite* data{};

		static bool is(lua_State* vm, int index);
		static Sprite* as(lua_State* vm, int index);
		static Sprite* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
