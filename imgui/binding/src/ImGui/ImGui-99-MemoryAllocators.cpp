#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}
}

namespace imgui::binding {
	void registerImGuiMemoryAllocators(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "SetAllocatorFunctions"sv, &notSupported);
		ctx.set_map_value(m, "GetAllocatorFunctions"sv, &notSupported);
		ctx.set_map_value(m, "MemAlloc"sv, &notSupported);
		ctx.set_map_value(m, "MemFree"sv, &notSupported);
	}
}
