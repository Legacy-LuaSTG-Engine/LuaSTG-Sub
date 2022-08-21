#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"

inline void lua_push_vec2(lua_State* L, Core::Vector2F const& v)
{
	lua_createtable(L, 0, 2);
	lua_pushlstring(L, "x", 1);
	lua_pushnumber(L, v.x);
	lua_rawset(L, -3);
	lua_pushlstring(L, "y", 1);
	lua_pushnumber(L, v.y);
	lua_rawset(L, -3);
}
inline Core::Vector2F luaL_check_vec2(lua_State* L, int idx)
{
	Core::Vector2F ret;
	if (!lua_istable(L, idx))
	{
		luaL_typerror(L, idx, "table  (vector2f)");
		return ret;
	}
	lua_pushlstring(L, "x", 1);
	lua_gettable(L, idx);
	ret.x = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_pushlstring(L, "y", 1);
	lua_gettable(L, idx);
	ret.y = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);
	return ret;
}
inline Core::Color4B Color4f_to_Color4B(float c[4])
{
	return Core::Color4B(
		(uint8_t)std::clamp(c[0] * 255.0f, 0.0f, 255.0f),
		(uint8_t)std::clamp(c[1] * 255.0f, 0.0f, 255.0f),
		(uint8_t)std::clamp(c[2] * 255.0f, 0.0f, 255.0f),
		(uint8_t)std::clamp(c[3] * 255.0f, 0.0f, 255.0f)
	);
}
inline void Color4B_to_Color4f(Core::Color4B c, float d[4])
{
	d[0] = (float)c.r / 255.0f;
	d[1] = (float)c.g / 255.0f;
	d[2] = (float)c.b / 255.0f;
	d[3] = (float)c.a / 255.0f;
}
inline void lua_push_color4f(lua_State* L, float c[4])
{
	lua_createtable(L, 0, 4);

	lua_pushlstring(L, "r", 1);
	lua_pushnumber(L, c[0]);
	lua_rawset(L, -3);

	lua_pushlstring(L, "g", 1);
	lua_pushnumber(L, c[1]);
	lua_rawset(L, -3);

	lua_pushlstring(L, "b", 1);
	lua_pushnumber(L, c[2]);
	lua_rawset(L, -3);

	lua_pushlstring(L, "a", 1);
	lua_pushnumber(L, c[3]);
	lua_rawset(L, -3);
}
inline void luaL_check_color4f(lua_State* L, int idx, float c[4])
{
	if (!lua_istable(L, idx))
	{
		luaL_typerror(L, idx, "table (color4f)");
		return;
	}

	lua_pushlstring(L, "r", 1);
	lua_gettable(L, idx);
	c[0] = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_pushlstring(L, "g", 1);
	lua_gettable(L, idx);
	c[1] = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_pushlstring(L, "b", 1);
	lua_gettable(L, idx);
	c[2] = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_pushlstring(L, "a", 1);
	lua_gettable(L, idx);
	c[3] = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);
}

namespace LuaSTGPlus::LuaWrapper
{
	std::string_view const ParticleSystemWrapper::ClassID("lstg.ParticleSystemInstance");

	void ParticleSystemWrapper::UserData::ReleaseAll()
	{
		if (res)
		{
			if (ptr)
			{
				res->FreeInstance(ptr);
			}
			res->Release();
		}
		res = nullptr;
		ptr = nullptr;
	}

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
			static int SetActive(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->SetActive(lua_toboolean(L, 2));
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
					int const emi = (int)luaL_checkinteger(L, 2);
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
					lua_pushinteger(L, self->ptr->GetEmission());
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
					int const argc = lua_gettop(L) - 1;
					if (argc >= 4)
					{
						float const delta = (float)luaL_checknumber(L, 2);
						float const x = (float)luaL_checknumber(L, 3);
						float const y = (float)luaL_checknumber(L, 4);
						float const rot = (float)(luaL_checknumber(L, 5) * L_DEG_TO_RAD);
						self->ptr->SetRotation((float)rot);
						if (self->ptr->IsActived())  // 兼容性处理
						{
							self->ptr->SetActive(false);
							self->ptr->SetCenter(Core::Vector2F(x, y));
							self->ptr->SetActive(true);
						}
						else
						{
							self->ptr->SetCenter(Core::Vector2F(x, y));
						}
						self->ptr->Update(delta);
					}
					else if (argc == 3)
					{
						float const delta = (float)luaL_checknumber(L, 2);
						float const x = (float)luaL_checknumber(L, 3);
						float const y = (float)luaL_checknumber(L, 4);
						if (self->ptr->IsActived())  // 兼容性处理
						{
							self->ptr->SetActive(false);
							self->ptr->SetCenter(Core::Vector2F(x, y));
							self->ptr->SetActive(true);
						}
						else
						{
							self->ptr->SetCenter(Core::Vector2F(x, y));
						}
						self->ptr->Update(delta);
					}
					else
					{
						float const delta = (float)luaL_optnumber(L, 2, 1.0 / 60.0);
						self->ptr->Update(delta);
					}
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
			static int SetOldBehavior(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					bool const b = lua_toboolean(L, 2);
					self->ptr->SetOldBehavior(b);
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
				return 0;
			}

			// LuaSTG-x API
			
			static int getAliveCount(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushinteger(L, (lua_Integer)self->ptr->GetAliveCount());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getAlphaVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fAlphaVar);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getCenter(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_push_vec2(L, self->ptr->GetCenter());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getColorEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					if (lua_toboolean(L, 2))
						lua_push_color4f(L, self->ptr->GetParticleSystemInfo().colColorEnd);
					else
						ColorWrapper::CreateAndPush(L, Color4f_to_Color4B(self->ptr->GetParticleSystemInfo().colColorEnd));
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getColorStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					if (lua_toboolean(L, 2))
						lua_push_color4f(L, self->ptr->GetParticleSystemInfo().colColorStart);
					else
						ColorWrapper::CreateAndPush(L, Color4f_to_Color4B(self->ptr->GetParticleSystemInfo().colColorStart));
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getColorVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fColorVar);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getDirection(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fDirection);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getEmissionFreq(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushinteger(L, self->ptr->GetParticleSystemInfo().nEmission);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getGravityMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fGravityMax);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getGravityMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fGravityMin);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getLifeMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fParticleLifeMax);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getLifeMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fParticleLifeMin);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getLifetime(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fLifetime);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getRadialAccelMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fRadialAccelMax);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getRadialAccelMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fRadialAccelMin);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getRelative(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushboolean(L, self->ptr->GetParticleSystemInfo().bRelative);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getRenderMode(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					TranslateBlendModeToString(L, self->ptr->GetBlendMode());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getResource(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->res)
				{
					lua_push_string_view(L, self->res->GetResName());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getRotation(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetRotation());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSeed(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, (lua_Number)self->ptr->GetSeed()); // TODO
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSizeEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSizeEnd);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSizeStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSizeStart);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSizeVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSizeVar);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpeedMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpeedMax);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpeedMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpeedMin);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpinEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpinEnd);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpinStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpinStart);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpinVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpinVar);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getSpread(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fSpread);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getTangentialAccelMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fTangentialAccelMax);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int getTangentialAccelMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushnumber(L, self->ptr->GetParticleSystemInfo().fTangentialAccelMin);
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}

			static int isActive(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					lua_pushboolean(L, self->ptr->IsActived());
					return 1;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setActive(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->SetActive(lua_toboolean(L, 2));
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}

			static int setAlphaVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fAlphaVar = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setCenter(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					Core::Vector2F const vec2 = luaL_check_vec2(L, 2);
					self->ptr->SetCenter(vec2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setColorEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					if (lua_isuserdata(L, 2))
					{
						Core::Color4B* col = ColorWrapper::Cast(L, 2);
						Color4B_to_Color4f(*col, self->ptr->GetParticleSystemInfo().colColorEnd);
					}
					else
					{
						luaL_check_color4f(L, 2, self->ptr->GetParticleSystemInfo().colColorEnd);
					}
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setColorStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					if (lua_isuserdata(L, 2))
					{
						Core::Color4B* col = ColorWrapper::Cast(L, 2);
						Color4B_to_Color4f(*col, self->ptr->GetParticleSystemInfo().colColorStart);
					}
					else
					{
						luaL_check_color4f(L, 2, self->ptr->GetParticleSystemInfo().colColorStart);
					}
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setColorVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fColorVar = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setDirection(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fDirection = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setEmissionFreq(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().nEmission = (int)luaL_checkinteger(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setGravityMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fGravityMax = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setGravityMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fGravityMin = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setLifeMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fParticleLifeMax = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setLifeMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fParticleLifeMin = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setLifetime(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fLifetime = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setRadialAccelMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fRadialAccelMax = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setRadialAccelMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fRadialAccelMin = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setRelative(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().bRelative = lua_toboolean(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setRenderMode(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					BlendMode const blend = TranslateBlendMode(L, 2);
					self->ptr->SetBlendMode(blend);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setRotation(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					float const rot = (float)luaL_checknumber(L, 2);
					self->ptr->SetRotation(rot);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSeed(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					uint32_t const seed = (uint32_t)luaL_checknumber(L, 2);
					self->ptr->SetSeed(seed);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSizeEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSizeEnd = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSizeStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSizeStart = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSizeVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSizeVar = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpeedMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpeedMax = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpeedMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpeedMin = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpinEnd(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpinEnd = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpinStart(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpinStart = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpinVar(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpinVar = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setSpread(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fSpread = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setTangentialAccelMax(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fTangentialAccelMax = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}
			static int setTangentialAccelMin(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->ptr)
				{
					self->ptr->GetParticleSystemInfo().fTangentialAccelMin = (float)luaL_checknumber(L, 2);
					return 0;
				}
				else
				{
					return luaL_error(L, "invalid particle system instance.");
				}
			}

			static int __tostring(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				if (self->res)
				{
					lua_pushfstring(L, "lstg.ParticleSystem(\"%s\")", self->res->GetResName().c_str());
				}
				else
				{
					lua_pushstring(L, "lstg.ParticleSystem");
				}
				return 1;
			}
			static int __gc(lua_State* L)
			{
				UserData* self = (UserData*)luaL_checkudata(L, 1, ClassID.data());
				self->ReleaseAll();
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
			{ "SetActive", &Wrapper::SetActive },
			{ "GetAliveCount", &Wrapper::GetAliveCount },
			{ "SetEmission", &Wrapper::SetEmission },
			{ "GetEmission", &Wrapper::GetEmission },
			{ "Update", &Wrapper::Update },
			{ "Render", &Wrapper::Render },
			{ "SetOldBehavior", &Wrapper::SetOldBehavior },

		#define F(X) { #X, &Wrapper::X }
			F(getAliveCount),
			F(getAlphaVar),
			F(getCenter),
			F(getColorEnd),
			F(getColorStart),
			F(getColorVar),
			F(getDirection),
			F(getEmissionFreq),
			F(getGravityMax),
			F(getGravityMin),
			F(getLifeMax),
			F(getLifeMin),
			F(getLifetime),
			F(getRadialAccelMax),
			F(getRadialAccelMin),
			F(getRelative),
			F(getRenderMode),
			F(getResource),
			F(getRotation),
			F(getSeed),
			F(getSizeEnd),
			F(getSizeStart),
			F(getSizeVar),
			F(getSpeedMax),
			F(getSpeedMin),
			F(getSpinEnd),
			F(getSpinStart),
			F(getSpinVar),
			F(getSpread),
			F(getTangentialAccelMax),
			F(getTangentialAccelMin),

			F(isActive),
			F(setActive),

			F(setAlphaVar),
			F(setCenter),
			F(setColorEnd),
			F(setColorStart),
			F(setColorVar),
			F(setDirection),
			F(setEmissionFreq),
			F(setGravityMax),
			F(setGravityMin),
			F(setLifeMax),
			F(setLifeMin),
			F(setLifetime),
			F(setRadialAccelMax),
			F(setRadialAccelMin),
			F(setRelative),
			F(setRenderMode),
			F(setRotation),
			F(setSeed),
			F(setSizeEnd),
			F(setSizeStart),
			F(setSizeVar),
			F(setSpeedMax),
			F(setSpeedMin),
			F(setSpinEnd),
			F(setSpinStart),
			F(setSpinVar),
			F(setSpread),
			F(setTangentialAccelMax),
			F(setTangentialAccelMin),
		#undef F

			{ NULL, NULL }
		};

		luaL_Reg const mt[] = {
			{ "__tostring", &Wrapper::__tostring },
			{ "__gc", &Wrapper::__gc },
			{ NULL, NULL },
		};

		luaL_Reg const ins[] = {
			{ "ParticleSystemData", Wrapper::ParticleSystemInstance },
			{ NULL, NULL }
		};

		luaL_register(L, "lstg", ins); // ??? lstg
		RegisterClassIntoTable(L, ".ParticleSystemData", lib, ClassID.data(), mt);
		lua_pop(L, 1);
	}
}
