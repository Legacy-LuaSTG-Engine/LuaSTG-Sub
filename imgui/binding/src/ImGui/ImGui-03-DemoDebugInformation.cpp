#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int ShowDemoWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto open = ctx.get_value<bool>(1);
			ImGui::ShowDemoWindow(&open);
			ctx.push_value(open);
			return 1;
		}
		ImGui::ShowDemoWindow();
		return 0;
	}
	int ShowMetricsWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto open = ctx.get_value<bool>(1);
			ImGui::ShowMetricsWindow(&open);
			ctx.push_value(open);
			return 1;
		}
		ImGui::ShowMetricsWindow();
		return 0;
	}
	int ShowDebugLogWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto open = ctx.get_value<bool>(1);
			ImGui::ShowDebugLogWindow(&open);
			ctx.push_value(open);
			return 1;
		}
		ImGui::ShowDebugLogWindow();
		return 0;
	}
	int ShowIDStackToolWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto open = ctx.get_value<bool>(1);
			ImGui::ShowIDStackToolWindow(&open);
			ctx.push_value(open);
			return 1;
		}
		ImGui::ShowIDStackToolWindow();
		return 0;
	}
	int ShowAboutWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 1) {
			auto open = ctx.get_value<bool>(1);
			ImGui::ShowAboutWindow(&open);
			ctx.push_value(open);
			return 1;
		}
		ImGui::ShowAboutWindow();
		return 0;
	}
	int ShowStyleEditor(lua_State* const vm) {
		auto const style = imgui::binding::ImGuiStyleBinding::as(vm, 1);
		ImGui::ShowStyleEditor(style);
		return 0;
	}
	int ShowStyleSelector(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const result = ImGui::ShowStyleSelector(label.data());
		ctx.push_value(result);
		return 1;
	}
	int ShowFontSelector(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		ImGui::ShowFontSelector(label.data());
		return 0;
	}
	int ShowUserGuide(lua_State*) {
		ImGui::ShowUserGuide();
		return 0;
	}
	int GetVersion(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetVersion();
		ctx.push_value(std::string_view(result));
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiDemoDebugInformation(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "ShowDemoWindow"sv, &ShowDemoWindow);
		ctx.set_map_value(m, "ShowMetricsWindow"sv, &ShowMetricsWindow);
		ctx.set_map_value(m, "ShowDebugLogWindow"sv, &ShowDebugLogWindow);
		ctx.set_map_value(m, "ShowIDStackToolWindow"sv, &ShowIDStackToolWindow);
		ctx.set_map_value(m, "ShowAboutWindow"sv, &ShowAboutWindow);
		ctx.set_map_value(m, "ShowStyleEditor"sv, &ShowStyleEditor);
		ctx.set_map_value(m, "ShowStyleSelector"sv, &ShowStyleSelector);
		ctx.set_map_value(m, "ShowFontSelector"sv, &ShowFontSelector);
		ctx.set_map_value(m, "ShowUserGuide"sv, &ShowUserGuide);
		ctx.set_map_value(m, "GetVersion"sv, &GetVersion);
	}
}
