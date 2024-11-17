#pragma once
#include <cstdint>
#include <string_view>
#include <string>
#include <optional>
#include "lua.hpp"

namespace lua {
	struct StackIndex {
		int32_t value{};

		inline explicit StackIndex(int32_t const value) noexcept : value(value) {
		}
	};

	class StackBalancer {
	public:
		explicit StackBalancer(lua_State* L);
		~StackBalancer();
	private:
		lua_State* L;
		int32_t N;
	};

	class Stack {
	public:
		explicit Stack(lua_State* L) noexcept : L(L) {
		}

		[[nodiscard]] StackIndex indexOfTop() const {
			return StackIndex(lua_gettop(L));
		}

		void pushValue(std::nullopt_t const) const {
			lua_pushnil(L);
		}

		void pushValue(bool const value) const {
			lua_pushboolean(L, value ? 1 : 0);
		}

		void pushValue(std::string_view const& value) const {
			lua_pushlstring(L, value.data(), value.length());
		}

		void pushValue(StackIndex const value) const {
			lua_pushvalue(L, value.value);
		}

		template<typename T>
		[[nodiscard]] T getValue(StackIndex index) const;

		template<typename T>
		[[nodiscard]] T getValue(int32_t const index) const {
			return getValue<T>(StackIndex(index));
		}

		// module and class system

		[[nodiscard]] StackIndex pushModule(std::string_view const& name) const {
			constexpr luaL_Reg empty[] = { {} };
			luaL_register(L, name.data(), empty);
			auto const index = lua_gettop(L);
			lua_pushnil(L);
			lua_setglobal(L, name.data());
			return StackIndex(index);
		}

		[[nodiscard]] StackIndex createMetaTable(std::string_view const& name) const {
			luaL_newmetatable(L, name.data());
			return StackIndex(lua_gettop(L));
		}

		void setMetaTable(StackIndex const index, std::string_view const& name) const {
			luaL_getmetatable(L, name.data());
			lua_setmetatable(L, index.value);
		}

		template<typename T>
		[[nodiscard]] T* createUserData() const {
			return static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		}

		template<typename T>
		[[nodiscard]] T* createUserDataWithNew() const {
			auto p = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
			return new(p) T();
		}

		// map

		[[nodiscard]] StackIndex createMap(size_t const reserve = 0) const {
			lua_createtable(L, 0, static_cast<int>(reserve));
			return indexOfTop();
		}

		void setMapValue(StackIndex const index, std::string_view const& key, lua_CFunction const value) const {
			lua_pushcfunction(L, value);
			lua_setfield(L, index.value, key.data());
		}

		void setMapValue(StackIndex const index, std::string_view const& key, StackIndex const value_index) const {
			lua_pushvalue(L, value_index.value);
			lua_setfield(L, index.value, key.data());
		}

	private:
		lua_State* L{};
	};

	static_assert(sizeof(Stack) == sizeof(lua_State*));
	static_assert(alignof(Stack) == alignof(lua_State*));
}
