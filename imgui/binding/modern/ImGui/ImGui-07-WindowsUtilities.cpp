#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
	int IsWindowAppearing(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::IsWindowAppearing();
		ctx.push_value(result);
		return 1;
	}
	int IsWindowCollapsed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::IsWindowCollapsed();
		ctx.push_value(result);
		return 1;
	}
	int IsWindowFocused(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const flags = ctx.get_value<ImGuiFocusedFlags>(1, 0);
		auto const result = ImGui::IsWindowFocused(flags);
		ctx.push_value(result);
		return 1;
	}
	int IsWindowHovered(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const flags = ctx.get_value<ImGuiHoveredFlags>(1, 0);
		auto const result = ImGui::IsWindowHovered(flags);
		ctx.push_value(result);
		return 1;
	}
	int GetWindowPos(lua_State* const vm) {
		auto const result = ImGui::GetWindowPos();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetWindowSize(lua_State* const vm) {
		auto const result = ImGui::GetWindowSize();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetWindowWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetWindowWidth();
		ctx.push_value(result);
		return 1;
	}
	int GetWindowHeight(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetWindowHeight();
		ctx.push_value(result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiWindowsUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "IsWindowAppearing"sv, &IsWindowAppearing);
		ctx.set_map_value(m, "IsWindowCollapsed"sv, &IsWindowCollapsed);
		ctx.set_map_value(m, "IsWindowFocused"sv, &IsWindowFocused);
		ctx.set_map_value(m, "IsWindowHovered"sv, &IsWindowHovered);
		ctx.set_map_value(m, "GetWindowDrawList"sv, &notSupported);
		ctx.set_map_value(m, "GetWindowPos"sv, &GetWindowPos);
		ctx.set_map_value(m, "GetWindowSize"sv, &GetWindowSize);
		ctx.set_map_value(m, "GetWindowWidth"sv, &GetWindowWidth);
		ctx.set_map_value(m, "GetWindowHeight"sv, &GetWindowHeight);
	}
}
