#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int BeginTabBar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiTabBarFlags>(2, 0);
		auto const result = ImGui::BeginTabBar(str_id.data(), flags);
		ctx.push_value(result);
		return 1;
	}
	int EndTabBar(lua_State*) {
		ImGui::EndTabBar();
		return 0;
	}
	int BeginTabItem(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto p_open = ctx.get_value<bool>(2, false);
		auto const flags = ctx.get_value<ImGuiTabItemFlags>(3, 0);
		auto const result = ImGui::BeginTabItem(label.data(), ctx.is_non_or_nil(2) ? nullptr : &p_open, flags);
		ctx.push_value(result);
		ctx.push_value(p_open);
		return 2;
	}
	int EndTabItem(lua_State*) {
		ImGui::EndTabItem();
		return 0;
	}
	int TabItemButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiTabItemFlags>(2, 0);
		auto const result = ImGui::TabItemButton(label.data(), flags);
		ctx.push_value(result);
		return 1;
	}
	int SetTabItemClosed(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const tab_or_docked_window_label = ctx.get_value<std::string_view>(1);
		ImGui::SetTabItemClosed(tab_or_docked_window_label.data());
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiTabBarsTabs(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginTabBar"sv, &BeginTabBar);
		ctx.set_map_value(m, "EndTabBar"sv, &EndTabBar);
		ctx.set_map_value(m, "BeginTabItem"sv, &BeginTabItem);
		ctx.set_map_value(m, "EndTabItem"sv, &EndTabItem);
		ctx.set_map_value(m, "TabItemButton"sv, &TabItemButton);
		ctx.set_map_value(m, "SetTabItemClosed"sv, &SetTabItemClosed);
	}
}
