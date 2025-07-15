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
    template<size_t N, typename F>
    int DragFloatN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        float v[N]{};
        for (int i = 0; i < static_cast<int>(N); i += 1) {
            v[i] = ctx.get_array_value<float>(2, i + 1);
        }
        auto const v_speed = ctx.get_value<float>(3, 1.0f);
        auto const v_min = ctx.get_value<float>(4, 0.0f);
        auto const v_max = ctx.get_value<float>(5, 0.0f);
        auto const format = ctx.get_value<std::string_view>(6, "%.3f"sv);
        auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
        auto const result = f(label.data(), v, v_speed, v_min, v_max, format.data(), flags);
        ctx.push_value(result);
        ctx.push_value(lua::stack_index_t(2));
        return 2;
	}
    int DragFloat2(lua_State* const vm) {
        return DragFloatN<2>(vm, &ImGui::DragFloat2);
    }
    int DragFloat3(lua_State* const vm) {
        return DragFloatN<3>(vm, &ImGui::DragFloat3);
    }
    int DragFloat4(lua_State* const vm) {
        return DragFloatN<4>(vm, &ImGui::DragFloat4);
    }
    int DragFloatRange2(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto v_current_min = ctx.get_value<float>(2);
        auto v_current_max = ctx.get_value<float>(3);
        auto const v_speed = ctx.get_value<float>(4, 1.0f);
        auto const v_min = ctx.get_value<float>(5, 0.0f);
        auto const v_max = ctx.get_value<float>(6, 0.0f);
        auto result = false;
        if (ctx.index_of_top() >= 8) {
            auto const format = ctx.get_value<std::string_view>(7);
            auto const format_max = ctx.get_value<std::string_view>(8);
            auto const flags = ctx.get_value<ImGuiSliderFlags>(9, 0);
            result = ImGui::DragFloatRange2(label.data(), &v_current_min, &v_current_max, v_speed, v_min, v_max, format.data(), format_max.data(), flags);
        }
        else if (ctx.index_of_top() >= 7) {
            auto const format = ctx.get_value<std::string_view>(7);
            result = ImGui::DragFloatRange2(label.data(), &v_current_min, &v_current_max, v_speed, v_min, v_max, format.data());
        }
        else {
            result = ImGui::DragFloatRange2(label.data(), &v_current_min, &v_current_max, v_speed, v_min, v_max);
        }
        ctx.push_value(result);
        ctx.push_value(v_current_min);
        ctx.push_value(v_current_max);
        return 3;
	}
    int DragInt(lua_State* const vm) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto v = ctx.get_value<int>(2);
        auto const v_speed = ctx.get_value<float>(3, 1.0f);
        auto const v_min = ctx.get_value<int>(4, 0);
        auto const v_max = ctx.get_value<int>(5, 0);
        auto result = false;
        if (ctx.index_of_top() >= 6) {
            auto const format = ctx.get_value<std::string_view>(6);
            auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
            result = ImGui::DragInt(label.data(), &v, v_speed, v_min, v_max, format.data(), flags);
        }
        else {
            result = ImGui::DragInt(label.data(), &v, v_speed, v_min, v_max);
        }
        ctx.push_value(result);
        ctx.push_value(v);
        return 2;
	}
    template<size_t N, typename F>
    int DragIntN(lua_State* const vm, F const f) {
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        int v[N]{};
        for (int i = 0; i < static_cast<int>(N); i += 1) {
            v[i] = ctx.get_array_value<int>(2, i + 1);
        }
        auto const v_speed = ctx.get_value<float>(3, 1.0f);
        auto const v_min = ctx.get_value<int>(4, 0);
        auto const v_max = ctx.get_value<int>(5, 0);
        auto const format = ctx.get_value<std::string_view>(6, "%d"sv);
        auto const flags = ctx.get_value<ImGuiSliderFlags>(7, 0);
        auto const result = f(label.data(), v, v_speed, v_min, v_max, format.data(), flags);
        ctx.push_value(result);
        ctx.push_value(lua::stack_index_t(2));
        return 2;
	}
    int DragInt2(lua_State* const vm) {
        return DragIntN<2>(vm, &ImGui::DragInt2);
    }
    int DragInt3(lua_State* const vm) {
        return DragIntN<3>(vm, &ImGui::DragInt3);
    }
    int DragInt4(lua_State* const vm) {
        return DragIntN<4>(vm, &ImGui::DragInt4);
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
        ctx.set_map_value(m, "DragFloatRange2"sv, &DragFloatRange2);
        ctx.set_map_value(m, "DragInt"sv, &DragInt);
        ctx.set_map_value(m, "DragInt2"sv, &DragInt2);
        ctx.set_map_value(m, "DragInt3"sv, &DragInt3);
        ctx.set_map_value(m, "DragInt4"sv, &DragInt4);
	}
}
