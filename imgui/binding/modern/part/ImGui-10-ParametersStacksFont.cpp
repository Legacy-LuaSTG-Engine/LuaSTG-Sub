#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
	int GetFontSize(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetFontSize();
		ctx.push_value(result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiParametersStacksFont(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PushFont"sv, &notSupported);
		ctx.set_map_value(m, "PopFont"sv, &notSupported);
		ctx.set_map_value(m, "GetFont"sv, &notSupported);
		ctx.set_map_value(m, "GetFontSize"sv, &GetFontSize);
		ctx.set_map_value(m, "GetFontBaked"sv, &notSupported);
	}
}
