#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int BeginMenuBar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::BeginMenuBar();
		ctx.push_value(result);
		return 1;
	}
	int EndMenuBar(lua_State*) {
		ImGui::EndMenuBar();
		return 0;
	}
	int BeginMainMenuBar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::BeginMainMenuBar();
		ctx.push_value(result);
		return 1;
	}
	int EndMainMenuBar(lua_State*) {
		ImGui::EndMainMenuBar();
		return 0;
	}
	int BeginMenu(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const enabled = ctx.get_value<bool>(2, true);
		auto const result = ImGui::BeginMenu(label.data(), enabled);
		ctx.push_value(result);
		return 1;
	}
	int EndMenu(lua_State*) {
		ImGui::EndMenu();
		return 0;
	}
	int MenuItem(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const shortcut = ctx.get_value<std::string_view>(2, {});
		auto selected = ctx.get_value<bool>(3, false);
		auto const enabled = ctx.get_value<bool>(4, true);
		auto const result = ImGui::MenuItem(label.data(), shortcut.data(), &selected, enabled);
		ctx.push_value(result);
		ctx.push_value(selected);
		return 2;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsMenus(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginMenuBar"sv, &BeginMenuBar);
		ctx.set_map_value(m, "EndMenuBar"sv, &EndMenuBar);
		ctx.set_map_value(m, "BeginMainMenuBar"sv, &BeginMainMenuBar);
		ctx.set_map_value(m, "EndMainMenuBar"sv, &EndMainMenuBar);
		ctx.set_map_value(m, "BeginMenu"sv, &BeginMenu);
		ctx.set_map_value(m, "EndMenu"sv, &EndMenu);
		ctx.set_map_value(m, "MenuItem"sv, &MenuItem);
	}
}
