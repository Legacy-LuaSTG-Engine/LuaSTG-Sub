#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int PushStyleColor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiCol>(1);
		if (ctx.is_number(2)) {
			auto const col = ctx.get_value<ImU32>(2);
			ImGui::PushStyleColor(idx, col);
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
			ImGui::PushStyleColor(idx, *col);
		}
		return 0;
	}
	int PopStyleColor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const count = ctx.get_value<int>(1, 1);
		ImGui::PopStyleColor(count);
		return 0;
	}
	int PushStyleVar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiStyleVar>(1);
		if (ctx.is_number(2)) {
			auto const val = ctx.get_value<float>(2);
			ImGui::PushStyleVar(idx, val);
		}
		else {
			auto const val = imgui::binding::ImVec2Binding::as(vm, 2);
			ImGui::PushStyleVar(idx, *val);
		}
		return 0;
	}
	int PushStyleVarX(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiStyleVar>(1);
		auto const val_x = ctx.get_value<float>(2);
		ImGui::PushStyleVarX(idx, val_x);
		return 0;
	}
	int PushStyleVarY(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiStyleVar>(1);
		auto const val_y = ctx.get_value<float>(2);
		ImGui::PushStyleVarY(idx, val_y);
		return 0;
	}
	int PopStyleVar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const count = ctx.get_value<int>(1, 1);
		ImGui::PopStyleVar(count);
		return 0;
	}
	int PushItemFlag(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const option = ctx.get_value<ImGuiItemFlags>(1);
		auto const enabled = ctx.get_value<bool>(2);
		ImGui::PushItemFlag(option, enabled);
		return 0;
	}
	int PopItemFlag(lua_State*) {
		ImGui::PopItemFlag();
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiParametersStacksShared(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PushStyleColor"sv, &PushStyleColor);
		ctx.set_map_value(m, "PopStyleColor"sv, &PopStyleColor);
		ctx.set_map_value(m, "PushStyleVar"sv, &PushStyleVar);
		ctx.set_map_value(m, "PushStyleVarX"sv, &PushStyleVarX);
		ctx.set_map_value(m, "PushStyleVarY"sv, &PushStyleVarY);
		ctx.set_map_value(m, "PopStyleVar"sv, &PopStyleVar);
		ctx.set_map_value(m, "PushItemFlag"sv, &PushItemFlag);
		ctx.set_map_value(m, "PopItemFlag"sv, &PopItemFlag);
	}
}
