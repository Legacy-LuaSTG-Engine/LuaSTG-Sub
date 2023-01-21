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

namespace lua
{
	struct stack_t
	{
		lua_State*& L;

		inline stack_t(lua_State*& state) : L(state) {}

		// C -> lua

		template<typename T>
		inline void push_value(T value) { assert(false); }

		template<>
		inline void push_value(bool value) { lua_pushboolean(L, value); }

		template<>
		inline void push_value(std::string_view value) { lua_pushlstring(L, value.data(), value.size()); }

		// C -> lua array

		inline void create_array(size_t size) { lua_createtable(L, static_cast<int>(size), 0); }

		template<typename T>
		inline void set_array_value_zero_base(size_t c_index, T value) { assert(false); }

		template<>
		inline void set_array_value_zero_base(size_t c_index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, static_cast<int>(c_index + 1)); }

		template<typename T>
		inline void set_array_value(int32_t index, T value) { assert(false); }

		template<>
		inline void set_array_value(int32_t index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, index); }

		// lua -> C

		template<typename T>
		inline T get_value(int32_t index) { assert(false); }

		template<>
		inline float get_value(int32_t index) { return (float)luaL_checknumber(L, index); }
		template<>
		inline double get_value(int32_t index) { return luaL_checknumber(L, index); }

		template<>
		inline std::string_view get_value(int32_t index) { size_t len = 0; char const* str = luaL_checklstring(L, index, &len); return { str, len }; }
	};
}
