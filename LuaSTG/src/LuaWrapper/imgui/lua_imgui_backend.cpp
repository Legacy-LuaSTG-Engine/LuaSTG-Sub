#include "imgui/lua_imgui_common.hpp"
#include "imgui/lua_imgui_backend.hpp"
#include "Global.h"
#include "AppFrame.h"

static int lib_NewFrame(lua_State* L)
{
    LAPP.UpdateDebugGUI();
    return 0;
}
static int lib_RenderDrawData(lua_State* L)
{
    LAPP.DrawDebugGUI();
    return 0;
}

void imgui_binding_lua_register_backend(lua_State* L)
{
    const luaL_Reg lib_fun[] = {
        {"NewFrame", &lib_NewFrame},
        {"RenderDrawData", &lib_RenderDrawData},
        {NULL, NULL},
    };
    const auto lib_func = (sizeof(lib_fun) / sizeof(luaL_Reg)) - 1;
    
    //                                      // ? m
    lua_pushstring(L, "backend");           // ? m k
    lua_createtable(L, 0, lib_func);        // ? m k t
    luaL_setfuncs(L, lib_fun, 0);           // ? m k t
    lua_settable(L, -3);                    // ? m
}
