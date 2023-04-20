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
	struct stack_index_t
	{
		int32_t value{};

		stack_index_t() = default;
		stack_index_t(int32_t index) : value(index) {};
	};

	struct stack_balancer_t
	{
		lua_State*& L;
		int N;

		inline stack_balancer_t(lua_State*& state) : L(state), N(lua_gettop(state)) {}
		inline ~stack_balancer_t() { lua_settop(L, N); }
	};

	struct stack_t
	{
		lua_State*& L;

		inline stack_t(lua_State*& state) : L(state) {}
		
		// lua stack

		inline stack_index_t index_of_top() { return lua_gettop(L); }

		// C -> lua

		template<typename T>
		inline void push_value(T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void push_value(bool value) { lua_pushboolean(L, value); }

		template<>
		inline void push_value(std::string_view value) { lua_pushlstring(L, value.data(), value.size()); }

		template<>
		inline void push_value(int32_t value) { lua_pushinteger(L, value); }

		template<>
		inline void push_value(uint32_t value)
		{
			constexpr uint32_t const int32_max = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
			bool const value_condition = value <= int32_max;
			constexpr bool const size_condition = sizeof(lua_Integer) > sizeof(uint32_t);
			if (size_condition || value_condition)
			{
				lua_pushinteger(L, static_cast<lua_Integer>(value));
			}
			else if constexpr (sizeof(lua_Number) >= sizeof(double))
			{
				lua_pushnumber(L, (lua_Number)value);
			}
			else
			{
				assert(false);
				lua_pushnumber(L, (lua_Number)value);
			}
		}

		template<>
		inline void push_value(float value) { lua_pushnumber(L, value); }

		template<>
		inline void push_value(double value)
		{
			if constexpr (sizeof(lua_Number) >= sizeof(double))
			{
				lua_pushnumber(L, value);
			}
			else
			{
				assert(false);
				lua_pushnumber(L, (lua_Number)value);
			}
		}

		template<>
		inline void push_value(lua_CFunction value) { lua_pushcfunction(L, value); }
		
		template<>
		inline void push_value(stack_index_t index) { lua_pushvalue(L, index.value); }

		// C -> lua, struct

		template<typename T>
		inline void push_vector2(T x, T y)
		{
			auto const idx = create_map(2);
			set_map_value(idx, "x", x);
			set_map_value(idx, "y", y);
		}

		template<typename T>
		inline void push_vector2(T vec2)
		{
			auto const idx = create_map(2);
			set_map_value(idx, "x", vec2.x);
			set_map_value(idx, "y", vec2.y);
		}

		// C -> lua array

		inline stack_index_t create_array(size_t size) { lua_createtable(L, static_cast<int>(size), 0); return index_of_top(); }

		template<typename T>
		inline void set_array_value_zero_base(size_t c_index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value_zero_base(size_t c_index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, static_cast<int>(c_index + 1)); }

		template<typename T>
		inline void set_array_value(stack_index_t index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value(stack_index_t index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, index.value); }

		// C -> lua map

		inline stack_index_t create_map(size_t reserve = 0u) { lua_createtable(L, 0, static_cast<int>(reserve)); return index_of_top(); }

		template<typename T>
		inline void set_map_value(stack_index_t index, std::string_view key, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, int32_t value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, uint32_t value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, float value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, double value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, stack_index_t value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, lua_CFunction value)
		{
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		// lua -> C

		template<typename T>
		inline T get_value(stack_index_t index) { typename T::__invalid_type__ _{}; }

		template<>
		inline bool get_value(stack_index_t index) { return lua_toboolean(L, index.value); }

		template<>
		inline float get_value(stack_index_t index) { return (float)luaL_checknumber(L, index.value); }
		template<>
		inline double get_value(stack_index_t index) { return luaL_checknumber(L, index.value); }

		template<>
		inline std::string_view get_value(stack_index_t index) { size_t len = 0; char const* str = luaL_checklstring(L, index.value, &len); return { str, len }; }

		// userdata

		template<typename T>
		inline T* create_userdata()
		{
			return static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		}

		// package system

		inline stack_index_t push_module(std::string_view name)
		{
			std::string const name_copy(name);
			luaL_Reg const list[] = { {NULL, NULL} };
			luaL_register(L, name_copy.c_str(), list);
			return index_of_top();
		}

		inline stack_index_t create_metatable(std::string_view name)
		{
			std::string const name_copy(name);
			luaL_newmetatable(L, name_copy.c_str());
			return index_of_top();
		}

		inline stack_index_t push_metatable(std::string_view name)
		{
			std::string const name_copy(name);
			luaL_getmetatable(L, name_copy.c_str());
			return index_of_top();
		}

		inline void set_metatable(stack_index_t index, std::string_view name)
		{
			std::string const name_copy(name);
			luaL_getmetatable(L, name_copy.c_str());
			lua_setmetatable(L, index.value);
		}
	};
}
