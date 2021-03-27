#pragma once
#include "lua.hpp"

namespace LuaSTGPlus
{
	struct GameObjectClass
	{
		static const char* IS_CLASS;
		static const char* IS_RENDER_CLASS;
		static const char* DEFAULT_FUNCTION;
		
		bool IsDefaultUpdate = false;
		bool IsDefaultRender = false;
		bool IsRenderClass   = false;
		
		void Reset();
		bool CheckClassClass(lua_State* L, int index);
		bool CheckGameObjectClass(lua_State* L, int index);
		
		static bool CheckClassValid(lua_State* L, int index);
	};
};
