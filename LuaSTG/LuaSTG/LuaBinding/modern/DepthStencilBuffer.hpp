#pragma once
#include "lua.hpp"
#include "core/DepthStencilBuffer.hpp"

namespace luastg::binding {
	struct DepthStencilBuffer {
		static std::string_view class_name;

		[[maybe_unused]] core::IDepthStencilBuffer* data{};

		static bool is(lua_State* vm, int index);
		static DepthStencilBuffer* as(lua_State* vm, int index);
		static DepthStencilBuffer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
