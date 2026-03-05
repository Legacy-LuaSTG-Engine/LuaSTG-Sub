#pragma once
#include "core/VideoDecoder.hpp"
#include "lua.hpp"
#include <string_view>

namespace luastg::binding {
	struct VideoDecoder {
		static const std::string_view class_name;

		[[maybe_unused]] core::IVideoDecoder* data{};

		static bool is(lua_State* vm, int index);
		static VideoDecoder* as(lua_State* vm, int index);
		static VideoDecoder* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
