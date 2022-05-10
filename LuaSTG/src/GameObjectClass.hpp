#pragma once
#include <cstdint>
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
		uint8_t IsDefaultUpdate = false;
		uint8_t IsDefaultRender = false;
		uint8_t IsRenderClass   = false;
		uint8_t _Padding = 0;
		
		void Reset();
		bool CheckClassClass(lua_State* L, int index);
		bool CheckGameObjectClass(lua_State* L, int index);
		
		static bool CheckClassValid(lua_State* L, int index);
	};
};
