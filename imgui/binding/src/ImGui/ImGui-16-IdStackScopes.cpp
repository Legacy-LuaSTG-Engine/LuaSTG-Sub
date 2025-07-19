#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int PushID(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_number(1)) {
			auto const int_id = ctx.get_value<int>(1);
			ImGui::PushID(int_id);
		}
		else if (ctx.is_string(1)) {
			auto const str_id = ctx.get_value<std::string_view>(1);
			ImGui::PushID(str_id.data(), str_id.data() + str_id.size());
		}
		else if (ctx.is_userdata(1) || ctx.is_light_userdata(1)) {
			auto const ptr_id = lua_touserdata(vm, 1);
			ImGui::PushID(ptr_id);
		}
		else {
			luaL_typerror(vm, 1, "number/string/userdata/lightuserdata");
		}
		return 0;
	}
	int PopID(lua_State*) {
		ImGui::PopID();
		return 0;
	}
	int GetID(lua_State* const vm) {
		lua::stack_t const ctx(vm);
		if (ctx.is_number(1)) {
			auto const int_id = ctx.get_value<int>(1);
			auto const result = ImGui::GetID(int_id);
			ctx.push_value(result);
		}
		else if (ctx.is_string(1)) {
			auto const str_id = ctx.get_value<std::string_view>(1);
			auto const result = ImGui::GetID(str_id.data(), str_id.data() + str_id.size());
			ctx.push_value(result);
		}
		else if (ctx.is_userdata(1) || ctx.is_light_userdata(1)) {
			auto const ptr_id = lua_touserdata(vm, 1);
			auto const result = ImGui::GetID(ptr_id);
			ctx.push_value(result);
		}
		else {
			luaL_typerror(vm, 1, "number/string/userdata/lightuserdata");
		}
		return 1;
	}
}

namespace imgui::binding {
	void registerImGuiIdStackScopes(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PushID"sv, &PushID);
		ctx.set_map_value(m, "PopID"sv, &PopID);
		ctx.set_map_value(m, "GetID"sv, &GetID);
	}
}
