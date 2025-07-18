#include "lua_imgui.hpp"
#include "lua_imgui_binding.hpp"
#include "lua/plus.hpp"

int luaopen_imgui(lua_State* const vm) {
    lua::stack_t const ctx(vm);
    imgui::binding::registerAll(vm);
    std::ignore = ctx.push_module(imgui::binding::module_name);
    return 1;
}
