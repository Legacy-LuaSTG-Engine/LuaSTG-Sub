#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
#define GET_BOOLEAN(API) \
	int API (lua_State* const vm) { \
		lua::stack_t const ctx(vm); \
		auto const result = ImGui:: API (); \
		ctx.push_value(result); \
		return 1; \
	}
#define GET_VEC2(API) \
	int API (lua_State* const vm) { \
		auto const result = ImGui:: API (); \
		imgui::binding::ImVec2Binding::create(vm, result); \
		return 1; \
	}

	int IsItemHovered(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const flags = ctx.get_value<ImGuiHoveredFlags>(1, 0);
		auto const result = ImGui::IsItemHovered(flags);
		ctx.push_value(result);
		return 1;
	}
	GET_BOOLEAN(IsItemActive)
	GET_BOOLEAN(IsItemFocused)
	int IsItemClicked(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const mouse_button = ctx.get_value<ImGuiMouseButton>(1, 0);
		auto const result = ImGui::IsItemClicked(mouse_button);
		ctx.push_value(result);
		return 1;
	}
	GET_BOOLEAN(IsItemVisible)
	GET_BOOLEAN(IsItemEdited)
	GET_BOOLEAN(IsItemActivated)
	GET_BOOLEAN(IsItemDeactivated)
	GET_BOOLEAN(IsItemDeactivatedAfterEdit)
	GET_BOOLEAN(IsItemToggledOpen)
	GET_BOOLEAN(IsAnyItemHovered)
	GET_BOOLEAN(IsAnyItemActive)
	GET_BOOLEAN(IsAnyItemFocused)
	int GetItemID(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const result = ImGui::GetItemID();
		ctx.push_value(result);
		return 1;
	}
	GET_VEC2(GetItemRectMin)
	GET_VEC2(GetItemRectMax)
	GET_VEC2(GetItemRectSize)
}

namespace imgui::binding {
	void registerImGuiItemWidgetsUtilitiesAndQuery(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "IsItemHovered"sv, &IsItemHovered);
		ctx.set_map_value(m, "IsItemActive"sv, &IsItemActive);
		ctx.set_map_value(m, "IsItemFocused"sv, &IsItemFocused);
		ctx.set_map_value(m, "IsItemClicked"sv, &IsItemClicked);
		ctx.set_map_value(m, "IsItemVisible"sv, &IsItemVisible);
		ctx.set_map_value(m, "IsItemEdited"sv, &IsItemEdited);
		ctx.set_map_value(m, "IsItemActivated"sv, &IsItemActivated);
		ctx.set_map_value(m, "IsItemDeactivated"sv, &IsItemDeactivated);
		ctx.set_map_value(m, "IsItemDeactivatedAfterEdit"sv, &IsItemDeactivatedAfterEdit);
		ctx.set_map_value(m, "IsItemToggledOpen"sv, &IsItemToggledOpen);
		ctx.set_map_value(m, "IsAnyItemHovered"sv, &IsAnyItemHovered);
		ctx.set_map_value(m, "IsAnyItemActive"sv, &IsAnyItemActive);
		ctx.set_map_value(m, "IsAnyItemFocused"sv, &IsAnyItemFocused);
		ctx.set_map_value(m, "GetItemID"sv, &GetItemID);
		ctx.set_map_value(m, "GetItemRectMin"sv, &GetItemRectMin);
		ctx.set_map_value(m, "GetItemRectMax"sv, &GetItemRectMax);
		ctx.set_map_value(m, "GetItemRectSize"sv, &GetItemRectSize);
	}
}
