#include "LuaWrapper/LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::GameObjectManagerWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		// 高级方法
		static int ObjTable(lua_State* L) noexcept
		{
			return LPOOL.GetObjectTable(L);
		}
		// 对象管理器
		static int GetnObj(lua_State* L) noexcept
		{
			lua_pushinteger(L, (lua_Integer)LPOOL.GetObjectCount());
			return 1;
		}
		static int ObjFrame(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoFrame();
			return 0;
		}
		static int ObjRender(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoRender();
			return 0;
		}
		static int BoundCheck(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.BoundCheck();
			return 0;
		}
		static int SetBound(lua_State* L) noexcept
		{
			LPOOL.SetBound(
				luaL_checknumber(L, 1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4)
			);
			return 0;
		}
		static int CollisionCheck(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.CollisionCheck(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
			return 0;
		}
		static int UpdateXY(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.UpdateXY();
			return 0;
		}
		static int AfterFrame(lua_State* L) noexcept
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.AfterFrame();
			return 0;
		}
		static int ResetPool(lua_State* L) noexcept
		{
			std::ignore = L;
			LPOOL.ResetPool();
			return 0;
		}
		// EX+ 对象更新相关，影响 frame 回调函数以及对象更新
		static int GetSuperPause(lua_State* L) noexcept
		{
			lua_pushinteger(L, LPOOL.GetNextFrameSuperPauseTime());
			return 1;
		}
		static int SetSuperPause(lua_State* L) noexcept
		{
			LPOOL.SetNextFrameSuperPauseTime(luaL_checkinteger(L, 1));
			return 0;
		}
		static int AddSuperPause(lua_State* L) noexcept
		{
			lua_Integer a = luaL_checkinteger(L, 1);
			LPOOL.SetNextFrameSuperPauseTime(LPOOL.GetNextFrameSuperPauseTime() + a);
			return 0;
		}
		static int GetCurrentSuperPause(lua_State* L) noexcept
		{
			lua_pushinteger(L, LPOOL.GetSuperPauseTime());
			return 1;
		}
		// EX+ multi world  world mask
		static int GetWorldFlag(lua_State* L) noexcept
		{
			lua_pushinteger(L, LPOOL.GetWorldFlag());
			return 1;
		}
		static int SetWorldFlag(lua_State* L) noexcept
		{
			LPOOL.SetWorldFlag(luaL_checkinteger(L, 1));
			return 1;
		}
		static int IsSameWorld(lua_State* L) noexcept
		{
			int a = luaL_checkinteger(L, 1);
			int b = luaL_checkinteger(L, 2);
			lua_pushboolean(L, GameObjectPool::CheckWorld(a, b));
			return 1;
		}
		static int CheckWorlds(lua_State* L) noexcept
		{
			int a1 = luaL_checkinteger(L, 1);
			int a2 = luaL_checkinteger(L, 2);
			lua_pushboolean(L, LPOOL.CheckWorlds(a1, a2));
			return 1;
		}
		static int ActiveWorlds(lua_State* L) noexcept
		{
			int a1 = luaL_optinteger(L, 1, 0);
			int a2 = luaL_optinteger(L, 2, 0);
			int a3 = luaL_optinteger(L, 3, 0);
			int a4 = luaL_optinteger(L, 4, 0);
			LPOOL.ActiveWorlds(a1, a2, a3, a4);
			return 0;
		}
		// EX+
		static int GetCurrentObject(lua_State* L) noexcept
		{
			return LPOOL.PushCurrentObject(L);
		}
	};

	luaL_Reg const lib[] = {
		// 高级方法
		{ "ObjTable", &Wrapper::ObjTable },
		// 对象管理器
		{ "GetnObj", &Wrapper::GetnObj },
		{ "ObjFrame", &Wrapper::ObjFrame },
		{ "ObjRender", &Wrapper::ObjRender },
		{ "BoundCheck", &Wrapper::BoundCheck },
		{ "SetBound", &Wrapper::SetBound },
		{ "CollisionCheck", &Wrapper::CollisionCheck },
		{ "UpdateXY", &Wrapper::UpdateXY },
		{ "AfterFrame", &Wrapper::AfterFrame },
		{ "ResetPool", &Wrapper::ResetPool },
		// 对象遍历
		{ "NextObject", &GameObjectPool::api_NextObject },
		{ "ObjList", &GameObjectPool::api_ObjList },
		// 对象控制函数
		{ "New", &GameObjectPool::api_New },
		{ "ResetObject", &GameObjectPool::api_ResetObject },
		{ "Del", &GameObjectPool::api_Del },
		{ "Kill", &GameObjectPool::api_Kill },
		{ "IsValid", &GameObjectPool::api_IsValid },
		{ "BoxCheck", &GameObjectPool::api_BoxCheck },
		{ "ColliCheck", &GameObjectPool::api_ColliCheck },
		{ "Angle", &GameObjectPool::api_Angle },
		{ "Dist", &GameObjectPool::api_Dist },
		{ "GetV", &GameObjectPool::api_GetV },
		{ "SetV", &GameObjectPool::api_SetV },
		// 对象属性访问
		{ "GetAttr", &GameObjectPool::api_GetAttr },
		{ "SetAttr", &GameObjectPool::api_SetAttr },
		// 对象默认回调函数
		{ "DefaultRenderFunc", &GameObjectPool::api_DefaultRenderFunc },
		// 对象资源控制
		{ "SetImgState", &GameObjectPool::api_SetImgState },
		{ "SetParState", &GameObjectPool::api_SetParState },
		{ "ParticleStop", &GameObjectPool::api_ParticleStop },
		{ "ParticleFire", &GameObjectPool::api_ParticleFire },
		{ "ParticleGetn", &GameObjectPool::api_ParticleGetn },
		{ "ParticleGetEmission", &GameObjectPool::api_ParticleGetEmission },
		{ "ParticleSetEmission", &GameObjectPool::api_ParticleSetEmission },
		// EX+
		{ "GetSuperPause", &Wrapper::GetSuperPause },
		{ "SetSuperPause", &Wrapper::SetSuperPause },
		{ "AddSuperPause", &Wrapper::AddSuperPause },
		{ "GetCurrentSuperPause", &Wrapper::GetCurrentSuperPause },
		{ "GetWorldFlag", &Wrapper::GetWorldFlag },
		{ "SetWorldFlag", &Wrapper::SetWorldFlag },
		{ "IsInWorld", &Wrapper::IsSameWorld },
		{ "IsSameWorld", &Wrapper::CheckWorlds },
		{ "ActiveWorlds", &Wrapper::ActiveWorlds },
		{ "GetCurrentObject", &Wrapper::GetCurrentObject },
		{ NULL, NULL },
	};

	luaL_Reg const lib_empty[] = {
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);                      // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".GameObjectManager", lib); // ??? lstg lstg.GameObjectManager
	lua_setfield(L, -1, "GameObjectManager");                       // ??? lstg
	lua_pop(L, 1);                                                  // ???
}
