#pragma once
#include "lua.hpp"

#define LGOBJ_CC_INIT 1
#define LGOBJ_CC_DEL 2
#define LGOBJ_CC_FRAME 3
#define LGOBJ_CC_RENDER 4
#define LGOBJ_CC_COLLI 5
#define LGOBJ_CC_KILL 6

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
