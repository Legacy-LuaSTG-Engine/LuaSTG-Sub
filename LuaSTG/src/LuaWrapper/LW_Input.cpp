#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::InputWrapper::Register(lua_State* L) LNOEXCEPT
{
	struct Wrapper
	{
		static int GetKeyboardState(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetMouseState(lua_State* L)LNOEXCEPT
		{
			//参数为整数，索引从1到8，返回bool值，索引1到3分别为鼠标左中右键，有的鼠标可能没有中键，剩下的按键看实际硬件情况
			lua_pushboolean(L, LAPP.GetMouseState(luaL_checkinteger(L, 1)));
			return 1;
		}
		static int GetMousePosition(lua_State* L)LNOEXCEPT
		{
			fcyVec2 tPos = LAPP.GetMousePosition(lua_toboolean(L, 1));
			lua_pushnumber(L, tPos.x);
			lua_pushnumber(L, tPos.y);
			return 2;
		}
		static int GetMouseWheelDelta(lua_State* L)LNOEXCEPT
		{
			//返回有符号整数，每次获取的是自上次获取以来所有的滚轮操作，有的鼠标可能没有滚轮
			lua_pushinteger(L, (lua_Integer)LAPP.GetMouseWheelDelta());
			return 1;
		}
		// 应该废弃的方法
		static int GetLastKey(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LAPP.GetLastKey());
			return 1;
		}
		static int GetKeyboardStateETC(lua_State* L)LNOEXCEPT
		{
			//检查键盘按键是否按下，Dinput KeyCode
			lua_pushboolean(L, LAPP.GetKeyboardState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetAsyncKeyStateETC(lua_State* L)LNOEXCEPT
		{
			//检查键盘按键是否按下，使用微软VKcode，通过GetAsyncKeyState函数获取
			//和GetKeyboardState不同，这个检测的不是按下过的，而是现在被按住的键
			lua_pushboolean(L, LAPP.GetAsyncKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
	};

	luaL_Reg lib_compat[] = {
		{ "GetKeyState", &Wrapper::GetKeyboardState },
		{ "GetMousePosition", &Wrapper::GetMousePosition },
		{ "GetMouseWheelDelta", &Wrapper::GetMouseWheelDelta },
		{ "GetMouseState", &Wrapper::GetMouseState },
		// 应该废弃的方法
		{ "GetLastKey", &Wrapper::GetLastKey },
		{ "GetKeyboardState", &Wrapper::GetKeyboardStateETC },
		{ "GetAsyncKeyState", &Wrapper::GetAsyncKeyStateETC },
		{NULL, NULL},
	};

	luaL_Reg lib[] = {
		{ "GetKeyboardState", &Wrapper::GetKeyboardState },
		{ "GetMouseState", &Wrapper::GetMouseState },
		{ "GetMousePosition", &Wrapper::GetMousePosition },
		{ "GetMouseWheelDelta", &Wrapper::GetMouseWheelDelta },
		{NULL, NULL},
	};
	
	luaL_register(L, LUASTG_LUA_LIBNAME, lib_compat);   // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Input", lib); // ??? lstg lstg.Input
	lua_setfield(L, -1, "Input");                       // ??? lstg
	lua_pop(L, 1);                                      // ???
}
