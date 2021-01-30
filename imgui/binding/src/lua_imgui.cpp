#include "lua_imgui_common.hpp"
#include "lua_imgui.hpp"
#include "lua_imgui_enum.hpp"
#include "lua_imgui_type.hpp"
#include "lua_imgui_ImGui.hpp"
#include "imgui.h"

int luaopen_imgui(lua_State* L)
{
    _imgui_binding_lua_new_library(L, "imgui");
    
    //                                      // ? m
    lua_pushstring(L, "IMGUI_VERSION");     // ? m k
    lua_pushstring(L, IMGUI_VERSION);       // ? m k s
    lua_settable(L, -3);                    // ? m
    lua_pushstring(L, "IMGUI_VERSION_NUM"); // ? m k
    lua_pushinteger(L, IMGUI_VERSION_NUM);  // ? m k v
    lua_settable(L, -3);                    // ? m
    
    imgui_binding_lua_register_enum(L);
    imgui_binding_lua_register_ImVec2(L);
    imgui_binding_lua_register_ImVec4(L);
    imgui_binding_lua_register_ImGuiStyle(L);
    imgui_binding_lua_register_ImGuiTextBuffer(L);
    imgui_binding_lua_register_ImGui(L);
    
    return 1;
}
