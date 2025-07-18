#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int PushClipRect(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const clip_rect_min = imgui::binding::ImVec2Binding::as(vm, 1);
		auto const clip_rect_max = imgui::binding::ImVec2Binding::as(vm, 2);
		auto const intersect_with_current_clip_rect = ctx.get_value<bool>(3);
		ImGui::PushClipRect(*clip_rect_min, *clip_rect_max, intersect_with_current_clip_rect);
		return 0;
	}
	int PopClipRect(lua_State*) {
		ImGui::PopClipRect();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiClipping(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PushClipRect"sv, &PushClipRect);
		ctx.set_map_value(m, "PopClipRect"sv, &PopClipRect);
	}
}
