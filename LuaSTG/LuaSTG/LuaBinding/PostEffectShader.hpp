#pragma once
#include "Core/Graphics/Renderer.hpp"
#include "lua.hpp"

namespace LuaSTG::LuaBinding
{
	class PostEffectShader
	{
	public:
		static void Register(lua_State* L);
		static void Create(lua_State* L, Core::Graphics::IPostEffectShader* p_shader);
		static Core::Graphics::IPostEffectShader* Cast(lua_State* L, int idx);
	};
}
