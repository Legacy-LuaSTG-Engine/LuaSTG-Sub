#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"
#include <array>
#include <vector>
#include <memory_resource>

using std::string_view_literals::operator ""sv;

namespace {
    int BeginListBox(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto result = false;
        if (ctx.index_of_top() >= 2) {
            auto const size = imgui::binding::ImVec2Binding::as(vm, 2);
            result = ImGui::BeginListBox(label.data(), *size);
        }
        else {
            result = ImGui::BeginListBox(label.data());
        }
        ctx.push_value(result);
        return 1;
    }
    int EndListBox(lua_State*) {
        ImGui::EndListBox();
        return 0;
    }

    struct ListBoxGetterWrapper {
        static char const* getter(void* const user_data, int const idx) {
            auto const vm = static_cast<lua_State*>(user_data);
            lua_pushvalue(vm, 3);     // ..., getter
            lua_pushvalue(vm, 4);     // ..., getter, user_data
            lua_pushinteger(vm, idx); // ..., getter, user_data, idx
            lua_call(vm, 2, 1);       // ..., ?
            if (lua_type(vm, -1) == LUA_TSTRING) {
                auto const s = lua_tostring(vm, -1);
                // Keep it on the stack to prevent invalid after GC
                //lua_pop(vm, 1);     // ..., string
                return s;
            }
            else {
                lua_pop(vm, 1);       // ...
                return nullptr;
            }
        }
    };

    int ListBox(lua_State* const vm) {
        lua::stack_t const ctx(vm);
        auto const label = ctx.get_value<std::string_view>(1);
        auto current_item = ctx.get_value<int>(2);
        auto result = false;
        if (ctx.is_table(3)) {
            auto const items_count = !ctx.is_non_or_nil(4) ? ctx.get_value<int>(4) : static_cast<int>(ctx.get_array_size(3));
            auto const height_in_items = ctx.get_value<int>(5, -1);
            if (items_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            std::array<size_t, 256 / sizeof(size_t)> buf{};
            std::pmr::monotonic_buffer_resource res(buf.data(), 256, std::pmr::get_default_resource());
            std::pmr::vector<char const*> items(static_cast<size_t>(items_count), &res);
            for (int i = 0; i < items_count; i += 1) {
                auto const item = ctx.get_array_value<std::string_view>(3, i + 1);
                items[i] = item.data();
            }
            result = ImGui::ListBox(label.data(), &current_item, items.data(), items_count, height_in_items);
        }
        else {
            auto const items_count = ctx.get_value<int>(5);
            auto const height_in_items = ctx.get_value<int>(6, -1);
            if (items_count < 0) {
                return luaL_error(vm, "items_count is less then 0");
            }
            result = ImGui::ListBox(label.data(), &current_item, &ListBoxGetterWrapper::getter, vm, items_count, height_in_items);
        }
        ctx.push_value(result);
        ctx.push_value(current_item);
        return 2;
    }
}

namespace imgui::binding {
	void registerImGuiWidgetsListBoxes(lua_State* const vm) {
		lua::stack_balancer_t const sb(vm);
		lua::stack_t const ctx(vm);

		auto const m = ctx.push_module(module_ImGui_name);
		ctx.set_map_value(m, "BeginListBox"sv, &BeginListBox);
        ctx.set_map_value(m, "EndListBox"sv, &EndListBox);
        ctx.set_map_value(m, "ListBox"sv, &ListBox);
	}
}
