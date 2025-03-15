#pragma once
#include "lua.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTG::Sub::LuaBinding {
	struct SpriteRenderer {
		static std::string_view const class_name;

		[[maybe_unused]] Core::Graphics::ISpriteRenderer* data{};
		[[maybe_unused]] Core::Vector2F position;
		[[maybe_unused]] Core::Vector2F scale;
		[[maybe_unused]] float rotation{};

		static bool is(lua_State* vm, int index);
		static SpriteRenderer* as(lua_State* vm, int index);
		static SpriteRenderer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
