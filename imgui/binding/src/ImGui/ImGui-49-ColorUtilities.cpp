#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int ColorConvertU32ToFloat4(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const in = ctx.get_value<ImU32>(1);
		auto const result = ImGui::ColorConvertU32ToFloat4(in);
		imgui::binding::ImVec4Binding::create(vm, result);
		return 1;
	}
	int ColorConvertFloat4ToU32(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const in = imgui::binding::ImVec4Binding::as(vm, 1);
		auto const result = ImGui::ColorConvertFloat4ToU32(*in);
		ctx.push_value(result);
		return 1;
	}
	int ColorConvertRGBtoHSV(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const r = ctx.get_value<float>(1);
		auto const g = ctx.get_value<float>(2);
		auto const b = ctx.get_value<float>(3);
		float out_h{};
		float out_s{};
		float out_v{};
		ImGui::ColorConvertRGBtoHSV(r, g, b, out_h, out_s, out_v);
		ctx.push_value(out_h);
		ctx.push_value(out_s);
		ctx.push_value(out_v);
		return 3;
	}
	int ColorConvertHSVtoRGB(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const h = ctx.get_value<float>(1);
		auto const s = ctx.get_value<float>(2);
		auto const v = ctx.get_value<float>(3);
		float out_r{};
		float out_g{};
		float out_b{};
		ImGui::ColorConvertHSVtoRGB(h, s, v, out_r, out_g, out_b);
		ctx.push_value(out_r);
		ctx.push_value(out_g);
		ctx.push_value(out_b);
		return 3;
	}
}

namespace imgui::binding {
	void registerImGuiColorUtilities(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "ColorConvertU32ToFloat4"sv, &ColorConvertU32ToFloat4);
		ctx.set_map_value(m, "ColorConvertFloat4ToU32"sv, &ColorConvertFloat4ToU32);
		ctx.set_map_value(m, "ColorConvertRGBtoHSV"sv, &ColorConvertRGBtoHSV);
		ctx.set_map_value(m, "ColorConvertHSVtoRGB"sv, &ColorConvertHSVtoRGB);
	}
}
