#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;
	
void imgui::binding::registerConstantsManually(lua_State* const vm) {
	lua::stack_balancer_t const sb(vm);
	lua::stack_t const ctx(vm);
	auto const m = ctx.push_module(module_name); // imgui
	{
		auto const e = ctx.create_map();
		ctx.set_map_value(m, "ImGuiMod"sv, e);
		ctx.set_map_value(e, "None"sv, ImGuiMod_None);
		ctx.set_map_value(e, "Ctrl"sv, ImGuiMod_Ctrl);
		ctx.set_map_value(e, "Shift"sv, ImGuiMod_Shift);
		ctx.set_map_value(e, "Alt"sv, ImGuiMod_Alt);
		ctx.set_map_value(e, "Super"sv, ImGuiMod_Super);
		ctx.set_map_value(e, "Mask_"sv, ImGuiMod_Mask_);
	}
}
