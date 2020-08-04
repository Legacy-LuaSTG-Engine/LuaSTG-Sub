#pragma once
#include "Global.h"

#define LOBJ_CLASS_ISCLASS         "is_class"
#define LOBJ_CLASS_RENDERCLASS     ".render"
#define LOBJ_CLASS_DEFAULTFUNCTION "default_function"

namespace LuaSTGPlus {
	struct GameObjectClass {
		bool IsDefaultUpdate;
		bool IsDefaultRender;
		bool IsRenderClass;

		void Reset() {
			IsDefaultUpdate = false;
			IsDefaultRender = false;
			IsRenderClass = false;
		}

		static inline bool CheckClassValid(lua_State* L, int index) {
			if (!lua_istable(L, index)) {
				return false;
			}
			lua_getfield(L, index, LOBJ_CLASS_ISCLASS);			// ??? class ??? ? 
			if (!lua_toboolean(L, -1)) {
				return false;
			}
			lua_pop(L, 1);										// ??? class ??? 
			return true;
		}

		bool CheckClassClass(lua_State* L, int index) {
			Reset(); // clear info first
			// ??? class ???

			// default function
			lua_getfield(L, index, LOBJ_CLASS_DEFAULTFUNCTION);	// ??? class ??? ? 
			if (lua_isnumber(L, -1)) {
				lua_Integer mask = lua_tointeger(L, -1);		// ??? class ??? n 
				if (mask & (1 << LGOBJ_CC_FRAME)) {
					IsDefaultUpdate = true;
				}
				if (mask & (1 << LGOBJ_CC_RENDER)) {
					IsDefaultRender = true;
				}
			}
			lua_pop(L, 1);										// ??? class ??? 

			// render class
			lua_getfield(L, index, LOBJ_CLASS_RENDERCLASS);		// ??? class ??? ? 
			IsRenderClass = lua_toboolean(L, -1);				// ??? class ??? b 
			lua_pop(L, 1);										// ??? class ??? 

			return true;
		}

		bool CheckGameObjectClass(lua_State* L, int index) {
			// ??? object ???
			lua_rawgeti(L, index, 1);	// ??? object ??? class 
			auto ret = CheckClassClass(L, -1);
			lua_pop(L, 1);				// ??? object ??? 
			return ret;
		}
	};
}
