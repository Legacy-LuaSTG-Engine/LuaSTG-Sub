#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int Value(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const prefix = ctx.get_value<std::string_view>(1);
        if (ctx.is_boolean(2)) {
            auto const b = ctx.get_value<bool>(2);
            ImGui::Value(prefix.data(), b);
        }
        else if (ctx.is_number(2)) {
            auto const v = ctx.get_value<float>(2);
            auto const float_format = ctx.get_value<std::string_view>(3, {});
            ImGui::Value(prefix.data(), v, float_format.data());
        }
        else {
            return luaL_typerror(vm, 2, "boolean/number");
        }
        return 0;
	}
    int ValueB(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const prefix = ctx.get_value<std::string_view>(1);
        auto const b = ctx.get_value<bool>(2);
        ImGui::Value(prefix.data(), b);
        return 0;
    }
    int ValueI(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const prefix = ctx.get_value<std::string_view>(1);
        auto const v = ctx.get_value<lua_Number>(2);
        if (v > static_cast<lua_Number>(std::numeric_limits<int>::max())) {
            ImGui::Value(prefix.data(), static_cast<unsigned int>(v));
        }
        else {
            ImGui::Value(prefix.data(), static_cast<int>(v));
        }
        return 0;
    }
    int ValueF(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const prefix = ctx.get_value<std::string_view>(1);
        auto const v = ctx.get_value<float>(2);
        auto const float_format = ctx.get_value<std::string_view>(3, {});
        ImGui::Value(prefix.data(), v, float_format.data());
        return 0;
    }
}

namespace imgui::binding {
	void registerImGuiWidgetsValueHelpers(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "Value"sv, &Value);
        ctx.set_map_value(m, "ValueB"sv, &ValueB);
        ctx.set_map_value(m, "ValueI"sv, &ValueI);
        ctx.set_map_value(m, "ValueF"sv, &ValueF);
	}
}
