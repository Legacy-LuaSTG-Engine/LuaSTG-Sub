#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int SetNavCursorVisible(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const visible = ctx.get_value<bool>(1);
		ImGui::SetNavCursorVisible(visible);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiKeyboardGamepadNavigation(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "SetNavCursorVisible"sv, &SetNavCursorVisible);
	}
}
