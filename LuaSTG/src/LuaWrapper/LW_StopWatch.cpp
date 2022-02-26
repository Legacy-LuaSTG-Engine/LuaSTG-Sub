#include "LuaWrapper/LuaWrapper.hpp"

namespace LuaSTGPlus
{
	namespace LuaWrapper
	{
		void StopWatchWrapper::Register(lua_State* L)LNOEXCEPT
		{
			struct Function
			{
#define GETUDATA(p, i) fcyStopWatch* (p) = static_cast<fcyStopWatch*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_STOPWATCH));
				static int Reset(lua_State* L)
				{
					GETUDATA(p, 1);
					p->Reset();
					return 1;
				}
				static int Pause(lua_State* L)
				{
					GETUDATA(p, 1);
					p->Pause();
					return 1;
				}
				static int Resume(lua_State* L)
				{
					GETUDATA(p, 1);
					p->Resume();
					return 1;
				}
				static int GetElapsed(lua_State* L)
				{
					GETUDATA(p, 1);
					lua_pushnumber(L, (lua_Number)p->GetElapsed());
					return 1;
				}
				static int Meta_ToString(lua_State* L)LNOEXCEPT
				{
					::lua_pushfstring(L, LUASTG_LUA_TYPENAME_STOPWATCH);
					return 1;
				}
#undef GETUDATA
			};

			luaL_Reg tMethods[] =
			{
				{ "Reset", &Function::Reset },
				{ "Pause", &Function::Pause },
				{ "Resume", &Function::Resume },
				{ "GetElapsed", &Function::GetElapsed },
				{ NULL, NULL }
			};
			luaL_Reg tMetaTable[] =
			{
				{ "__tostring", &Function::Meta_ToString },
				{ NULL, NULL }
			};
			
			RegisterClassIntoTable(L, ".StopWatch", tMethods, LUASTG_LUA_TYPENAME_STOPWATCH, tMetaTable);
		}

		void StopWatchWrapper::CreateAndPush(lua_State* L)
		{
			fcyStopWatch* p = static_cast<fcyStopWatch*>(lua_newuserdata(L, sizeof(fcyStopWatch))); // udata
			new(p) fcyStopWatch();
			luaL_getmetatable(L, LUASTG_LUA_TYPENAME_STOPWATCH); // udata mt
			lua_setmetatable(L, -2); // udata
		}

	}
}
