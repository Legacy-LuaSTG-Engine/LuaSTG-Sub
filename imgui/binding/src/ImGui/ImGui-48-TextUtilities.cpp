#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int CalcTextSize(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const text = ctx.get_value<std::string_view>(1);
		auto const hide_text_after_double_hash = ctx.get_value<bool>(2, false);
		auto const wrap_width = ctx.get_value<float>(3, -1.0f);
		auto const result = ImGui::CalcTextSize(text.data(), text.data() + text.size(), hide_text_after_double_hash, wrap_width);
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiTextUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "CalcTextSize"sv, &CalcTextSize);
	}
}
