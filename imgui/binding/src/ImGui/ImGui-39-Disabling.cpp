#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int BeginDisabled(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const disabled = ctx.get_value<bool>(1, true);
		ImGui::BeginDisabled(disabled);
		return 0;
	}
	int EndDisabled(lua_State*) {
		ImGui::EndDisabled();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiDisabling(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginDisabled"sv, &BeginDisabled);
		ctx.set_map_value(m, "EndDisabled"sv, &EndDisabled);
	}
}
