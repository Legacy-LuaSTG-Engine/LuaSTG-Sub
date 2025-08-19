#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

#define FMT_STRING "%.*s"

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}

	int BeginTooltip(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::BeginTooltip();
		ctx.push_value(result);
		return 1;
	}
	int EndTooltip(lua_State*) {
		ImGui::EndTooltip();
		return 0;
	}
	int SetTooltip(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::SetTooltip(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}

	int BeginItemTooltip(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::BeginItemTooltip();
		ctx.push_value(result);
		return 1;
	}
	int SetItemTooltip(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::SetItemTooltip(FMT_STRING, static_cast<int>(s.size()), s.data());
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiTooltips(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);

		ctx.set_map_value(m, "BeginTooltip"sv, &BeginTooltip);
		ctx.set_map_value(m, "EndTooltip"sv, &EndTooltip);
		ctx.set_map_value(m, "_SetTooltip"sv, &SetTooltip);
		ctx.set_map_value(m, "SetTooltipV"sv, &notSupported);

		ctx.set_map_value(m, "BeginItemTooltip"sv, &BeginItemTooltip);
		ctx.set_map_value(m, "_SetItemTooltip"sv, &SetItemTooltip);
		ctx.set_map_value(m, "SetItemTooltipV"sv, &notSupported);

		luaL_dostring(vm, R"(
local select = select
local ImGui = require("imgui.ImGui")
function ImGui.SetTooltip(fmt, ...)
	if select("#", ...) > 0 then
		ImGui._SetTooltip(fmt:format(...))
	else
		ImGui._SetTooltip(fmt)
	end
end
function ImGui.SetItemTooltip(fmt, ...)
	if select("#", ...) > 0 then
		ImGui._SetItemTooltip(fmt:format(...))
	else
		ImGui._SetItemTooltip(fmt)
	end
end
		)");
	}
}
