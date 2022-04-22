#include "LuaWrapper.hpp"

namespace LuaSTGPlus::LuaWrapper
{
	std::string_view const ParticleSystemWrapper::ClassID("lstg.ParticleSystemInstance");

	ParticleSystemWrapper::UserData* ParticleSystemWrapper::Cast(lua_State* L, int idx)
	{
		return (UserData*)luaL_checkudata(L, idx, ClassID.data());
	}
	ParticleSystemWrapper::UserData* ParticleSystemWrapper::Create(lua_State* L)
	{
		UserData* self = (UserData*)lua_newuserdata(L, sizeof(UserData)); // udata
		self->res = nullptr;
		self->ptr = nullptr;
		luaL_getmetatable(L, ClassID.data()); // udata mt
		lua_setmetatable(L, -2); // udata
		return self;
	}
	void ParticleSystemWrapper::Register(lua_State* L)
	{
		struct Wrapper
		{
			static int SetInactive(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->SetInactive();
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int SetActive(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->SetActive();
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int GetAliveCount(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					size_t const alive = self->ptr->GetAliveCount();
					lua_pushinteger(L, (lua_Integer)alive);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int SetEmission(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					float const emi = (float)luaL_checknumber(L, 2);
					self->ptr->SetEmission(emi);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int GetEmission(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					float const emi = self->ptr->GetEmission();
					lua_pushnumber(L, emi);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int Update(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					float const x = (float)luaL_checknumber(L, 2);
					float const y = (float)luaL_checknumber(L, 3);
					float const rot = (float)luaL_checknumber(L, 4);
					float const delta = (float)luaL_optnumber(L, 5, 1.0 / 60.0);
					self->ptr->SetRotation((float)rot);
					if (self->ptr->IsActived())  // 兼容性处理
					{
						self->ptr->SetInactive();
						self->ptr->SetCenter(fcyVec2(x, y));
						self->ptr->SetActive();
					}
					else
					{
						self->ptr->SetCenter(fcyVec2(x, y));
					}
					self->ptr->Update(delta);
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
				return 0;
			}
			static int Render(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					float const scale = (float)luaL_checknumber(L, 2);
					LAPP.Render(self->ptr, scale, scale);
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
				return 0;
			}

			static int __tostring(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->res)
				{
					lua_pushfstring(L, "lstg.ParticleSystemInstance(\"%s\")", self->res->GetResName().c_str());
				}
				else
				{
					lua_pushstring(L, "lstg.ParticleSystemInstance");
				}
				return 1;
			}
			static int __gc(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->res)
				{
					if (self->ptr)
					{
						self->res->FreeInstance(self->ptr);
					}
					self->res->Release();
				}
				self->res = nullptr;
				self->ptr = nullptr;
				return 0;
			}
			
			static int ParticleSystemInstance(lua_State* L)
			{
				char const* ps_name = luaL_checkstring(L, 1);

				auto p_res = LRES.FindParticle(ps_name);
				if (!p_res)
					return luaL_error(L, "particle system '%s' not found.", ps_name);

				try
				{
					auto* p_obj = p_res->AllocInstance();
					auto* p_lud = Create(L);
					p_lud->res = *p_res;
					p_lud->res->AddRef();
					p_lud->ptr = p_obj;
				}
				catch (const std::bad_alloc&)
				{
					return luaL_error(L, "create particle system instance of '%s' failed, memory allocation failed.", ps_name);
				}
				
				return 1;
			}
		};

		luaL_Reg const lib[] = {
			{ "SetInactive", &Wrapper::SetInactive },
			{ "SetActive", &Wrapper::SetActive },
			{ "GetAliveCount", &Wrapper::GetAliveCount },
			{ "SetEmission", &Wrapper::SetEmission },
			{ "GetEmission", &Wrapper::GetEmission },
			{ "Update", &Wrapper::Update },
			{ "Render", &Wrapper::Render },
			{ NULL, NULL }
		};

		luaL_Reg const mt[] = {
			{ "__tostring", &Wrapper::__tostring },
			{ "__gc", &Wrapper::__gc },
			{ NULL, NULL },
		};

		luaL_Reg const ins[] = {
			{ "ParticleSystemInstance", Wrapper::ParticleSystemInstance },
			{ NULL, NULL }
		};

		luaL_register(L, "lstg", ins); // ??? lstg
		RegisterClassIntoTable(L, ".ParticleSystemInstance", lib, ClassID.data(), mt);
		lua_pop(L, 1);
	}
}
