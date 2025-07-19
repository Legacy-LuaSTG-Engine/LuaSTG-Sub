#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Button(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		if (ctx.index_of_top() >= 2) {
			auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const result = ImGui::Button(label.data(), *size);
			ctx.push_value(result);
		}
		else {
			auto const result = ImGui::Button(label.data());
			ctx.push_value(result);
		}
		return 1;
	}
	int SmallButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const result = ImGui::SmallButton(label.data());
		ctx.push_value(result);
		return 1;
	}
	int InvisibleButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
		auto const flags = ctx.get_value<ImGuiButtonFlags>(3, 0);
		auto const result = ImGui::InvisibleButton(str_id.data(), *size, flags);
		ctx.push_value(result);
		return 1;
	}
	int ArrowButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const dir = ctx.get_value<ImGuiDir>(2);
		auto const result = ImGui::ArrowButton(str_id.data(), dir);
		ctx.push_value(result);
		return 1;
	}
	int Checkbox(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto v = ctx.get_value<bool>(2);
		auto const result = ImGui::Checkbox(label.data(), &v);
		ctx.push_value(result);
		ctx.push_value(v);
		return 2;
	}
	int CheckboxFlags(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		// TODO: int, unsigned int
		auto u_flags = ctx.get_value<unsigned int>(2);
		auto u_flags_value = ctx.get_value<unsigned int>(3);
		auto const result = ImGui::CheckboxFlags(label.data(), &u_flags, u_flags_value);
		ctx.push_value(result);
		ctx.push_value(u_flags);
		return 2;
	}
	int RadioButton(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		if (ctx.index_of_top() >= 3) {
			auto v = ctx.get_value<int>(2);
			auto const v_button = ctx.get_value<int>(3);
			auto const result = ImGui::RadioButton(label.data(), &v, v_button);
			ctx.push_value(result);
			ctx.push_value(v);
			return 2;
		}
		auto const active = ctx.get_value<bool>(2);
		auto const result = ImGui::RadioButton(label.data(), active);
		ctx.push_value(result);
		return 1;
	}
	int ProgressBar(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const fraction = ctx.get_value<float>(1);
		if (ctx.index_of_top() >= 3) {
			auto const size_arg = imgui::binding::ImVec2Binding::as(vm, 2);
			auto const overlay = ctx.get_value<std::string_view>(3);
			ImGui::ProgressBar(fraction, *size_arg, overlay.data());
		}
		else if (ctx.index_of_top() >= 2) {
			auto const size_arg = imgui::binding::ImVec2Binding::as(vm, 2);
			ImGui::ProgressBar(fraction, *size_arg);
		}
		else {
			ImGui::ProgressBar(fraction);
		}
		return 0;
	}
	int Bullet(lua_State*) {
		ImGui::Bullet();
		return 0;
	}
	int TextLink(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const result = ImGui::TextLink(label.data());
		ctx.push_value(result);
		return 1;
	}
	int TextLinkOpenURL(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		if (ctx.index_of_top() >= 2) {
			auto const url = ctx.get_value<std::string_view>(2);
			auto const result = ImGui::TextLinkOpenURL(label.data(), url.data());
			ctx.push_value(result);
		}
		else {
			auto const result = ImGui::TextLinkOpenURL(label.data());
			ctx.push_value(result);
		}
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsMain(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Button"sv, &Button);
		ctx.set_map_value(m, "SmallButton"sv, &SmallButton);
		ctx.set_map_value(m, "InvisibleButton"sv, &InvisibleButton);
		ctx.set_map_value(m, "ArrowButton"sv, &ArrowButton);
		ctx.set_map_value(m, "Checkbox"sv, &Checkbox);
		ctx.set_map_value(m, "CheckboxFlags"sv, &CheckboxFlags);
		ctx.set_map_value(m, "RadioButton"sv, &RadioButton);
		ctx.set_map_value(m, "ProgressBar"sv, &ProgressBar);
		ctx.set_map_value(m, "Bullet"sv, &Bullet);
		ctx.set_map_value(m, "TextLink"sv, &TextLink);
		ctx.set_map_value(m, "TextLinkOpenURL"sv, &TextLinkOpenURL);
	}
}
