#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	// TODO: ImColor

	int ColorEdit3(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(3, 0);
		auto result = false;
		if (ctx.is_table(2)) {
			float col[3]{};
			for (int i = 0; i < 3; i += 1) {
				col[i] = ctx.get_array_value<float>(2, i + 1);
			}
			result = ImGui::ColorEdit3(label.data(), col, flags);
			for (int i = 0; i < 3; i += 1) {
				ctx.set_array_value<float>(2, i + 1, col[i]);
			}
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
			result = ImGui::ColorEdit3(label.data(), &col->x, flags);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int ColorEdit4(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(3, 0);
		auto result = false;
		if (ctx.is_table(2)) {
			float col[4]{};
			for (int i = 0; i < 4; i += 1) {
				col[i] = ctx.get_array_value<float>(2, i + 1);
			}
			result = ImGui::ColorEdit4(label.data(), col, flags);
			for (int i = 0; i < 4; i += 1) {
				ctx.set_array_value<float>(2, i + 1, col[i]);
			}
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
			result = ImGui::ColorEdit4(label.data(), &col->x, flags);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int ColorPicker3(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(3, 0);
		auto result = false;
		if (ctx.is_table(2)) {
			float col[3]{};
			for (int i = 0; i < 3; i += 1) {
				col[i] = ctx.get_array_value<float>(2, i + 1);
			}
			result = ImGui::ColorPicker3(label.data(), col, flags);
			for (int i = 0; i < 3; i += 1) {
				ctx.set_array_value<float>(2, i + 1, col[i]);
			}
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
			result = ImGui::ColorPicker3(label.data(), &col->x, flags);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int ColorPicker4(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(3, 0);
		float const* ref_col{};
		float ref_col_arr4[4]{};
		if (ctx.is_table(4)) {
			for (int i = 0; i < 4; i += 1) {
				ref_col_arr4[i] = ctx.get_array_value<float>(4, i + 1);
			}
			ref_col = ref_col_arr4;
		}
		else if (ctx.has_value(4)) {
			auto const ref_col_vec4 = imgui::binding::ImVec4Binding::as(vm, 4);
			ref_col = &ref_col_vec4->x;
		}
		auto result = false;
		if (ctx.is_table(2)) {
			float col[4]{};
			for (int i = 0; i < 4; i += 1) {
				col[i] = ctx.get_array_value<float>(2, i + 1);
			}
			result = ImGui::ColorPicker4(label.data(), col, flags, ref_col);
			for (int i = 0; i < 4; i += 1) {
				ctx.set_array_value<float>(2, i + 1, col[i]);
			}
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
			result = ImGui::ColorPicker4(label.data(), &col->x, flags, ref_col);
		}
		ctx.push_value(result);
		ctx.push_value(lua::stack_index_t(2));
		return 2;
	}
	int ColorButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const desc_id = ctx.get_value<std::string_view>(1);
		auto const col = imgui::binding::ImVec4Binding::as(vm, 2);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(3, 0);
		auto result = false;
		if (ctx.index_of_top() >= 4) {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 4);
			result = ImGui::ColorButton(desc_id.data(), *col, flags, *size);
		}
		else {
			result = ImGui::ColorButton(desc_id.data(), *col, flags);
		}
		ctx.push_value(result);
		return 1;
	}
	int SetColorEditOptions(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const flags = ctx.get_value<ImGuiColorEditFlags>(1);
		ImGui::SetColorEditOptions(flags);
		return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsColorEditorPicker(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "ColorEdit3"sv, &ColorEdit3);
		ctx.set_map_value(m, "ColorEdit4"sv, &ColorEdit4);
		ctx.set_map_value(m, "ColorPicker3"sv, &ColorPicker3);
		ctx.set_map_value(m, "ColorPicker4"sv, &ColorPicker4);
		ctx.set_map_value(m, "ColorButton"sv, &ColorButton);
		ctx.set_map_value(m, "SetColorEditOptions"sv, &SetColorEditOptions);
	}
}
