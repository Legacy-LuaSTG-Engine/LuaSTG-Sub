#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int SetNextItemAllowOverlap(lua_State*) {
		ImGui::SetNextItemAllowOverlap();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiOverlappingMode(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "SetNextItemAllowOverlap"sv, &SetNextItemAllowOverlap);
	}
}
