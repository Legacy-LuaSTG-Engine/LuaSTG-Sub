#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Columns(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const count = ctx.get_value<int>(1, 1);
		auto const id = ctx.get_value<std::string_view>(2, {});
		auto const borders = ctx.get_value<bool>(3, true);
		ImGui::Columns(count, id.data(), borders);
		return 0;
	}
	int NextColumn(lua_State*) {
		ImGui::NextColumn();
		return 0;
	}
	int GetColumnIndex(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetColumnIndex();
		ctx.push_value(result);
		return 1;
	}
	int GetColumnWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_index = ctx.get_value<int>(1, -1);
		auto const result = ImGui::GetColumnWidth(column_index);
		ctx.push_value(result);
		return 1;
	}
	int SetColumnWidth(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_index = ctx.get_value<int>(1);
		auto const width = ctx.get_value<float>(2);
		ImGui::SetColumnWidth(column_index, width);
		return 0;
	}
	int GetColumnOffset(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_index = ctx.get_value<int>(1, -1);
		auto const result = ImGui::GetColumnOffset(column_index);
		ctx.push_value(result);
		return 1;
	}
	int SetColumnOffset(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_index = ctx.get_value<int>(1);
		auto const offset_x = ctx.get_value<float>(2);
		ImGui::SetColumnOffset(column_index, offset_x);
		return 0;
	}
	int GetColumnsCount(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetColumnsCount();
		ctx.push_value(result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiLegacyColumns(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Columns"sv, &Columns);
		ctx.set_map_value(m, "NextColumn"sv, &NextColumn);
		ctx.set_map_value(m, "GetColumnIndex"sv, &GetColumnIndex);
		ctx.set_map_value(m, "GetColumnWidth"sv, &GetColumnWidth);
		ctx.set_map_value(m, "SetColumnWidth"sv, &SetColumnWidth);
		ctx.set_map_value(m, "GetColumnOffset"sv, &GetColumnOffset);
		ctx.set_map_value(m, "SetColumnOffset"sv, &SetColumnOffset);
		ctx.set_map_value(m, "GetColumnsCount"sv, &GetColumnsCount);
	}
}
