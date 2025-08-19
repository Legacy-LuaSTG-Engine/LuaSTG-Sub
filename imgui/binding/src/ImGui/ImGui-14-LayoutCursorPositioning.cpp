#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int GetCursorScreenPos(lua_State* const vm) {
		auto const result = ImGui::GetCursorScreenPos();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int SetCursorScreenPos(lua_State* const vm) {
		auto const pos = imgui::binding::ImVec2Binding::as(vm, 1);
		ImGui::SetCursorScreenPos(*pos);
		return 0;
	}
	int GetContentRegionAvail(lua_State* const vm) {
		auto const result = ImGui::GetContentRegionAvail();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetCursorPos(lua_State* const vm) {
		auto const result = ImGui::GetCursorPos();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetCursorPosX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetCursorPosX();
		ctx.push_value(result);
		return 1;
	}
	int GetCursorPosY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetCursorPosY();
		ctx.push_value(result);
		return 1;
	}
	int SetCursorPos(lua_State* const vm) {
		auto const local_pos = imgui::binding::ImVec2Binding::as(vm, 1);
		ImGui::SetCursorPos(*local_pos);
		return 0;
	}
	int SetCursorPosX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const local_x = ctx.get_value<float>(1);
		ImGui::SetCursorPosX(local_x);
		return 0;
	}
	int SetCursorPosY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const local_y = ctx.get_value<float>(1);
		ImGui::SetCursorPosY(local_y);
		return 0;
	}
	int GetCursorStartPos(lua_State* const vm) {
		auto const result = ImGui::GetCursorStartPos();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiLayoutCursorPositioning(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "GetCursorScreenPos"sv, &GetCursorScreenPos);
		ctx.set_map_value(m, "SetCursorScreenPos"sv, &SetCursorScreenPos);
		ctx.set_map_value(m, "GetContentRegionAvail"sv, &GetContentRegionAvail);
		ctx.set_map_value(m, "GetCursorPos"sv, &GetCursorPos);
		ctx.set_map_value(m, "GetCursorPosX"sv, &GetCursorPosX);
		ctx.set_map_value(m, "GetCursorPosY"sv, &GetCursorPosY);
		ctx.set_map_value(m, "SetCursorPos"sv, &SetCursorPos);
		ctx.set_map_value(m, "SetCursorPosX"sv, &SetCursorPosX);
		ctx.set_map_value(m, "SetCursorPosY"sv, &SetCursorPosY);
		ctx.set_map_value(m, "GetCursorStartPos"sv, &GetCursorStartPos);
	}
}
