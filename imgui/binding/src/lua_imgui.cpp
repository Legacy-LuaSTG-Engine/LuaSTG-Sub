#include "lua_imgui_common.hpp"
#include "lua_imgui.hpp"
#include "lua_imgui_enum.hpp"
#include "lua_imgui_type.hpp"
#include "lua_imgui_ImGui.hpp"
#include "imgui.h"

int luaopen_imgui(lua_State* L)
{
    _luaL_reglib(L, lua_module_imgui);      // ? M
    
    lua_pushstring(L, "IMGUI_VERSION");     // ? M k
    lua_pushstring(L, IMGUI_VERSION);       // ? M k s
    lua_settable(L, -3);                    // ? M
    lua_pushstring(L, "IMGUI_VERSION_NUM"); // ? M k
    lua_pushinteger(L, IMGUI_VERSION_NUM);  // ? M k v
    lua_settable(L, -3);                    // ? M
    
    imgui_binding_lua_register_enum(L);
    imgui_binding_lua_register_ImVec2(L);
    imgui_binding_lua_register_ImVec4(L);
    imgui_binding_lua_register_ImGuiStyle(L);
    imgui_binding_lua_register_ImGuiTextBuffer(L);
    imgui_binding_lua_register_ImGui(L);
    
    return 1;
}
