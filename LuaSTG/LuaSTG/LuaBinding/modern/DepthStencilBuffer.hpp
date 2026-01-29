#pragma once
#include "lua.hpp"
#include "core/Graphics/Device.hpp"

namespace luastg::binding {
	struct DepthStencilBuffer {
		static std::string_view class_name;

		[[maybe_unused]] core::Graphics::IDepthStencilBuffer* data{};

		static bool is(lua_State* vm, int index);
		static DepthStencilBuffer* as(lua_State* vm, int index);
		static DepthStencilBuffer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
