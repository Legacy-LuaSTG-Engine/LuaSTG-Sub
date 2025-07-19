#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int SetItemDefaultFocus(lua_State*) {
		ImGui::SetItemDefaultFocus();
		return 0;
	}
	int SetKeyboardFocusHere(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const offset = ctx.get_value<int>(1, 0);
		ImGui::SetKeyboardFocusHere(offset);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiFocusActivation(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "SetItemDefaultFocus"sv, &SetItemDefaultFocus);
		ctx.set_map_value(m, "SetKeyboardFocusHere"sv, &SetKeyboardFocusHere);
	}
}
