#include "LuaBinding/LuaWrapper.hpp"
#include "fcyMisc/fcyRandom.h"

namespace LuaSTGPlus::LuaWrapper
{
	void RandomizerWrapper::Register(lua_State* L)noexcept
	{
		struct Function
		{
		#define GETUDATA(p, i) fcyRandomWELL512* (p) = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_RANDGEN));
			static int Seed(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				p->SetSeed((uint32_t)luaL_checkinteger(L, 2));
				return 0;
			}
			static int GetSeed(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				lua_pushinteger(L, (lua_Integer)p->GetRandSeed());
				return 1;
			}
			static int Int(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				lua_Integer a = luaL_checkinteger(L, 2);
				lua_Integer b = luaL_checkinteger(L, 3);
				lua_Integer ret = (lua_Integer)p->GetRandUInt((std::max)(static_cast<uint32_t>(b - a), 0U));
				lua_pushinteger(L, a + ret);
				return 1;
			}
			static int Float(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				float a = (float)luaL_checknumber(L, 2);
				float b = (float)luaL_checknumber(L, 3);
				lua_pushnumber(L, (lua_Number)p->GetRandFloat(a, b));
				return 1;
			}
			static int Sign(lua_State* L)noexcept
			{
				GETUDATA(p, 1);
				lua_pushinteger(L, (lua_Integer)p->GetRandUInt(1) * 2 - 1);
				return 1;
			}
			static int Meta_ToString(lua_State* L)noexcept
			{
				lua_pushfstring(L, LUASTG_LUA_TYPENAME_RANDGEN);
				return 1;
			}
		#undef GETUDATA
		};

		luaL_Reg tMethods[] =
		{
			{ "Seed", &Function::Seed },
			{ "GetSeed", &Function::GetSeed },
			{ "Int", &Function::Int },
			{ "Float", &Function::Float },
			{ "Sign", &Function::Sign },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] =
		{
			{ "__tostring", &Function::Meta_ToString },
			{ NULL, NULL }
		};

		RegisterClassIntoTable(L, ".Rand", tMethods, LUASTG_LUA_TYPENAME_RANDGEN, tMetaTable);
	}

	void RandomizerWrapper::CreateAndPush(lua_State* L)
	{
		fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(lua_newuserdata(L, sizeof(fcyRandomWELL512))); // udata
		new(p) fcyRandomWELL512();
		luaL_getmetatable(L, LUASTG_LUA_TYPENAME_RANDGEN); // udata mt
		lua_setmetatable(L, -2); // udata
	}
}
