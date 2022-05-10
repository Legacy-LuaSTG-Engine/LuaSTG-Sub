#include "GameObjectClass.hpp"
#include "LuaWrapper/lua_utility.hpp"

namespace LuaSTGPlus
{
	constexpr char const IS_CLASS[] = "is_class";
	constexpr char const IS_RENDER_CLASS[] = ".render";
	constexpr char const DEFAULT_FUNCTION[] = "default_function";
	
	void GameObjectClass::Reset()
	{
		IsDefaultUpdate = false;
		IsDefaultRender = false;
		IsRenderClass = false;
	}
	
	bool GameObjectClass::CheckClassClass(lua_State* L, int index)
	{
		Reset();
		//													// ??? class ???
		
		// default function
		lua_getfield(L, index, DEFAULT_FUNCTION);			// ??? class ??? ? 
		if (lua_isnumber(L, -1))
		{
			lua_Integer const mask = lua_tointeger(L, -1);	// ??? class ??? n 
			if (mask & (1 << LGOBJ_CC_FRAME))
			{
				IsDefaultUpdate = true;
			}
			if (mask & (1 << LGOBJ_CC_RENDER))
			{
				IsDefaultRender = true;
			}
		}
		lua_pop(L, 1);										// ??? class ??? 
		
		// render class
		lua_getfield(L, index, IS_RENDER_CLASS);			// ??? class ??? ? 
		IsRenderClass = lua_to_uint8_boolean(L, -1);		// ??? class ??? b 
		lua_pop(L, 1);										// ??? class ??? 
		
		return true;
	}
	
	bool GameObjectClass::CheckGameObjectClass(lua_State* L, int index)
	{
		// ??? object ???
		lua_rawgeti(L, index, 1);	// ??? object ??? class 
		const bool ret = CheckClassClass(L, -1);
		lua_pop(L, 1);				// ??? object ??? 
		return ret;
	}
	
	bool GameObjectClass::CheckClassValid(lua_State* L, int index)
	{
		if (!lua_istable(L, index))
		{
			return false;
		}
		lua_getfield(L, index, IS_CLASS);
		bool const is_class = lua_toboolean(L, -1);
		lua_pop(L, 1);
		return is_class;
	}
};
