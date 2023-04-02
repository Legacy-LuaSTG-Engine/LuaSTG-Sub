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
		union
		{
			struct
			{
				uint32_t IsDefaultCreate : 1;
				uint32_t IsDefaultDestroy : 1;
				uint32_t IsDefaultUpdate : 1;
				uint32_t IsDefaultRender : 1;
				uint32_t IsDefaultTrigger : 1;
				uint32_t IsDefaultLegacyKill : 1;
				uint32_t IsRenderClass : 1;
			};
			uint32_t __Value{};
		};
		
		inline void Reset() noexcept { __Value = 0; }
		bool CheckClassClass(lua_State* L, int index);
		bool CheckGameObjectClass(lua_State* L, int index);
		
		static bool CheckClassValid(lua_State* L, int index);
	};

	static_assert(sizeof(GameObjectClass) == sizeof(uint32_t));
};
