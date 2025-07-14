#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	int DragFloat(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto v = ctx.get_value<float>(2);
        auto const v_speed = ctx.get_value<float>(3, 1.0f);
        auto const v_min = ctx.get_value<float>(4, 0.0f);
        auto const v_max = ctx.get_value<float>(5, 0.0f);
        auto result = false;
        if (ctx.index_of_top() >= 6) {
            auto const format = ctx.get_value<std::string_view>(6);
            auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
            result = ImGui::DragFloat(label.data(), &v, v_speed, v_min, v_max, format.data(), flags);
        }
        else {
            result = ImGui::DragFloat(label.data(), &v, v_speed, v_min, v_max);
        }
        ctx.push_value(result);
        ctx.push_value(v);
        return 2;
	}
    template<size_t N>
    int DragFloatN(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        float v[N]{};
        for (int i = 0; i < static_cast<int>(N); i += 1) {
            v[i] = ctx.get_array_value<float>(2, i + 1);
        }
        auto const v_speed = ctx.get_value<float>(3, 1.0f);
        auto const v_min = ctx.get_value<float>(4, 0.0f);
        auto const v_max = ctx.get_value<float>(5, 0.0f);
        auto result = false;
        if (ctx.index_of_top() >= 6) {
            auto const format = ctx.get_value<std::string_view>(6);
            auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
            result = ImGui::DragFloat(label.data(), v, v_speed, v_min, v_max, format.data(), flags);
        }
        else {
            result = ImGui::DragFloat(label.data(), v, v_speed, v_min, v_max);
        }
        ctx.push_value(result);
        ctx.push_value(lua::stack_index_t(2));
        return 2;
	}
    int DragFloat2(lua_State* const vm) {
        return DragFloatN<2>(vm);
    }
    int DragFloat3(lua_State* const vm) {
        return DragFloatN<3>(vm);
    }
    int DragFloat4(lua_State* const vm) {
        return DragFloatN<4>(vm);
    }
}

namespace imgui::binding {
	void registerImGuiWidgetsDragSliders(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "DragFloat"sv, &DragFloat);
        ctx.set_map_value(m, "DragFloat2"sv, &DragFloat2);
        ctx.set_map_value(m, "DragFloat3"sv, &DragFloat3);
        ctx.set_map_value(m, "DragFloat4"sv, &DragFloat4);
	}
}
