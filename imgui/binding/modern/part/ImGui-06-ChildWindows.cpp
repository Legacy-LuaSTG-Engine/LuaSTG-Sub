#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	template<typename T>
	void commonBeginChild(T const id, lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.index_of_top() >= 4) {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const child_flags = ctx.get_value<ImGuiChildFlags>(3);
			auto const window_flags = ctx.get_value<ImGuiWindowFlags>(4);
			auto const result = ImGui::BeginChild(id, *size, child_flags, window_flags);
			ctx.push_value(result);
		}
		else if (ctx.index_of_top() >= 3) {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const child_flags = ctx.get_value<ImGuiChildFlags>(3);
			auto const result = ImGui::BeginChild(id, *size, child_flags);
			ctx.push_value(result);
		}
		else if (ctx.index_of_top() >= 2) {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const result = ImGui::BeginChild(id, *size);
			ctx.push_value(result);
		}
		else {
			auto const result = ImGui::BeginChild(id);
			ctx.push_value(result);
		}
	}
	int BeginChild(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_string(1)) {
			auto const str_id = ctx.get_value<std::string_view>(1);
			commonBeginChild(str_id.data(), vm);
		}
		else {
			auto const id = ctx.get_value<ImGuiID>(1);
			commonBeginChild(id, vm);
		}
		return 1;
	}
	int EndChild(lua_State*) {
		ImGui::EndChild();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiChildWindows(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginChild"sv, &BeginChild);
		ctx.set_map_value(m, "EndChild"sv, &EndChild);
	}
}
