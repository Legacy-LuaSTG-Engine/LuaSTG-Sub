#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

#define FMT_STRING "%.*s"

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}

	int DebugTextEncoding(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const text = ctx.get_value<std::string_view>(1);
		ImGui::DebugTextEncoding(text.data());
		return 0;
	}
	int DebugFlashStyleColor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiCol>(1);
		ImGui::DebugFlashStyleColor(idx);
		return 0;
	}
	int DebugStartItemPicker(lua_State*) {
		ImGui::DebugStartItemPicker();
		return 0;
	}
	int DebugLog(lua_State* const vm) {
	#ifndef IMGUI_DISABLE_DEBUG_TOOLS
		lua::stack_t const ctx(vm);
		auto const s = ctx.get_value<std::string_view>(1);
		ImGui::DebugLog(FMT_STRING, static_cast<int>(s.size()), s.data());
	#else
		std::ignore = vm;
	#endif
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiDebugUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "DebugTextEncoding"sv, &DebugTextEncoding);
		ctx.set_map_value(m, "DebugFlashStyleColor"sv, &DebugFlashStyleColor);
		ctx.set_map_value(m, "DebugStartItemPicker"sv, &DebugStartItemPicker);
		ctx.set_map_value(m, "DebugCheckVersionAndDataLayout"sv, &notSupported);
		ctx.set_map_value(m, "_DebugLog"sv, &DebugLog);
		ctx.set_map_value(m, "DebugLogV"sv, &notSupported);

		luaL_dostring(vm, R"(
local select = select
local ImGui = require("imgui.ImGui")
function ImGui.DebugLog(fmt, ...)
	if select("#", ...) > 0 then
		ImGui._DebugLog(fmt:format(...))
	else
		ImGui._DebugLog(fmt)
	end
end
		)");
	}
}
