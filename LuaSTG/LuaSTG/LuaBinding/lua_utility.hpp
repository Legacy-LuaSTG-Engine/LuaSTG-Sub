#pragma once
#include <string_view>
#include "lua.hpp"

inline std::string_view luaL_check_string_view(lua_State* L, int idx)
{
	size_t len = 0;
	char const* str = luaL_checklstring(L, idx, &len);
	return std::string_view(str, len);
}
inline void lua_push_string_view(lua_State* L, std::string_view const& str)
{
	lua_pushlstring(L, str.data(), str.length());
}

inline uint8_t lua_to_uint8_boolean(lua_State* L, int idx)
{
	return lua_toboolean(L, idx) != 0;
}

inline uint32_t luaL_checki_uint32(lua_State* L, int idx)
{
	return (uint32_t)luaL_checkinteger(L, idx);
}
inline uint32_t luaL_checkf_uint32(lua_State* L, int idx)
{
	return (uint32_t)luaL_checknumber(L, idx);
}
inline void lua_pushi_uint32(lua_State* L, uint32_t v)
{
	lua_pushinteger(L, (lua_Integer)v);
}
inline void lua_pushf_uint32(lua_State* L, uint32_t v)
{
	lua_pushnumber(L, (lua_Number)v);
}

inline float luaL_check_float(lua_State* L, int idx)
{
	return (float)luaL_checknumber(L, idx);
}
