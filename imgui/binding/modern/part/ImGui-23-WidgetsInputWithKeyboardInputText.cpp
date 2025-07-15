#include "lua_imgui_binding.hpp"
#include "lua_imgui_type.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
    int InputText(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto const buf = imgui_binding_lua_to_ImGuiTextBuffer(vm, 2);
        auto const buf_size = ctx.get_value<size_t>(3, buf->size());
        auto const flags = ctx.get_value<ImGuiInputTextFlags>(4, 0);
        // TODO: supports callback
        auto const result = ImGui::InputText(label.data(), const_cast<char*>(buf->c_str()), buf_size, flags);
        ctx.push_value(result);
        return 1;
	}
    int InputTextMultiline(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto const buf = imgui_binding_lua_to_ImGuiTextBuffer(vm, 2);
        auto const buf_size = ctx.get_value<size_t>(3, buf->size());
        auto const size = imgui::binding::ImVec2Binding::as(vm, 4);
        auto const flags = ctx.get_value<ImGuiInputTextFlags>(5, 0);
        // TODO: supports callback
        auto const result = ImGui::InputTextMultiline(label.data(), const_cast<char*>(buf->c_str()), buf_size, *size, flags);
        ctx.push_value(result);
        return 1;
	}
    int InputTextWithHint(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto const hint = ctx.get_value<std::string_view>(2);
        auto const buf = imgui_binding_lua_to_ImGuiTextBuffer(vm, 3);
        auto const buf_size = ctx.get_value<size_t>(4, buf->size());
        auto const flags = ctx.get_value<ImGuiInputTextFlags>(5, 0);
        // TODO: supports callback
        auto const result = ImGui::InputTextWithHint(label.data(), hint.data(), const_cast<char*>(buf->c_str()), buf_size, flags);
        ctx.push_value(result);
        return 1;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsInputWithKeyboardInputText(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "InputText"sv, &InputText);
        ctx.set_map_value(m, "InputTextMultiline"sv, &InputTextMultiline);
        ctx.set_map_value(m, "InputTextWithHint"sv, &InputTextWithHint);
	}
}
