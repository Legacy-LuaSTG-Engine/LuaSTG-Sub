#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
	int BeginCombo(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto const preview_value = ctx.get_value<std::string_view>(2);
		auto const flags = ctx.get_value<ImGuiComboFlags>(3, 0);
		auto const result = ImGui::BeginCombo(label.data(), preview_value.data(), flags);
		ctx.push_value(result);
		return 1;
	}
	int EndCombo(lua_State*) {
		ImGui::EndCombo();
		return 0;
	}
	int Combo(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		auto const label = ctx.get_value<std::string_view>(1);
		auto current_item = ctx.get_value<int>(2);
		auto result = false;
		if (ctx.is_string(3)) {
			auto const items_separated_by_zeros = ctx.get_value<std::string_view>(3);
			auto const popup_max_height_in_items = ctx.get_value<int>(4, -1);
			result = ImGui::Combo(label.data(), &current_item, items_separated_by_zeros.data(), popup_max_height_in_items);
		}
		else if (ctx.is_table(3)) {
			auto const items_count = ctx.get_value<int>(4, static_cast<int>(ctx.get_array_size(3)));
			auto const popup_max_height_in_items = ctx.get_value<int>(5, -1);
			if (items_count < 0) {
				return luaL_argerror(vm, 4, "items_count is less than 0");
			}
			std::array<size_t, 256 / sizeof(size_t)> buf{};
			std::pmr::monotonic_buffer_resource res(buf.data(), 256, std::pmr::get_default_resource());
			std::pmr::vector<char const*> items(static_cast<size_t>(items_count), &res);
			for (int i = 0; i < items_count; i += 1) {
				auto const value = ctx.get_array_value<std::string_view>(3, i + 1);
				items[i] = value.data();
			}
			result = ImGui::Combo(label.data(), &current_item, items.data(), items_count, popup_max_height_in_items);
		}
		else if (ctx.is_function(3)) {
			struct GetterWrapper {
				static char const* getter(void* const user_data, int const idx) {
					auto const vm = static_cast<lua_State*>(user_data);
					// label, current_item, getter, user_data, ...
					lua_pushvalue(vm, 3);     // ..., getter
					lua_pushvalue(vm, 4);     // ..., getter, user_data
					lua_pushinteger(vm, idx); // ..., getter, user_data, idx
					lua_call(vm, 2, 1);       // ..., value
					auto const result = lua_tostring(vm, -1);
					lua_pop(vm, 1);           // ...
					return result;
				}
			};
			auto const items_count = ctx.get_value<int>(5);
			auto const popup_max_height_in_items = ctx.get_value<int>(6, -1);
			result = ImGui::Combo(label.data(), &current_item, GetterWrapper::getter, vm, items_count, popup_max_height_in_items);
		}
		else {
			return luaL_typerror(vm, 3, "string / table -- string[] / function -- fun(user_data:any, idx:integer):string");
		}
		ctx.push_value(result);
		ctx.push_value(current_item);
		return 2;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsComboBoxDropdown(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginCombo"sv, &BeginCombo);
		ctx.set_map_value(m, "EndCombo"sv, &EndCombo);
		ctx.set_map_value(m, "Combo"sv, &Combo);
	}
}
