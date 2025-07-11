#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Begin(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 3) {
			auto const name = ctx.get_value<std::string_view>(1);
			auto open = ctx.get_value<bool>(2);
			auto const flags = ctx.get_value<ImGuiWindowFlags>(3);
			imgui::binding::beginSetNextWindowSizeConstraintsCallbackWrapper(vm);
			auto const result = ImGui::Begin(name.data(), &open, flags);
			imgui::binding::endSetNextWindowSizeConstraintsCallbackWrapper(vm);
			ctx.push_value(result);
			ctx.push_value(open);
			return 2;
		}
		if (ctx.index_of_top() >= 2) {
			auto const name = ctx.get_value<std::string_view>(1);
			auto open = ctx.get_value<bool>(2);
			imgui::binding::beginSetNextWindowSizeConstraintsCallbackWrapper(vm);
			auto const result = ImGui::Begin(name.data(), &open);
			imgui::binding::endSetNextWindowSizeConstraintsCallbackWrapper(vm);
			ctx.push_value(result);
			ctx.push_value(open);
			return 2;
		}
		auto const name = ctx.get_value<std::string_view>(1);
		imgui::binding::beginSetNextWindowSizeConstraintsCallbackWrapper(vm);
		auto const result = ImGui::Begin(name.data());
		imgui::binding::endSetNextWindowSizeConstraintsCallbackWrapper(vm);
		ctx.push_value(result);
		return 1;
	}
	int End(lua_State*) {
		ImGui::End();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWindows(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Begin"sv, &Begin);
		ctx.set_map_value(m, "End"sv, &End);
	}
}
