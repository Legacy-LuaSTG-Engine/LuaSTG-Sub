#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
}

namespace imgui::binding {
	std::string_view const module_ImGui_name{"imgui.ImGui"sv};

	void registerImGuiContextCreationAndAccess(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.create_module(module_ImGui_name);
		ctx.set_map_value(m, "CreateContext"sv, &notSupported);
		ctx.set_map_value(m, "DestroyContext"sv, &notSupported);
		ctx.set_map_value(m, "GetCurrentContext"sv, &notSupported);
		ctx.set_map_value(m, "SetCurrentContext"sv, &notSupported);

		auto const imgui = ctx.push_module(module_name);
		ctx.set_map_value(imgui, "ImGui"sv, m);
	}
}
