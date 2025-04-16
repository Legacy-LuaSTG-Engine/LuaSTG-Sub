#pragma once
#include "lua.hpp"
#include "Core/Graphics/Mesh.hpp"

namespace luastg::binding {
	struct MeshRenderer {
		static std::string_view class_name;

		[[maybe_unused]] core::Graphics::IMeshRenderer* data{};
		[[maybe_unused]] core::Vector3F position;
		[[maybe_unused]] core::Vector3F scale;
		[[maybe_unused]] core::Vector3F rotation_yaw_pitch_roll;

		static bool is(lua_State* vm, int index);
		static MeshRenderer* as(lua_State* vm, int index);
		static MeshRenderer* create(lua_State* vm);
		static void registerClass(lua_State* vm);
	};
}
