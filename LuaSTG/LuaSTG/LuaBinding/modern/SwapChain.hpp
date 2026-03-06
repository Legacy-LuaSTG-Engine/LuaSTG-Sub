#pragma once
#include <string_view>
#include "lua.hpp"
#include "core/SwapChain.hpp"

namespace luastg::binding {

	struct SwapChain {

		static std::string_view class_name;

		[[maybe_unused]] core::ISwapChain* data{};

		static bool is(lua_State* L, int index);

		static SwapChain* as(lua_State* L, int index);

		static SwapChain* create(lua_State* L);

		static void registerClass(lua_State* L);

	};

}
