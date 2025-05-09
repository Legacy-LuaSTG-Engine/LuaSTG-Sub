#pragma once
#include "Core/Graphics/Renderer.hpp"
#include "lua.hpp"

namespace luastg::binding
{
	class PostEffectShader
	{
	public:
		static void Register(lua_State* L);
		static void Create(lua_State* L, core::Graphics::IPostEffectShader* p_shader);
		static core::Graphics::IPostEffectShader* Cast(lua_State* L, int idx);
	};
}
