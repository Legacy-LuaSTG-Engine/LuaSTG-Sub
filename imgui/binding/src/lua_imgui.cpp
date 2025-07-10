#include "lua_imgui_common.hpp"
#include "lua_imgui.hpp"
#include "lua_imgui_enum.hpp"
#include "lua_imgui_type.hpp"
#include "lua_imgui_ImGui.hpp"
#include "lua_imgui_binding.hpp"

int luaopen_imgui(lua_State* L)
{
    _luaL_reglib(L, lua_module_imgui);      // ? M

    imgui::binding::registerAll(L);

    imgui_binding_lua_register_enum(L);
    imgui_binding_lua_register_ImGuiStyle(L);
    imgui_binding_lua_register_ImGuiTextBuffer(L);
    imgui_binding_lua_register_ImGui(L);
    
    return 1;
}
