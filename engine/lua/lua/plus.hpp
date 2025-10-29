#pragma once

#include <string>
#include <string_view>
#include <optional>
#include "lua.hpp"

namespace lua {
	struct stack_index_t {
		int32_t value{};

		stack_index_t() = default;
		constexpr stack_index_t(int32_t const index) : value(index) {}

		bool operator>(int32_t const right) const {
			return value > right;
		}
		bool operator>=(int32_t const right) const {
			return value >= right;
		}
		bool operator<(int32_t const right) const {
			return value < right;
		}
		bool operator<=(int32_t const right) const {
			return value <= right;
		}
		bool operator==(int32_t const right) const {
			return value == right;
		}
	};

	class stack_balancer_t {
	public:
		stack_balancer_t() = delete;
		stack_balancer_t(stack_balancer_t const&) = delete;
		stack_balancer_t(stack_balancer_t&&) = delete;
		explicit stack_balancer_t(lua_State* const state) : L(state), N(lua_gettop(state)) {}
		~stack_balancer_t() { lua_settop(L, N); }
		stack_balancer_t& operator=(stack_balancer_t const&) = delete;
		stack_balancer_t& operator=(stack_balancer_t&&) = delete;
	private:
		lua_State* const L{};
		int const N{};
	};

	struct stack_t {
		lua_State* L{};

		explicit stack_t(lua_State* const& state) : L(state) {}

		// lua stack

		[[nodiscard]] stack_index_t index_of_top() const { return lua_gettop(L); }

		void pop_value(int32_t const count = 1) const { lua_pop(L, count); }

		// C -> lua

		template<size_t N>
		void push_value(char const (&str)[N]) const {
			for (auto i = static_cast<intptr_t>(N - 1); i >= 0; i -= 1) {
				if (str[i] != '\0') {
					lua_pushlstring(L, str, i + 1);
					return;
				}
			}
			lua_pushlstring(L, str, 0);
		}

		template<typename T>
		void push_value(T const& value) const {
			if constexpr (std::is_same_v<T, std::nullopt_t>) {
				lua_pushnil(L);
			}
			else if constexpr (std::is_enum_v<T>) {
				push_value(static_cast<std::underlying_type_t<T>>(value));
			}
			else if constexpr (std::is_same_v<T, bool>) {
				lua_pushboolean(L, value);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				lua_pushinteger(L, value);
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				static_assert(std::is_same_v<double, lua_Number>);
				if (value > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
					lua_pushnumber(L, value);
				}
				else {
					lua_pushinteger(L, static_cast<int32_t>(value));
				}
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				// WARN: not full supported
				lua_pushnumber(L, static_cast<double>(value));
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				// WARN: not full supported
				lua_pushnumber(L, static_cast<double>(value));
			}
			else if constexpr (std::is_same_v<T, float>) {
				lua_pushnumber(L, value);
			}
			else if constexpr (std::is_same_v<T, double>) {
				static_assert(std::is_same_v<double, lua_Number>);
				lua_pushnumber(L, value);
			}
			else if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, char const*>) {
				static_assert(false, "FIXME");
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
		void push_vector2(T x, T y) {
			auto const idx = create_map(2);
			set_map_value(idx, "x", x);
			set_map_value(idx, "y", y);
		}

		template<typename T>
		void push_vector2(T vec2) {
			auto const idx = create_map(2);
			set_map_value(idx, "x", vec2.x);
			set_map_value(idx, "y", vec2.y);
		}

		// C -> lua array

		stack_index_t create_array(size_t const size = 0) const { lua_createtable(L, static_cast<int>(size), 0); return index_of_top(); }

		template<typename T>
		inline void set_array_value_zero_base(size_t c_index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value_zero_base(size_t c_index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, static_cast<int>(c_index + 1)); }

		template<typename T>
		void set_array_value_zero_base(stack_index_t const array_index, stack_index_t const index, T const& value) const {
			lua_pushinteger(L, index.value + 1);
			push_value(value);
			lua_settable(L, array_index.value);
		}

		template<typename T>
		inline void set_array_value(stack_index_t index, T value) { typename T::__invalid_type__ _{}; }

		template<>
		inline void set_array_value(stack_index_t index, std::string_view value) { lua_pushlstring(L, value.data(), value.size()); lua_rawseti(L, -2, index.value); }

		//void set_array_value(stack_index_t const array_index, int32_t const index, std::nullopt_t) const { lua_pushnil(L); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, stack_index_t const value_index) const { lua_pushvalue(L, value_index.value); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, bool const value) const { push_value(value); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, int32_t const value) const { push_value(value); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, std::string_view const& value) const { push_value(value); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, void* ptr) const { lua_pushlightuserdata(L, ptr); lua_rawseti(L, array_index.value, index); }
		//void set_array_value(stack_index_t const array_index, int32_t const index, float const value) const { lua_pushnumber(L, value); lua_rawseti(L, array_index.value, index); }

		template<typename T>
		void set_array_value(stack_index_t const array_index, stack_index_t const index, T const& value) const {
			lua_pushinteger(L, index.value);
			push_value(value);
			lua_settable(L, array_index.value);
		}
		void set_array_value(stack_index_t const array_index, stack_index_t const index, void* const ptr) const {
			lua_pushinteger(L, index.value);
			lua_pushlightuserdata(L, ptr);
			lua_settable(L, array_index.value);
		}

		inline size_t get_array_size(stack_index_t const index) const { return lua_objlen(L, index.value); }

		inline void push_array_value_zero_base(stack_index_t array_index, size_t c_index) { lua_rawgeti(L, array_index.value, static_cast<int>(c_index + 1)); }

		// C -> lua map

		inline stack_index_t create_map(size_t reserve = 0u) const { lua_createtable(L, 0, static_cast<int>(reserve)); return index_of_top(); }

		template<typename T>
		inline void set_map_value(stack_index_t const index, std::string_view const key, T const value) const {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		void push_map_value(stack_index_t const map_index, std::string_view const& key) const {
			push_value(key);
			lua_gettable(L, map_index.value);
		}

		template<typename T>
		T get_map_value(stack_index_t const map_index, std::string_view const& key) const {
			constexpr stack_index_t top_index(-1);
			push_value(key);
			lua_gettable(L, map_index.value);
			auto const result = get_value<T>(top_index);
			pop_value();
			return result;
		}

		template<typename T>
		T get_map_value(stack_index_t const map_index, std::string_view const& key, T const& default_value) const {
			constexpr stack_index_t top_index(-1);
			push_value(key);
			lua_gettable(L, map_index.value);
			auto const result = get_value<T>(top_index, default_value);
			pop_value();
			return result;
		}

		// lua -> C

		template<typename T>
		[[nodiscard]] T get_value(stack_index_t const index) const {
			if constexpr (std::is_enum_v<T>) {
				return static_cast<T>(get_value<std::underlying_type_t<T>>(index));
			}
			else if constexpr (std::is_same_v<T, bool>) {
				return lua_toboolean(L, index.value);
			}
			else if constexpr (std::is_same_v<T, int8_t>) {
				return static_cast<int8_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, uint8_t>) {
				return static_cast<uint8_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, int16_t>) {
				return static_cast<int16_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				return static_cast<uint16_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				return static_cast<int32_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				return static_cast<uint32_t>(luaL_checknumber(L, index.value));
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				// WARN: not full supported
				return static_cast<int64_t>(luaL_checknumber(L, index.value));
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				// WARN: not full supported
				return static_cast<uint64_t>(luaL_checknumber(L, index.value));
			}
			else if constexpr (std::is_same_v<T, float>) {
				return static_cast<float>(luaL_checknumber(L, index.value));
			}
			else if constexpr (std::is_same_v<T, double>) {
				return luaL_checknumber(L, index.value);
			}
			else if constexpr (std::is_same_v<T, std::string_view>) {
				size_t len = 0;
				char const* str = luaL_checklstring(L, index.value, &len);
				return { str, len };
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				size_t len = 0;
				char const* str = luaL_checklstring(L, index.value, &len);
				return { str, len };
			}
			else {
				static_assert(false, "FIXME");
				return {};
			}
		}

		// lua -> C (with default value)

		template<typename T>
		[[nodiscard]] T get_value(stack_index_t const index, T const& default_value) const {
			if (!is_non_or_nil(index))
				return get_value<T>(index);
			return default_value;
		}

		// array & map

		template<typename T>
		inline T get_array_value(stack_index_t const array_index, stack_index_t const index) const {
			lua_pushinteger(L, index.value);
			lua_gettable(L, array_index.value);
			auto const result = get_value<T>(-1);
			pop_value();
			return result;
		}
		template<>
		inline stack_index_t get_array_value(stack_index_t const array_index, stack_index_t const index) const {
			lua_pushinteger(L, index.value);
			lua_gettable(L, array_index.value);
			return { lua_gettop(L) };
		}

		template<typename T>
		inline T get_map_value(stack_index_t index, std::string_view key) { return typename T::__invalid_type__{}; }

		template<>
		inline uint32_t get_map_value(stack_index_t index, std::string_view key) {
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<uint32_t>(-1);
			pop_value();
			return s;
		}

		template<>
		inline double get_map_value(stack_index_t index, std::string_view key) {
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<double>(-1);
			pop_value();
			return s;
		}

		template<>
		inline std::string_view get_map_value(stack_index_t index, std::string_view key) {
			push_value(key);
			lua_gettable(L, index.value);
			auto const s = get_value<std::string_view>(-1);
			pop_value();
			return s;
		}

		inline bool has_map_value(stack_index_t index, std::string_view key) {
			push_value(key);
			lua_gettable(L, index.value);
			auto const r = has_value(-1) && !is_nil(-1);
			pop_value();
			return r;
		}

		// userdata

		template<typename T>
		T* create_userdata() const { return static_cast<T*>(lua_newuserdata(L, sizeof(T))); }

		template<typename T>
		T* as_userdata(stack_index_t const index) const { return static_cast<T*>(luaL_checkudata(L, index.value, T::class_name.data())); }

		template<typename T>
		T* as_userdata(stack_index_t const index, std::string_view const class_name) const { return static_cast<T*>(luaL_checkudata(L, index.value, class_name.data())); }

		// type

		[[nodiscard]] bool has_value(stack_index_t const index) const { return lua_type(L, index.value) != LUA_TNONE; }
		[[nodiscard]] bool is_non_or_nil(stack_index_t const index) const { auto const type = lua_type(L, index.value); return type == LUA_TNONE || type == LUA_TNIL; }
		[[nodiscard]] bool is_nil(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNIL; }
		[[nodiscard]] bool is_boolean(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TBOOLEAN; }
		[[nodiscard]] bool is_number(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNUMBER; }
		[[nodiscard]] bool is_string(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TSTRING; }
		[[nodiscard]] bool is_table(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TTABLE; }
		[[nodiscard]] bool is_function(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TFUNCTION; }
		[[nodiscard]] bool is_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TUSERDATA; }
		[[nodiscard]] bool is_light_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TLIGHTUSERDATA; }

		// package system

		[[nodiscard]] stack_index_t create_module(std::string_view const name) const {
			std::string const name_copy(name);
			constexpr luaL_Reg list[] = { {nullptr, nullptr} };
			luaL_register(L, name_copy.c_str(), list);
			return index_of_top();
		}

		[[nodiscard]] stack_index_t push_module(std::string_view const name) const {
			std::string const name_copy(name);
			auto const n = lua_gettop(L);                  // 1..n |  n + 1  |  n + 2  |
			lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED"); // .... | _LOADED |         | from lua 5.1/luajit source code
			lua_getfield(L, n + 1, name_copy.c_str());     // .... | _LOADED | module  |
			lua_remove(L, n + 1);                          // .... | module  |         |
			if (!lua_istable(L, n + 1)) {
				return { luaL_error(L, "module '%s' not found", name_copy.c_str()) };
			}
			return { n + 1 };
		}

		[[nodiscard]] stack_index_t create_metatable(std::string_view const name) const {
			std::string const name_copy(name);
			luaL_newmetatable(L, name_copy.c_str());
			return index_of_top();
		}

		[[nodiscard]] stack_index_t push_metatable(std::string_view const name) const {
			std::string const name_copy(name);
			luaL_getmetatable(L, name_copy.c_str());
			return index_of_top();
		}

		void set_metatable(stack_index_t const index, std::string_view const name) const {
			std::string const name_copy(name);
			luaL_getmetatable(L, name_copy.c_str());
			lua_setmetatable(L, index.value);
		}

		[[nodiscard]] bool is_metatable(stack_index_t const index, std::string_view const name) const {
			if (!lua_getmetatable(L, index.value)) {
				return false;
			}
			auto const mt_index = index_of_top();
			auto const named_mt_index = push_metatable(name);
			auto const result = lua_rawequal(L, mt_index.value, named_mt_index.value);
			pop_value(2);
			return !!result;
		}

	};

	namespace methods {
		// a + b
		constexpr std::string_view add{"__add"};
		// a - b
		constexpr std::string_view sub{"__sub"};
		// a * b
		constexpr std::string_view mul{"__mul"};
		// a / b
		constexpr std::string_view div{"__div"};
		// a % b
		constexpr std::string_view mod{"__mod"};
		// -n
		constexpr std::string_view unm{"__unm"};
		// #n
		constexpr std::string_view len{"__len"};
		// a .. b
		constexpr std::string_view concat{"__concat"};
		// a == b
		constexpr std::string_view eq{"__eq"};
		// a < b
		constexpr std::string_view lt{"__lt"};
		// a <= b
		constexpr std::string_view le{"__le"};
		// tostring(n)
		constexpr std::string_view to_string{"__tostring"};
		// n[k]
		constexpr std::string_view index{"__index"};
		// n[k] = v
		constexpr std::string_view new_index{"__newindex"};
		// n()
		constexpr std::string_view call{"__call"};
		// dispose
		constexpr std::string_view gc{"__gc"};
	}

	namespace fields {
		// n[k]
		constexpr std::string_view index{"__index"};
		// weak table
		constexpr std::string_view mode{"__mode"};
		// meta table
		constexpr std::string_view meta_table{"__metatable"};
	}
}
