#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Selectable(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto selected = ctx.get_value<bool>(2, false);
        auto const flags = ctx.get_value<ImGuiSelectableFlags>(3, 0);
        auto result = false;
        if (ctx.index_of_top() >= 4) {
            auto const size = imgui::binding::ImVec2Binding::as(vm, 4);
            result = ImGui::Selectable(label.data(), &selected, flags, *size);
        }
        else {
            result = ImGui::Selectable(label.data(), &selected, flags);
        }
        ctx.push_value(result);
        ctx.push_value(selected);
        return 2;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsSelectables(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Selectable"sv, &Selectable);
	}
}
