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

namespace lua
{
	struct stack_index_t
	{
		int32_t value{};

		stack_index_t() = default;
		stack_index_t(int32_t index) : value(index) {};

		inline bool operator>(int r) {
			return value > r;
		}
		inline bool operator>=(int r) {
			return value >= r;
		}
		inline bool operator<(int r) {
			return value < r;
		}
		inline bool operator<=(int r) {
			return value <= r;
		}
		inline bool operator==(int r) {
			return value == r;
		}
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
		lua_State* L{};

		explicit stack_t(lua_State*& state) : L(state) {}
		
		// lua stack

		[[nodiscard]] stack_index_t index_of_top() const { return lua_gettop(L); }

		void pop_value(int32_t const count = 1) const { lua_pop(L, 1); }

		// C -> lua

		template<typename T>
		void push_value(T const& value) {
			if constexpr (std::is_same_v<T, std::nullopt_t>) {
				lua_pushnil(L);
			}
			else if constexpr (std::is_same_v<T, bool>) {
				lua_pushboolean(L, value);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				static_assert(std::is_same_v<double, lua_Number>);
				if (value > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
					lua_pushnumber(L, value);
				} else {
					lua_pushinteger(L, static_cast<int32_t>(value));
				}
			}
			else if constexpr (std::is_same_v<T, float>) {
				lua_pushnumber(L, value);
			}
			else if constexpr (std::is_same_v<T, double>) {
				static_assert(std::is_same_v<double, lua_Number>);
				lua_pushnumber(L, value);
			}
			else if constexpr (std::is_same_v<T, std::string_view>) {
				lua_pushlstring(L, value.data(), value.size());
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				lua_pushlstring(L, value.c_str(), value.length());
			}
			else if constexpr (std::is_same_v<T, stack_index_t>) {
				lua_pushvalue(L, value.value);
			}
			else if constexpr (std::is_same_v<T, lua_CFunction>) {
				lua_pushcfunction(L, value);
			}
			else {
				static_assert(false, "FIXME");
			}
		}

		// C -> lua, struct

		template<typename T>
		void push_vector2(T x, T y)
		{
			auto const idx = create_map(2);
			set_map_value(idx, "x", x);
			set_map_value(idx, "y", y);
		}

		template<typename T>
		void push_vector2(T vec2)
		{
			auto const idx = create_map(2);
			set_map_value(idx, "x", vec2.x);
			set_map_value(idx, "y", vec2.y);
		}

		// C -> lua array

		inline stack_index_t create_array(size_t size = 0) { lua_createtable(L, static_cast<int>(size), 0); return index_of_top(); }

		template<typename T>
		inline void set_array_value_zero_base(size_t c_index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value_zero_base(size_t c_index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, static_cast<int>(c_index + 1)); }

		template<typename T>
		inline void set_array_value(stack_index_t index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value(stack_index_t index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, index.value); }

		inline void set_array_value(stack_index_t array_index, int32_t index, stack_index_t value_index) { lua_pushvalue(L, value_index.value); lua_rawseti(L, array_index.value, index); }

		inline size_t get_array_size(stack_index_t index) { return lua_objlen(L, index.value); }

		inline void push_array_value_zero_base(stack_index_t array_index, size_t c_index) { lua_rawgeti(L, array_index.value, static_cast<int>(c_index + 1)); }

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
		inline int32_t get_value(stack_index_t index) { return (int32_t)luaL_checkinteger(L, index.value); }

		template<>
		inline uint32_t get_value(stack_index_t index) { return (uint32_t)luaL_checknumber(L, index.value); }

		template<>
		inline float get_value(stack_index_t index) { return (float)luaL_checknumber(L, index.value); }
		template<>
		inline double get_value(stack_index_t index) { return luaL_checknumber(L, index.value); }

		template<>
		inline std::string_view get_value(stack_index_t index) { size_t len = 0; char const* str = luaL_checklstring(L, index.value, &len); return { str, len }; }

		template<typename T>
		inline T get_array_value(stack_index_t array_index, stack_index_t lua_index) { typename T::__invalid_type__ _{}; }

		template<>
		inline stack_index_t get_array_value(stack_index_t array_index, stack_index_t lua_index) {
			lua_pushinteger(L, lua_index.value);
			lua_gettable(L, array_index.value);
			return { lua_gettop(L) };
		}

		template<>
		inline int32_t get_array_value(stack_index_t array_index, stack_index_t lua_index) {
			lua_pushinteger(L, lua_index.value);
			lua_gettable(L, array_index.value);
			auto const result = static_cast<int32_t>(luaL_checkinteger(L, -1));
			lua_pop(L, 1);
			return result;
		}

		template<>
		inline uint32_t get_array_value(stack_index_t array_index, stack_index_t lua_index) {
			lua_pushinteger(L, lua_index.value);
			lua_gettable(L, array_index.value);
			auto const result = static_cast<uint32_t>(luaL_checknumber(L, -1));
			lua_pop(L, 1);
			return result;
		}

		template<typename T>
		inline T get_map_value(stack_index_t index, std::string_view key) { return typename T::__invalid_type__{}; }

		template<>
		inline uint32_t get_map_value(stack_index_t index, std::string_view key)
		{
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<uint32_t>(-1);
			pop_value();
			return s;
		}

		template<>
		inline double get_map_value(stack_index_t index, std::string_view key)
		{
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<double>(-1);
			pop_value();
			return s;
		}

		template<>
		inline std::string_view get_map_value(stack_index_t index, std::string_view key)
		{
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<std::string_view>(-1);
			pop_value();
			return s;
		}

		inline bool has_map_value(stack_index_t index, std::string_view key)
		{
			push_value(key);
			lua_gettable(L, index.value);
			auto const has_value = is_value(-1) && !is_nil(-1);
			pop_value();
			return has_value;
		}

		// lua -> C (with default value)

		template<typename T>
		T get_value(stack_index_t const index, T const& default_value) {
			if constexpr (std::is_same_v<T, bool>) {
				if (is_value(index))
					return lua_toboolean(L, index.value);
				return default_value;
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				return static_cast<uint32_t>(luaL_optnumber(L, index.value, static_cast<lua_Number>(default_value)));
			}
			else if constexpr (std::is_same_v<T, float>) {
				if (is_value(index))
					return static_cast<float>(luaL_checknumber(L, index.value));
				return default_value;
			}
			else if constexpr (std::is_same_v<T, double>) {
				if (is_value(index))
					return luaL_checknumber(L, index.value);
				return default_value;
			}
			else {
				static_assert(false, "FIXME");
				return {};
			}
		}

		// userdata

		template<typename T>
		inline T* create_userdata()
		{
			return static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		}

		// type

		bool is_value(stack_index_t const index) const { return lua_type(L, index.value) != LUA_TNONE; }
		bool is_nil(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNIL; }
		bool is_boolean(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TBOOLEAN; }
		bool is_number(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNUMBER; }
		bool is_string(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TSTRING; }
		bool is_table(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TTABLE; }
		bool is_function(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TFUNCTION; }
		bool is_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TUSERDATA; }
		bool is_light_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TLIGHTUSERDATA; }

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

		inline bool is_metatable(stack_index_t index, std::string_view name) {
			if (!lua_getmetatable(L, index.value)) {
				return false;
			}
			auto const mt_index = index_of_top();
			auto const ref_index = push_metatable(name);
			int const result = lua_rawequal(L, mt_index.value, ref_index.value);
			pop_value(2);
			return !!result;
		}

	};
}
