#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace imgui::binding {
	std::string_view const module_name{ "imgui" };

	void registerGlobal(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_name);
		ctx.set_map_value(m, "IMGUI_VERSION"sv, IMGUI_VERSION ""sv);
		ctx.set_map_value(m, "IMGUI_VERSION_NUM"sv, IMGUI_VERSION_NUM);
		ctx.set_map_value(m, "ImTextureID_Invalid"sv, /* ImTextureID_Invalid */ 0);
	}
}
