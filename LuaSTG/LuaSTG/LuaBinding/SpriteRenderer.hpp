#pragma once
#include "lua.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTG::Sub::LuaBinding {
	struct SpriteRenderer {
		static std::string_view const class_name;

		Core::Graphics::ISpriteRenderer* data{};
		Core::Vector2F position;
		Core::Vector2F scale;
		float rotation{};
		bool is_dirty{};

		static bool is(lua_State* vm, int index);
		static SpriteRenderer* as(lua_State* vm, int index);
		static SpriteRenderer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};

	struct SpriteRectRenderer {
		static std::string_view const class_name;

		Core::Graphics::ISpriteRenderer* data{};

		static bool is(lua_State* vm, int index);
		static SpriteRectRenderer* as(lua_State* vm, int index);
		static SpriteRectRenderer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};

	struct SpriteQuadRenderer {
		static std::string_view const class_name;

		Core::Graphics::ISpriteRenderer* data{};

		static bool is(lua_State* vm, int index);
		static SpriteQuadRenderer* as(lua_State* vm, int index);
		static SpriteQuadRenderer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
