#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int PushItemWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const item_width = ctx.get_value<float>(1);
		ImGui::PushItemWidth(item_width);
		return 0;
	}
	int PopItemWidth(lua_State*) {
		ImGui::PopItemWidth();
		return 0;
	}
	int SetNextItemWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const item_width = ctx.get_value<float>(1);
		ImGui::SetNextItemWidth(item_width);
		return 0;
	}
	int CalcItemWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::CalcItemWidth();
		ctx.push_value(result);
		return 1;
	}
	int PushTextWrapPos(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const wrap_local_pos_x = ctx.get_value<float>(1, 0.5f);
		ImGui::PushTextWrapPos(wrap_local_pos_x);
		return 0;
	}
	int PopTextWrapPos(lua_State*) {
		ImGui::PopTextWrapPos();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiParametersStacksCurrentWindow(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PushItemWidth"sv, &PushItemWidth);
		ctx.set_map_value(m, "PopItemWidth"sv, &PopItemWidth);
		ctx.set_map_value(m, "SetNextItemWidth"sv, &SetNextItemWidth);
		ctx.set_map_value(m, "CalcItemWidth"sv, &CalcItemWidth);
		ctx.set_map_value(m, "PushTextWrapPos"sv, &PushTextWrapPos);
		ctx.set_map_value(m, "PopTextWrapPos"sv, &PopTextWrapPos);
	}
}
