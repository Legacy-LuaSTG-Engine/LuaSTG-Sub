#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int GetScrollX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetScrollX();
		ctx.push_value(result);
		return 1;
	}
	int GetScrollY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetScrollY();
		ctx.push_value(result);
		return 1;
	}
	int SetScrollX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const scroll_x = ctx.get_value<float>(1);
		ImGui::SetScrollX(scroll_x);
		return 0;
	}
	int SetScrollY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const scroll_y = ctx.get_value<float>(1);
		ImGui::SetScrollY(scroll_y);
		return 0;
	}
	int GetScrollMaxX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetScrollMaxX();
		ctx.push_value(result);
		return 1;
	}
	int GetScrollMaxY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetScrollMaxY();
		ctx.push_value(result);
		return 1;
	}
	int SetScrollHereX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const center_x_ratio = ctx.get_value<float>(1, 0.5f);
		ImGui::SetScrollHereX(center_x_ratio);
		return 0;
	}
	int SetScrollHereY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const center_y_ratio = ctx.get_value<float>(1, 0.5f);
		ImGui::SetScrollHereY(center_y_ratio);
		return 0;
	}
	int SetScrollFromPosX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const local_x = ctx.get_value<float>(1);
		auto const center_x_ratio = ctx.get_value<float>(2, 0.5f);
		ImGui::SetScrollFromPosX(local_x, center_x_ratio);
		return 0;
	}
	int SetScrollFromPosY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const local_y = ctx.get_value<float>(1);
		auto const center_y_ratio = ctx.get_value<float>(2, 0.5f);
		ImGui::SetScrollFromPosY(local_y, center_y_ratio);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWindowsScrolling(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "GetScrollX"sv, &GetScrollX);
		ctx.set_map_value(m, "GetScrollY"sv, &GetScrollY);
		ctx.set_map_value(m, "SetScrollX"sv, &SetScrollX);
		ctx.set_map_value(m, "SetScrollY"sv, &SetScrollY);
		ctx.set_map_value(m, "GetScrollMaxX"sv, &GetScrollMaxX);
		ctx.set_map_value(m, "GetScrollMaxY"sv, &GetScrollMaxY);
		ctx.set_map_value(m, "SetScrollHereX"sv, &SetScrollHereX);
		ctx.set_map_value(m, "SetScrollHereY"sv, &SetScrollHereY);
		ctx.set_map_value(m, "SetScrollFromPosX"sv, &SetScrollFromPosX);
		ctx.set_map_value(m, "SetScrollFromPosY"sv, &SetScrollFromPosY);
	}
}
