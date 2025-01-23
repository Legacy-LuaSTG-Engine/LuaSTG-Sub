#pragma once

#include <string>
#include <string_view>
#include <optional>
#include "lua.hpp"

namespace lua {
	struct stack_index_t {
		int32_t value{};

		stack_index_t() = default;
		stack_index_t(int32_t const index) : value(index) {}

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

		explicit stack_t(lua_State*& state) : L(state) {}

		// lua stack

		[[nodiscard]] stack_index_t index_of_top() const { return lua_gettop(L); }

		void pop_value(int32_t const count = 1) const { lua_pop(L, count); }

		// C -> lua

		template<size_t N>
		void push_value(char const (&str)[N]) {
			for (size_t len = N - 1; len > 0; len -= 1) {
				if (str[len] != '\0') {
					lua_pushlstring(L, str, len + 1);
				}
			}
			lua_pushlstring(L, str, 0);
		}

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
				}
				else {
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
		inline void set_map_value(stack_index_t index, std::string_view key, int32_t value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, uint32_t value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, float value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, double value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, stack_index_t value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		template<>
		inline void set_map_value(stack_index_t index, std::string_view key, lua_CFunction value) {
			push_value(key);
			push_value(value);
			lua_settable(L, index.value);
		}

		// lua -> C

		template<typename T>
		T get_value(stack_index_t const index) {
			if constexpr (std::is_same_v<T, bool>) {
				return lua_toboolean(L, index.value);
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				return static_cast<int32_t>(luaL_checkinteger(L, index.value));
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				return static_cast<uint32_t>(luaL_checknumber(L, index.value));
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
		T get_value(stack_index_t const index, T const& default_value) {
			if constexpr (std::is_same_v<T, bool>) {
				if (has_value(index))
					return lua_toboolean(L, index.value);
				return default_value;
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				return static_cast<int32_t>(luaL_optinteger(L, index.value, default_value));
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				return static_cast<uint32_t>(luaL_optnumber(L, index.value, static_cast<lua_Number>(default_value)));
			}
			else if constexpr (std::is_same_v<T, float>) {
				if (has_value(index))
					return static_cast<float>(luaL_checknumber(L, index.value));
				return default_value;
			}
			else if constexpr (std::is_same_v<T, double>) {
				if (has_value(index))
					return luaL_checknumber(L, index.value);
				return default_value;
			}
			else {
				static_assert(false, "FIXME");
				return {};
			}
		}

		// array & map

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
		T* create_userdata() { return static_cast<T*>(lua_newuserdata(L, sizeof(T))); }

		// type

		[[nodiscard]] bool has_value(stack_index_t const index) const { return lua_type(L, index.value) != LUA_TNONE; }
		[[nodiscard]] bool is_nil(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNIL; }
		[[nodiscard]] bool is_boolean(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TBOOLEAN; }
		[[nodiscard]] bool is_number(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TNUMBER; }
		[[nodiscard]] bool is_string(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TSTRING; }
		[[nodiscard]] bool is_table(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TTABLE; }
		[[nodiscard]] bool is_function(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TFUNCTION; }
		[[nodiscard]] bool is_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TUSERDATA; }
		[[nodiscard]] bool is_light_userdata(stack_index_t const index) const { return lua_type(L, index.value) == LUA_TLIGHTUSERDATA; }

		// package system

		[[nodiscard]] stack_index_t push_module(std::string_view const name) const {
			std::string const name_copy(name);
			constexpr luaL_Reg list[] = { {nullptr, nullptr} };
			luaL_register(L, name_copy.c_str(), list);
			return index_of_top();
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
}
