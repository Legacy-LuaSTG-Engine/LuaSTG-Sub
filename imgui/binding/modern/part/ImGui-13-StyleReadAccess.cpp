#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int GetFontTexUvWhitePixel(lua_State* const vm) {
		auto const result = ImGui::GetFontTexUvWhitePixel();
		imgui::binding::ImVec2Binding::create(vm, result);
		return 1;
	}
	int GetColorU32(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_number(1)) {
			auto const col = ctx.get_value<ImU32>(1);
			auto const alpha_mul = ctx.get_value<float>(2, 1.0f);
			auto const result = ImGui::GetColorU32(col, alpha_mul);
			ctx.push_value(result);
		}
		else {
			auto const col = imgui::binding::ImVec4Binding::as(vm, 1);
			auto const result = ImGui::GetColorU32(*col);
			ctx.push_value(result);
		}
		return 1;
	}
	int GetStyleColorU32(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiCol>(1);
		auto const alpha_mul = ctx.get_value<float>(2, 1.0f);
		auto const result = ImGui::GetColorU32(idx, alpha_mul);
		ctx.push_value(result);
		return 1;
	}
	int GetStyleColorVec4(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const idx = ctx.get_value<ImGuiCol>(1);
		auto const result = ImGui::GetStyleColorVec4(idx);
		imgui::binding::ImVec4Binding::create(vm, result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiStyleReadAccess(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "GetFontTexUvWhitePixel"sv, &GetFontTexUvWhitePixel);
		ctx.set_map_value(m, "GetColorU32"sv, &GetColorU32);
		ctx.set_map_value(m, "GetStyleColorU32"sv, &GetStyleColorU32);
		ctx.set_map_value(m, "GetStyleColorVec4"sv, &GetStyleColorVec4);
	}
}
