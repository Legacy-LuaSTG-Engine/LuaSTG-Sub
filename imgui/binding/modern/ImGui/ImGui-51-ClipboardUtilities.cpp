#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int GetClipboardText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetClipboardText();
		ctx.push_value(std::string_view(result));
		return 1;
	}
	int SetClipboardText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const text = ctx.get_value<std::string_view>(1);
		ImGui::SetClipboardText(text.data());
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiClipboardUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "GetClipboardText"sv, &GetClipboardText);
		ctx.set_map_value(m, "SetClipboardText"sv, &SetClipboardText);
	}
}
