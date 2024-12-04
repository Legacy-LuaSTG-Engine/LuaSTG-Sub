#include "lua/Stack.hpp"

namespace lua {
	StackBalancer::StackBalancer(lua_State* L) : L(L), N(lua_gettop(L)) {
	}
	StackBalancer::~StackBalancer() { lua_settop(L, N); }
}

namespace lua {
	template<>
	[[nodiscard]] int32_t Stack::getValue<int32_t>(StackIndex const index) const {
		return static_cast<int32_t>(luaL_checkinteger(L, index.value));
	}

	template<>
	[[nodiscard]] std::string_view Stack::getValue<std::string_view>(StackIndex const index) const {
		size_t len{};
		auto str = luaL_checklstring(L, index.value, &len);
		// managed by lua VM
		// ReSharper disable once CppDFALocalValueEscapesFunction
		return { str, len };
	}

	template<>
	[[nodiscard]] std::string Stack::getValue<std::string>(StackIndex const index) const {
		size_t len{};
		auto str = luaL_checklstring(L, index.value, &len);
		// managed by lua VM
		// ReSharper disable once CppDFALocalValueEscapesFunction
		return { str, len };
	}
}
