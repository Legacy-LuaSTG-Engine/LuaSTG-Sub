#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}

	int IsRectVisible(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const size_or_rect_min = imgui::binding::ImVec2Binding::as(vm, 1);
		auto result = false;
		if (ctx.index_of_top() >= 2) {
			auto const rect_max = imgui::binding::ImVec2Binding::as(vm, 2);
			result = ImGui::IsRectVisible(*size_or_rect_min, *rect_max);
		}
		else {
			result = ImGui::IsRectVisible(*size_or_rect_min);
		}
		ctx.push_value(result);
		return 1;
	}
	int GetTime(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetTime();
		ctx.push_value(result);
		return 1;
	}
	int GetFrameCount(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetFrameCount();
		ctx.push_value(result);
		return 1;
	}
	int GetStyleColorName(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiCol>(1);
		auto const result = ImGui::GetStyleColorName(idx);
		ctx.push_value(std::string_view(result));
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiMiscellaneousUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "IsRectVisible"sv, &IsRectVisible);
		ctx.set_map_value(m, "GetTime"sv, &GetTime);
		ctx.set_map_value(m, "GetFrameCount"sv, &GetFrameCount);
		ctx.set_map_value(m, "GetDrawListSharedData"sv, &notSupported);
		ctx.set_map_value(m, "GetStyleColorName"sv, &GetStyleColorName);
		ctx.set_map_value(m, "SetStateStorage"sv, &notSupported);
		ctx.set_map_value(m, "GetStateStorage"sv, &notSupported);
	}
}
