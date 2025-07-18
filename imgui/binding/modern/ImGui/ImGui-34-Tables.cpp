#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int notSupported(lua_State* const vm) {
		return luaL_error(vm, "not supported");
	}

	int BeginTable(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const columns = ctx.get_value<int>(2);
		auto const flags = ctx.get_value<ImGuiTableFlags>(3, 0);
		auto const outer_size = imgui::binding::ImVec2Binding::as(vm, 4, ImVec2(0.0f, 0.0f));
		auto const inner_width = ctx.get_value<float>(5, 0.0f);
		auto const result = ImGui::BeginTable(str_id.data(), columns, flags, *outer_size, inner_width);
		ctx.push_value(result);
		return 1;
	}
	int EndTable(lua_State*) {
		ImGui::EndTable();
		return 0;
	}
	int TableNextRow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const row_flags = ctx.get_value<ImGuiTableRowFlags>(1, 0);
		auto const min_row_height = ctx.get_value<float>(2, 0.0f);
		ImGui::TableNextRow(row_flags, min_row_height);
		return 0;
	}
	int TableNextColumn(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::TableNextColumn();
		ctx.push_value(result);
		return 1;
	}
	int TableSetColumnIndex(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_n = ctx.get_value<int>(1);
		auto const result = ImGui::TableSetColumnIndex(column_n);
		ctx.push_value(result);
		return 1;
	}

	int TableSetupColumn(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiTableColumnFlags>(2, 0);
		auto const init_width_or_weight = ctx.get_value<float>(3, 0.0f);
		auto const user_id = ctx.get_value<ImGuiID>(4, 0);
		ImGui::TableSetupColumn(label.data(), flags, init_width_or_weight, user_id);
		return 0;
	}
	int TableSetupScrollFreeze(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const cols = ctx.get_value<int>(1);
		auto const rows = ctx.get_value<int>(2);
		ImGui::TableSetupScrollFreeze(cols, rows);
		return 0;
	}
	int TableHeader(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		ImGui::TableHeader(label.data());
		return 0;
	}
	int TableHeadersRow(lua_State*) {
		ImGui::TableHeadersRow();
		return 0;
	}
	int TableAngledHeadersRow(lua_State*) {
		ImGui::TableAngledHeadersRow();
		return 0;
	}

	int TableGetColumnCount(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::TableGetColumnCount();
		ctx.push_value(result);
		return 1;
	}
	int TableGetColumnIndex(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::TableGetColumnIndex();
		ctx.push_value(result);
		return 1;
	}
	int TableGetRowIndex(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::TableGetRowIndex();
		ctx.push_value(result);
		return 1;
	}
	int TableGetColumnName(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_n = ctx.get_value<int>(1, -1);
		auto const result = ImGui::TableGetColumnName(column_n);
		ctx.push_value(std::string_view(result));
		return 1;
	}
	int TableGetColumnFlags(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_n = ctx.get_value<int>(1, -1);
		auto const result = ImGui::TableGetColumnFlags(column_n);
		ctx.push_value(result);
		return 1;
	}
	int TableSetColumnEnabled(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const column_n = ctx.get_value<int>(1);
		auto const v = ctx.get_value<bool>(2);
		ImGui::TableSetColumnEnabled(column_n, v);
		return 0;
	}
	int TableGetHoveredColumn(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::TableGetHoveredColumn();
		ctx.push_value(result);
		return 1;
	}
	int TableSetBgColor(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const target = ctx.get_value<ImGuiTableBgTarget>(1);
		auto const color = ctx.get_value<ImU32>(2);
		auto const column_n = ctx.get_value<int>(3, -1);
		ImGui::TableSetBgColor(target, color, column_n);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiTables(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);

		ctx.set_map_value(m, "BeginTable"sv, &BeginTable);
		ctx.set_map_value(m, "EndTable"sv, &EndTable);
		ctx.set_map_value(m, "TableNextRow"sv, &TableNextRow);
		ctx.set_map_value(m, "TableNextColumn"sv, &TableNextColumn);
		ctx.set_map_value(m, "TableSetColumnIndex"sv, &TableSetColumnIndex);

		ctx.set_map_value(m, "TableSetupColumn"sv, &TableSetupColumn);
		ctx.set_map_value(m, "TableSetupScrollFreeze"sv, &TableSetupScrollFreeze);
		ctx.set_map_value(m, "TableHeader"sv, &TableHeader);
		ctx.set_map_value(m, "TableHeadersRow"sv, &TableHeadersRow);
		ctx.set_map_value(m, "TableAngledHeadersRow"sv, &TableAngledHeadersRow);

		ctx.set_map_value(m, "TableGetSortSpecs"sv, &notSupported);
		ctx.set_map_value(m, "TableGetColumnCount"sv, &TableGetColumnCount);
		ctx.set_map_value(m, "TableGetColumnIndex"sv, &TableGetColumnIndex);
		ctx.set_map_value(m, "TableGetRowIndex"sv, &TableGetRowIndex);
		ctx.set_map_value(m, "TableGetColumnName"sv, &TableGetColumnName);
		ctx.set_map_value(m, "TableGetColumnFlags"sv, &TableGetColumnFlags);
		ctx.set_map_value(m, "TableSetColumnEnabled"sv, &TableSetColumnEnabled);
		ctx.set_map_value(m, "TableGetHoveredColumn"sv, &TableGetHoveredColumn);
		ctx.set_map_value(m, "TableSetBgColor"sv, &TableSetBgColor);
	}
}
