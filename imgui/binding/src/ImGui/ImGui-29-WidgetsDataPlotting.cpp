#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
	struct PlotGetterWrapper {
        static float getter(void* const data, int const idx) {
            auto const vm = static_cast<lua_State*>(data);
            lua_pushvalue(vm, 2);     // ..., getter
            lua_pushvalue(vm, 3);     // ..., getter, data
            lua_pushinteger(vm, idx); // ..., getter, data, idx
            lua_call(vm, 2, 1);       // ..., ?
            if (lua_type(vm, -1) == LUA_TNUMBER) {
                auto const v = lua_tonumber(vm, -1);
                lua_pop(vm, 1);
                return static_cast<float>(v);
            }
            return {};
        }
    };

    int PlotLines(lua_State* const vm) {
        constexpr auto stride_unit = static_cast<int>(sizeof(float));
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        if (ctx.is_table(2)) {
            auto const values_count = !ctx.is_non_or_nil(3) ? ctx.get_value<int>(3) : static_cast<int>(ctx.get_array_size(2));
            if (values_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            auto const stride = ctx.get_value<int>(9, stride_unit);
            if (stride <= 0) {
                luaL_error(vm, "stride is less or equal to 0");
            }
            if ((stride % stride_unit) != 0) {
                luaL_error(vm, "stride must be a multiple of %d", stride_unit);
            }
            std::array<size_t, 1024 / sizeof(size_t)> buf{};
            std::pmr::monotonic_buffer_resource res(buf.data(), 1024, std::pmr::get_default_resource());
            std::pmr::vector<float> values(static_cast<size_t>(values_count), &res);
            for (int i = 0; i < values_count; i += (stride / stride_unit)) {
                values[i] = ctx.get_array_value<float>(2, i + 1);
            }
            auto const values_offset = ctx.get_value<int>(4, 0);
            auto const overlay_text = ctx.get_value<std::string_view>(5, {});
            auto const scale_min = ctx.get_value<float>(6, FLT_MAX);
            auto const scale_max = ctx.get_value<float>(7, FLT_MAX);
            auto const graph_size = imgui::binding::ImVec2Binding::as(vm, 8, ImVec2(0, 0));
            ImGui::PlotLines(label.data(), values.data(), values_count, values_offset, overlay_text.data(), scale_min, scale_max, *graph_size, stride);
        }
        else {
            auto const values_count = ctx.get_value<int>(4);
            if (values_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            auto const values_offset = ctx.get_value<int>(5, 0);
            auto const overlay_text = ctx.get_value<std::string_view>(6, {});
            auto const scale_min = ctx.get_value<float>(7, FLT_MAX);
            auto const scale_max = ctx.get_value<float>(8, FLT_MAX);
            auto const graph_size = imgui::binding::ImVec2Binding::as(vm, 9, ImVec2(0, 0));
            ImGui::PlotLines(label.data(), &PlotGetterWrapper::getter, vm, values_count, values_offset, overlay_text.data(), scale_min, scale_max, *graph_size);
        }
        return 0;
	}
    int PlotHistogram(lua_State* const vm) {
		constexpr auto stride_unit = static_cast<int>(sizeof(float));
		lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        if (ctx.is_table(2)) {
            auto const values_count = !ctx.is_non_or_nil(3) ? ctx.get_value<int>(3) : static_cast<int>(ctx.get_array_size(2));
            if (values_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            auto const stride = ctx.get_value<int>(9, stride_unit);
            if (stride <= 0) {
                luaL_error(vm, "stride is less or equal to 0");
            }
            if ((stride % stride_unit) != 0) {
                luaL_error(vm, "stride must be a multiple of %d", stride_unit);
            }
            std::array<size_t, 1024 / sizeof(size_t)> buf{};
            std::pmr::monotonic_buffer_resource res(buf.data(), 1024, std::pmr::get_default_resource());
            std::pmr::vector<float> values(static_cast<size_t>(values_count), &res);
            for (int i = 0; i < values_count; i += (stride / stride_unit)) {
                values[i] = ctx.get_array_value<float>(2, i + 1);
            }
            auto const values_offset = ctx.get_value<int>(4, 0);
            auto const overlay_text = ctx.get_value<std::string_view>(5, {});
            auto const scale_min = ctx.get_value<float>(6, FLT_MAX);
            auto const scale_max = ctx.get_value<float>(7, FLT_MAX);
            auto const graph_size = imgui::binding::ImVec2Binding::as(vm, 8, ImVec2(0, 0));
            ImGui::PlotHistogram(label.data(), values.data(), values_count, values_offset, overlay_text.data(), scale_min, scale_max, *graph_size, stride);
        }
        else {
            auto const values_count = ctx.get_value<int>(4);
            if (values_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            auto const values_offset = ctx.get_value<int>(5, 0);
            auto const overlay_text = ctx.get_value<std::string_view>(6, {});
            auto const scale_min = ctx.get_value<float>(7, FLT_MAX);
            auto const scale_max = ctx.get_value<float>(8, FLT_MAX);
            auto const graph_size = imgui::binding::ImVec2Binding::as(vm, 9, ImVec2(0, 0));
            ImGui::PlotHistogram(label.data(), &PlotGetterWrapper::getter, vm, values_count, values_offset, overlay_text.data(), scale_min, scale_max, *graph_size);
        }
        return 0;
	}
}

namespace imgui::binding {
	void registerImGuiWidgetsDataPlotting(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "PlotLines"sv, &PlotLines);
		ctx.set_map_value(m, "PlotHistogram"sv, &PlotHistogram);
	}
}
