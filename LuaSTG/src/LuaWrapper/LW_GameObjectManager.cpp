#include "LuaWrapper/LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::GameObjectManagerWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
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
				luaL_checkinteger(L, 1),
				luaL_checkinteger(L, 2),
				luaL_checkinteger(L, 3),
				luaL_checkinteger(L, 4)
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
		static int ResetObject(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "Invalid lstg.GameObject for 'ResetObject'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.DirtResetObject((size_t)luaL_checkinteger(L, -1)))
				return luaL_error(L, "invalid lstg.GameObject for 'ResetObject'.");
			return 0;
		}
		static int New(lua_State* L) noexcept
		{
			return LPOOL.New(L);
		}
		static int Del(lua_State* L) noexcept
		{
			return LPOOL.Del(L);
		}
		static int Kill(lua_State* L) noexcept
		{
			return LPOOL.Kill(L);
		}
		static int IsValid(lua_State* L) noexcept
		{
			return LPOOL.IsValid(L);
		}
		static int Angle(lua_State* L) noexcept
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) ??? id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) ??? id id
				double tRet;
				if (!LPOOL.Angle((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_pushnumber(L, tRet);
			}
			else if (lua_gettop(L) == 3) {
				// a b c ，此时是未知的
				if (lua_istable(L, 1)) {
					//如果第一个参数是object
					lua_rawgeti(L, 1, 2);  // t(object) x y ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Angle'.");
					}
					lua_pushnumber(L,
						atan2(luaL_checknumber(L, 3) - p->y, luaL_checknumber(L, 2) - p->x) * LRAD2DEGREE
					);
				}
				else if (lua_istable(L, 3)) {
					//如果第三个参数是object
					lua_rawgeti(L, 3, 2);  // x y t(object) ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Angle'.");
					}
					lua_pushnumber(L,
						atan2(p->y - luaL_checknumber(L, 2), p->x - luaL_checknumber(L, 1)) * LRAD2DEGREE
					);
				}
				else {
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				}
			}
			else
			{
				lua_pushnumber(L, 
					atan2(luaL_checknumber(L, 4) - luaL_checknumber(L, 2), luaL_checknumber(L, 3) - luaL_checknumber(L, 1)) * LRAD2DEGREE
				);
			}
			return 1;
		}
		static int ColliCheck(lua_State* L) {
			//对两个对象进行单独的碰撞检测
			// t(object) t(object) ???
			if (!lua_istable(L, 1) || !lua_istable(L, 2))
				return luaL_error(L, "invalid lstg object for 'ColliCheck'.");
			bool ignoreWorldMask = false;
			if (lua_gettop(L) == 3) {
				//有第三个参数则检查第三个参数
				// t(object) t(object) ignoreWorldMask ???
				ignoreWorldMask = lua_toboolean(L, 3) == 0 ? false : true;
			}
			lua_rawgeti(L, 1, 2);
			lua_rawgeti(L, 2, 2);
			bool pass;
			if (!LPOOL.ColliCheck((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), ignoreWorldMask, pass))
				return luaL_error(L, "invalid lstg object for 'ColliCheck'.");
			lua_pushboolean(L, pass);
			return 1;
		}
		static int Dist(lua_State* L) noexcept
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) id id
				double tRet;
				if (!LPOOL.Dist((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_pushnumber(L, tRet);
			}
			else if (lua_gettop(L) == 3) {
				// a b c ，此时是未知的
				if (lua_istable(L, 1)) {
					//如果第一个参数是object
					lua_rawgeti(L, 1, 2);  // t(object) x y ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Dist'.");
					}
					lua_Number dx = luaL_checknumber(L, 2) - p->x;
					lua_Number dy = luaL_checknumber(L, 3) - p->y;
					lua_pushnumber(L,
						sqrt(dx * dx + dy * dy)
					);
				}
				else if (lua_istable(L, 3)) {
					//如果第三个参数是object
					lua_rawgeti(L, 3, 2);  // x y t(object) ??? id
					GameObject* p = LPOOL.GetPooledObject((size_t)luaL_checkint(L, -1));
					if (!p) {
						return luaL_error(L, "invalid lstg object for 'Dist'.");
					}
					lua_Number dx = p->x - luaL_checknumber(L, 1);
					lua_Number dy = p->y - luaL_checknumber(L, 2);
					lua_pushnumber(L,
						sqrt(dx * dx + dy * dy)
					);
				}
				else {
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				}
			}
			else
			{
				lua_Number dx = luaL_checknumber(L, 3) - luaL_checknumber(L, 1);
				lua_Number dy = luaL_checknumber(L, 4) - luaL_checknumber(L, 2);
				lua_pushnumber(L, sqrt(dx*dx + dy*dy));
			}
			return 1;
		}
		static int GetV(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			double v, a;
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.GetV((size_t)luaL_checkinteger(L, -1), v, a))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			lua_pushnumber(L, v);
			lua_pushnumber(L, a);
			return 2;
		}
		static int SetV(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetV'.");
			if (lua_gettop(L) == 3)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), false))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else if (lua_gettop(L) == 4)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' 'rot' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), lua_toboolean(L, 4) == 0 ? false : true))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else
				return luaL_error(L, "invalid argument count for 'SetV'.");
			return 0;
		}
		static int SetImgState(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);

			BlendMode m = TranslateBlendMode(L, 2);
			fcyColor c(
				(fInt)luaL_checkinteger(L, 3),
				(fInt)luaL_checkinteger(L, 4),
				(fInt)luaL_checkinteger(L, 5),
				(fInt)luaL_checkinteger(L, 6)
			);
			if (!LPOOL.SetImgState(id, m, c))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			return 0;
		}
		static int SetParState(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetParState'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);

			BlendMode m = TranslateBlendMode(L, 2);
			fcyColor c(
				(fInt)luaL_checkinteger(L, 3),
				(fInt)luaL_checkinteger(L, 4),
				(fInt)luaL_checkinteger(L, 5),
				(fInt)luaL_checkinteger(L, 6)
			);
			if (!LPOOL.SetParState(id, m, c))
				return luaL_error(L, "invalid lstg object for 'SetParState'.");
			return 0;
		}
		static int BoxCheck(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			lua_rawgeti(L, 1, 2);  // t(object) 'l' 'r' 't' 'b' ??? id
			bool tRet;
			if (!LPOOL.BoxCheck(
				(size_t)luaL_checkinteger(L, -1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				tRet))
			{
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			}	
			lua_pushboolean(L, tRet);
			return 1;
		}
		static int ResetPool(lua_State* L) noexcept
		{
			LPOOL.ResetPool();
			return 0;
		}
		static int DefaultRenderFunc(lua_State* L) noexcept
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.DoDefaultRender(luaL_checkinteger(L, -1)))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			return 0;
		}
		static int NextObject(lua_State* L) noexcept
		{
			return LPOOL.NextObject(L);
		}
		static int ObjList(lua_State* L) noexcept
		{
			/*
			int g = luaL_checkinteger(L, 1);					// i(groupId)
			lua_pushcfunction(L, Wrapper::NextObject);	// i(groupId) next(f)
			lua_pushinteger(L, g);								// i(groupId) next(f) i(groupId)
			lua_pushinteger(L, LPOOL.FirstObject(g));			// i(groupId) next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
			//*/
			int g = luaL_checkinteger(L, 1);					// i(groupId)
			lua_pushcfunction(L, Wrapper::NextObject);			// i(groupId) next(f)
			lua_insert(L, 1);									// next(f) i(groupId)
			lua_pushinteger(L, LPOOL.FirstObject(g));			// next(f) i(groupId) id(firstobj) 最后的两个参数作为迭代器参数传入
			return 3;
		}
		static int ObjTable(lua_State* L) noexcept
		{
			return LPOOL.GetObjectTable(L);
		}
		static int ObjMetaIndex(lua_State* L) noexcept
		{
			return LPOOL.GetAttr(L);
		}
		static int ObjMetaNewIndex(lua_State* L) noexcept
		{
			return LPOOL.SetAttr(L);
		}
		static int ParticleStop(lua_State* L) noexcept
		{
			return LPOOL.ParticleStop(L);
		}
		static int ParticleFire(lua_State* L) noexcept
		{
			return LPOOL.ParticleFire(L);
		}
		static int ParticleGetn(lua_State* L) noexcept
		{
			return LPOOL.ParticleGetn(L);
		}
		static int ParticleGetEmission(lua_State* L) noexcept
		{
			return LPOOL.ParticleGetEmission(L);
		}
		static int ParticleSetEmission(lua_State* L) noexcept
		{
			return LPOOL.ParticleSetEmission(L);
		}
		// EX+
		static int GetCurrentObject(lua_State* L) noexcept
		{
			return LPOOL.PushCurrentObject(L);
		}
		// EX+ 对象更新相关，影响 frame 回调函数以及对象更新
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
		static int GetSuperPause(lua_State* L) noexcept
		{
			lua_pushinteger(L, LPOOL.GetNextFrameSuperPauseTime());
			return 1;
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
		static int ActiveWorlds(lua_State* L) noexcept
		{
			int a1 = luaL_optinteger(L, 1, 0);
			int a2 = luaL_optinteger(L, 2, 0);
			int a3 = luaL_optinteger(L, 3, 0);
			int a4 = luaL_optinteger(L, 4, 0);
			LPOOL.ActiveWorlds(a1, a2, a3, a4);
			return 0;
		}
		static int CheckWorlds(lua_State* L) noexcept
		{
			int a1 = luaL_checkinteger(L, 1);
			int a2 = luaL_checkinteger(L, 2);
			lua_pushboolean(L, LPOOL.CheckWorlds(a1, a2));
			return 1;
		}
	};

	luaL_Reg const lib[] = {
		{ "GetnObj", &Wrapper::GetnObj },
		{ "ObjFrame", &Wrapper::ObjFrame },
		{ "ObjRender", &Wrapper::ObjRender },
		{ "BoundCheck", &Wrapper::BoundCheck },
		{ "SetBound", &Wrapper::SetBound },
		{ "CollisionCheck", &Wrapper::CollisionCheck },
		{ "UpdateXY", &Wrapper::UpdateXY },
		{ "AfterFrame", &Wrapper::AfterFrame },
		{ "ResetPool", &Wrapper::ResetPool },
		{ "NextObject", &Wrapper::NextObject },
		{ "ObjList", &Wrapper::ObjList },
		{ "ObjTable", &Wrapper::ObjTable },
		// 对象控制函数
		{ "BoxCheck", &Wrapper::BoxCheck },
		{ "ResetObject", &Wrapper::ResetObject },
		{ "New", &Wrapper::New },
		{ "Del", &Wrapper::Del },
		{ "Kill", &Wrapper::Kill },
		{ "IsValid", &Wrapper::IsValid },
		{ "Angle", &Wrapper::Angle },
		{ "Dist", &Wrapper::Dist },
		{ "ColliCheck", &Wrapper::ColliCheck },
		{ "GetV", &Wrapper::GetV },
		{ "SetV", &Wrapper::SetV },
		{ "DefaultRenderFunc", &Wrapper::DefaultRenderFunc },
		{ "GetAttr", &Wrapper::ObjMetaIndex },
		{ "SetAttr", &Wrapper::ObjMetaNewIndex },
		// 对象资源控制
		{ "SetImgState", &Wrapper::SetImgState },
		{ "SetParState", &Wrapper::SetParState },
		{ "ParticleStop", &Wrapper::ParticleStop },
		{ "ParticleFire", &Wrapper::ParticleFire },
		{ "ParticleGetn", &Wrapper::ParticleGetn },
		{ "ParticleGetEmission", &Wrapper::ParticleGetEmission },
		{ "ParticleSetEmission", &Wrapper::ParticleSetEmission },
		// EX+
		{ "SetSuperPause", &Wrapper::SetSuperPause },
		{ "GetSuperPause", &Wrapper::GetSuperPause },
		{ "AddSuperPause", &Wrapper::AddSuperPause },
		{ "GetCurrentSuperPause", &Wrapper::GetCurrentSuperPause },
		{ "GetWorldFlag", &Wrapper::GetWorldFlag },
		{ "SetWorldFlag", &Wrapper::SetWorldFlag },
		{ "IsSameWorld", &Wrapper::CheckWorlds },
		{ "IsInWorld", &Wrapper::IsSameWorld },
		{ "GetCurrentObject", &Wrapper::GetCurrentObject },
		{ "ActiveWorlds", &Wrapper::ActiveWorlds },
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
