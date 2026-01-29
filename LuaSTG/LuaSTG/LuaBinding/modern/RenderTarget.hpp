#pragma once
#include "lua.hpp"
#include "core/Graphics/Device.hpp"

namespace luastg::binding {
	struct RenderTarget {
		static std::string_view class_name;

		[[maybe_unused]] core::Graphics::IRenderTarget* data{};

		static bool is(lua_State* vm, int index);
		static RenderTarget* as(lua_State* vm, int index);
		static RenderTarget* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
