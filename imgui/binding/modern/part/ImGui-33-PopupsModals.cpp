#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int BeginPopup(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiWindowFlags>(2, 0);
		auto const result = ImGui::BeginPopup(str_id.data(), flags);
		ctx.push_value(result);
		return 1;
	}
	int BeginPopupModal(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const name = ctx.get_value<std::string_view>(1);
		auto p_open = ctx.get_value<bool>(2, false);
		auto const flags = ctx.get_value<ImGuiWindowFlags>(3, 0);
		auto const result = ImGui::BeginPopupModal(name.data(), ctx.has_value(2) ? &p_open : nullptr, flags);
		ctx.push_value(result);
		ctx.push_value(p_open);
		return 2;
	}
	int EndPopup(lua_State*) {
		ImGui::EndPopup();
		return 0;
	}

	int OpenPopup(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const popup_flags = ctx.get_value<ImGuiPopupFlags>(2, 0);
		if (ctx.is_string(1)) {
			auto const str_id = ctx.get_value<std::string_view>(1);
			ImGui::OpenPopup(str_id.data(), popup_flags);
		}
		else {
			auto const id = ctx.get_value<ImGuiID>(1);
			ImGui::OpenPopup(id, popup_flags);
		}
		return 0;
	}
	int OpenPopupOnItemClick(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1, {});
		auto const popup_flags = ctx.get_value<ImGuiPopupFlags>(2, 1); // MAGIC NUMBER
		ImGui::OpenPopupOnItemClick(str_id.data(), popup_flags);
		return 0;
	}
	int CloseCurrentPopup(lua_State*) {
		ImGui::CloseCurrentPopup();
		return 0;
	}

	int BeginPopupContextItem(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1, {});
		auto const popup_flags = ctx.get_value<ImGuiPopupFlags>(2, 1); // MAGIC NUMBER
		auto const result = ImGui::BeginPopupContextItem(str_id.data(), popup_flags);
		ctx.push_value(result);
		return 1;
	}
	int BeginPopupContextWindow(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1, {});
		auto const popup_flags = ctx.get_value<ImGuiPopupFlags>(2, 1); // MAGIC NUMBER
		auto const result = ImGui::BeginPopupContextWindow(str_id.data(), popup_flags);
		ctx.push_value(result);
		return 1;
	}
	int BeginPopupContextVoid(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1, {});
		auto const popup_flags = ctx.get_value<ImGuiPopupFlags>(2, 1); // MAGIC NUMBER
		auto const result = ImGui::BeginPopupContextVoid(str_id.data(), popup_flags);
		ctx.push_value(result);
		return 1;
	}

	int IsPopupOpen(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const str_id = ctx.get_value<std::string_view>(1);
		auto const flags = ctx.get_value<ImGuiPopupFlags>(2, 0);
		auto const result = ImGui::IsPopupOpen(str_id.data(), flags);
		ctx.push_value(result);
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiPopupsModals(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);

		ctx.set_map_value(m, "BeginPopup"sv, &BeginPopup);
		ctx.set_map_value(m, "BeginPopupModal"sv, &BeginPopupModal);
		ctx.set_map_value(m, "EndPopup"sv, &EndPopup);

		ctx.set_map_value(m, "OpenPopup"sv, &OpenPopup);
		ctx.set_map_value(m, "OpenPopupOnItemClick"sv, &OpenPopupOnItemClick);
		ctx.set_map_value(m, "CloseCurrentPopup"sv, &CloseCurrentPopup);

		ctx.set_map_value(m, "BeginPopupContextItem"sv, &BeginPopupContextItem);
		ctx.set_map_value(m, "BeginPopupContextWindow"sv, &BeginPopupContextWindow);
		ctx.set_map_value(m, "BeginPopupContextVoid"sv, &BeginPopupContextVoid);

		ctx.set_map_value(m, "IsPopupOpen"sv, &IsPopupOpen);
	}
}
