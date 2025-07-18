#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Separator(lua_State*) {
		ImGui::Separator();
		return 0;
	}
	int SameLine(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const offset_from_start_x = ctx.get_value<float>(1, 0.0f);
		auto const spacing = ctx.get_value<float>(2, -1.0f);
		ImGui::SameLine(offset_from_start_x, spacing);
		return 0;
	}
	int NewLine(lua_State*) {
		ImGui::NewLine();
		return 0;
	}
	int Spacing(lua_State*) {
		ImGui::Spacing();
		return 0;
	}
	int Dummy(lua_State* const vm) {
		auto const size = imgui::binding::ImVec2Binding::as(vm, 1);
		ImGui::Dummy(*size);
		return 0;
	}
	int Indent(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const indent_w = ctx.get_value<float>(1, 0.0f);
		ImGui::Indent(indent_w);
		return 0;
	}
	int Unindent(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const indent_w = ctx.get_value<float>(1, 0.0f);
		ImGui::Unindent(indent_w);
		return 0;
	}
	int BeginGroup(lua_State*) {
		ImGui::BeginGroup();
		return 0;
	}
	int EndGroup(lua_State*) {
		ImGui::EndGroup();
		return 0;
	}
	int AlignTextToFramePadding(lua_State*) {
		ImGui::AlignTextToFramePadding();
		return 0;
	}
	int GetTextLineHeight(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetTextLineHeight();
		ctx.push_value(result);
		return 1;
	}
	int GetTextLineHeightWithSpacing(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetTextLineHeightWithSpacing();
		ctx.push_value(result);
		return 1;
	}
	int GetFrameHeight(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetFrameHeight();
		ctx.push_value(result);
		return 1;
	}
	int GetFrameHeightWithSpacing(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetFrameHeightWithSpacing();
		ctx.push_value(result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiOtherLayoutFunctions(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Separator"sv, &Separator);
		ctx.set_map_value(m, "SameLine"sv, &SameLine);
		ctx.set_map_value(m, "NewLine"sv, &NewLine);
		ctx.set_map_value(m, "Spacing"sv, &Spacing);
		ctx.set_map_value(m, "Dummy"sv, &Dummy);
		ctx.set_map_value(m, "Indent"sv, &Indent);
		ctx.set_map_value(m, "Unindent"sv, &Unindent);
		ctx.set_map_value(m, "BeginGroup"sv, &BeginGroup);
		ctx.set_map_value(m, "EndGroup"sv, &EndGroup);
		ctx.set_map_value(m, "AlignTextToFramePadding"sv, &AlignTextToFramePadding);
		ctx.set_map_value(m, "GetTextLineHeight"sv, &GetTextLineHeight);
		ctx.set_map_value(m, "GetTextLineHeightWithSpacing"sv, &GetTextLineHeightWithSpacing);
		ctx.set_map_value(m, "GetFrameHeight"sv, &GetFrameHeight);
		ctx.set_map_value(m, "GetFrameHeightWithSpacing"sv, &GetFrameHeightWithSpacing);
	}
}
