#include "LuaWrapper/LuaWrapper.hpp"
#include "GameObjectBentLaser.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	namespace LuaWrapper
	{
		struct BentLaserWrapper::Wrapper {
			GameObjectBentLaser* handle;
		};

		void BentLaserWrapper::Register(lua_State* L)LNOEXCEPT
		{
			struct Function
			{
#define GETUDATA(p, i) Wrapper* (p) = static_cast<Wrapper*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_BENTLASER));
#define CHECKUDATA(p) if (!(p)->handle) return luaL_error(L, "%s was released.", LUASTG_LUA_TYPENAME_BENTLASER);
				static int Update(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					if (lua_isnumber(L, 2))
					{
						float const x = (float)luaL_checknumber(L, 2);
						float const y = (float)luaL_checknumber(L, 3);
						float const rot = (float)luaL_checknumber(L, 4);
						int const node_count = (float)luaL_checkinteger(L, 5);
						float const width = (float)luaL_checknumber(L, 6);
						if (!p->handle->Update(x, y, rot, node_count, width, true))
							return luaL_error(L, "'Update' failed.");
					}
					else
					{
						if (!lua_istable(L, 2))
							return luaL_error(L, "invalid lstg object for 'Update'.");
						lua_rawgeti(L, 2, 2);  // self t(object) ??? id
						size_t id = (size_t)luaL_checkinteger(L, -1);
						lua_pop(L, 1);
						if (!p->handle->Update(id, luaL_checkinteger(L, 3), (float)luaL_checknumber(L, 4), luaL_optnumber(L, 5, 0) == 0))
							return luaL_error(L, "invalid lstg object for 'Update'.");
					}
					return 0;
				}
				static int UpdateNode(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					if (LUA_TNUMBER == lua_type(L, 2))
					{
						return p->handle->api_UpdateSingleNode(L);
					}
					else
					{
						if (!lua_istable(L, 2))
							return luaL_error(L, "invalid lstg object for 'UpdateNode'.");
						lua_rawgeti(L, 2, 2);  // self t(object) ??? id
						size_t id = (size_t)luaL_checkinteger(L, -1);
						lua_pop(L, 1);
						if (!p->handle->UpdateByNode(id, luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), (float)luaL_checknumber(L, 5), luaL_optnumber(L, 6, 0) == 0))
							return luaL_error(L, "invalid lstg object for 'UpdateNode'.");
					}
					return 0;
				}
				static int UpdatePositionByList(lua_State* L)LNOEXCEPT // u(laser) t(list) length width index revert 
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					if (!lua_istable(L, 2))
						return luaL_error(L, "invalid lstg object for 'Update'.");
					int i3 = luaL_checkinteger(L, 3);
					float f4 = (float)luaL_checknumber(L, 4);
					int i5 = luaL_optinteger(L, 5, 1);
					bool i6 = (bool)luaL_optinteger(L, 6, 0) != 0;
					lua_settop(L, 2); // udata t
					if (!p->handle->UpdatePositionByList(L, i3, f4, i5, i6))
						return luaL_error(L, "Update laser data failed.");
					return 0;
				}
				static int UpdateAllNodeByList(lua_State* L) LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					return p->handle->api_UpdateAllNodeByList(L);
				}
				static int SampleByLength(lua_State* L)LNOEXCEPT // t(self) <length>
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					float length = (float)luaL_checknumber(L, 2);
					lua_pop(L, 2); // 
					p->handle->SampleL(L, length); // t(list)
					return 1;
				}
				static int SampleByTime(lua_State* L)LNOEXCEPT // t(self) <length>
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					float time = (float)luaL_checknumber(L, 2);
					lua_pop(L, 2); // 
					p->handle->SampleT(L, time / 60.0f); // t(list)
					return 1;
				}
				static int Release(lua_State* L)LNOEXCEPT
				{
					return 0;
				}
				static int Render(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					if (!p->handle->Render(
						luaL_checkstring(L, 2),
						TranslateBlendMode(L, 3),
						*static_cast<fcyColor*>(luaL_checkudata(L, 4, LUASTG_LUA_TYPENAME_COLOR)),
						(float)luaL_checknumber(L, 5),
						(float)luaL_checknumber(L, 6),
						(float)luaL_checknumber(L, 7),
						(float)luaL_checknumber(L, 8),
#ifdef GLOBAL_SCALE_COLLI_SHAPE
						(float)luaL_optnumber(L, 9, 1.) * LRES.GetGlobalImageScaleFactor()
#else
						(float)luaL_optnumber(L, 9, 1.)
#endif // GLOBAL_SCALE_COLLI_SHAPE
					))
					{
						return luaL_error(L, "can't render object with texture '%s'.", luaL_checkstring(L, 2));
					}
					return 0;
				}
				static int CollisionCheck(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					bool r = p->handle->CollisionCheck(
						(float)luaL_checknumber(L, 2),
						(float)luaL_checknumber(L, 3),
						(float)luaL_optnumber(L, 4, 0),
						(float)luaL_optnumber(L, 5, 0),
						(float)luaL_optnumber(L, 6, 0),
						lua_toboolean(L, 7) == 0 ? false : true
					);
					::lua_pushboolean(L, r);
					return 1;
				}
				static int RenderCollider(lua_State* L) {
					GETUDATA(p, 1);
					CHECKUDATA(p);
					p->handle->RenderCollider(*static_cast<fcyColor*>(luaL_checkudata(L, 2, LUASTG_LUA_TYPENAME_COLOR)));
					return 0;
				}
				static int CollisionCheckWidth(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					bool r = p->handle->CollisionCheckW(
						(float)luaL_checknumber(L, 2),
						(float)luaL_checknumber(L, 3),
						(float)luaL_optnumber(L, 5, 0),
						(float)luaL_optnumber(L, 6, 0),
						(float)luaL_optnumber(L, 7, 0),
						lua_toboolean(L, 8) == 0 ? false : true,
						(float)luaL_checknumber(L, 4)
					);
					lua_pushboolean(L, r);
					return 1;
				}
				static int BoundCheck(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					::lua_pushboolean(L, p->handle->BoundCheck());
					return 1;
				}
				static int SetAllWidth(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					p->handle->SetAllWidth((float)luaL_checknumber(L, 2));
					return 0;
				}
				static int SetEnvelope(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					p->handle->SetEnvelope(
						(float)luaL_checknumber(L, 2),
						(float)luaL_checknumber(L, 3),
						(float)luaL_checknumber(L, 4),
						(float)luaL_checknumber(L, 5));
					return 0;
				}
				static int GetEnvelope(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					float a, b, c, d;
					p->handle->GetEnvelope(a, b, c, d);
					lua_pushnumber(L, (lua_Number)a);
					lua_pushnumber(L, (lua_Number)b);
					lua_pushnumber(L, (lua_Number)c);
					lua_pushnumber(L, (lua_Number)d);
					return 4;
				}

				static int Meta_Len(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					CHECKUDATA(p);
					lua_pushinteger(L, (lua_Integer)p->handle->GetSize());
					return 1;
				}
				static int Meta_ToString(lua_State* L)LNOEXCEPT
				{
					lua_pushfstring(L, LUASTG_LUA_TYPENAME_BENTLASER);
					return 1;
				}
				static int Meta_GC(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					if (p->handle) {
						GameObjectBentLaser::FreeInstance(p->handle);
						p->handle = nullptr;
					}
					return 0;
				}
#undef CHECKUDATA
#undef GETUDATA
			};

			luaL_Reg tMethods[] =
			{
				{ "Update", &Function::Update },
				{ "UpdateNode", &Function::UpdateNode },
				{ "Release", &Function::Release },
				{ "Render", &Function::Render },
				{ "CollisionCheck", &Function::CollisionCheck },
				{ "RenderCollider", &Function::RenderCollider },
				{ "CollisionCheckWidth", &Function::CollisionCheckWidth },
				{ "BoundCheck", &Function::BoundCheck },
				{ "SampleByLength", &Function::SampleByLength },
				{ "SampleByTime", &Function::SampleByTime },
				{ "UpdatePositionByList", &Function::UpdatePositionByList },
				{ "UpdateAllNode", &Function::UpdateAllNodeByList },
				{ "SetAllWidth", &Function::SetAllWidth },
				{ "SetEnvelope", &Function::SetEnvelope },
				{ "GetEnvelope", &Function::GetEnvelope },
				{ NULL, NULL }
			};

			luaL_Reg tMetaTable[] =
			{
				{ "__len", &Function::Meta_Len },
				{ "__tostring", &Function::Meta_ToString },
				{ "__gc", &Function::Meta_GC },
				{ NULL, NULL }
			};
			
			RegisterClassIntoTable(L, ".BentLaser", tMethods, LUASTG_LUA_TYPENAME_BENTLASER, tMetaTable);
		}

		void BentLaserWrapper::CreateAndPush(lua_State* L)
		{
			Wrapper* p = static_cast<Wrapper*>(lua_newuserdata(L, sizeof(Wrapper))); // udata
			try {
				p->handle = GameObjectBentLaser::AllocInstance();//可能有alloc失败的风险
			}
			catch (const std::bad_alloc&) {
				p->handle = nullptr;
			}
			luaL_getmetatable(L, LUASTG_LUA_TYPENAME_BENTLASER); // udata mt
			lua_setmetatable(L, -2); // udata
		}
	}
}
