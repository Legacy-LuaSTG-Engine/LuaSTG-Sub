#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int StyleColorsDark(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto const style = imgui::binding::ImGuiStyleBinding::as(vm, 1);
			ImGui::StyleColorsDark(style);
		}
		else {
			ImGui::StyleColorsDark();
		}
		return 0;
	}
	int StyleColorsLight(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto const style = imgui::binding::ImGuiStyleBinding::as(vm, 1);
			ImGui::StyleColorsLight(style);
		}
		else {
			ImGui::StyleColorsLight();
		}
		return 0;
	}
	int StyleColorsClassic(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto const style = imgui::binding::ImGuiStyleBinding::as(vm, 1);
			ImGui::StyleColorsClassic(style);
		}
		else {
			ImGui::StyleColorsClassic();
		}
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiStyles(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "StyleColorsDark"sv, &StyleColorsDark);
		ctx.set_map_value(m, "StyleColorsLight"sv, &StyleColorsLight);
		ctx.set_map_value(m, "StyleColorsClassic"sv, &StyleColorsClassic);
	}
}
