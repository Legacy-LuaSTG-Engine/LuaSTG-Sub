#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "AppFrame.h"

inline Core::Graphics::IWindow* _get_window()
{
    return LuaSTGPlus::AppFrame::GetInstance().GetAppModel()->getWindow();
}

#define getwindow(__NAME__) auto* __NAME__ = _get_window()

static int lib_setCustomMoveSizeEnable(lua_State* L)
{
    getwindow(window);
    window->setCustomSizeMoveEnable(lua_toboolean(L, 1));
    return 0;
}
static int lib_setCustomMinimizeButtonRect(lua_State* L)
{
    getwindow(window);
    window->setCustomMinimizeButtonRect(Core::RectI(
        (int32_t)luaL_checkinteger(L, 1),
        (int32_t)luaL_checkinteger(L, 2),
        (int32_t)luaL_checkinteger(L, 3),
        (int32_t)luaL_checkinteger(L, 4)
    ));
    return 0;
}
static int lib_setCustomCloseButtonRect(lua_State* L)
{
    getwindow(window);
    window->setCustomCloseButtonRect(Core::RectI(
        (int32_t)luaL_checkinteger(L, 1),
        (int32_t)luaL_checkinteger(L, 2),
        (int32_t)luaL_checkinteger(L, 3),
        (int32_t)luaL_checkinteger(L, 4)
    ));
    return 0;
}
static int lib_setCustomMoveButtonRect(lua_State* L)
{
    getwindow(window);
    window->setCustomMoveButtonRect(Core::RectI(
        (int32_t)luaL_checkinteger(L, 1),
        (int32_t)luaL_checkinteger(L, 2),
        (int32_t)luaL_checkinteger(L, 3),
        (int32_t)luaL_checkinteger(L, 4)
    ));
    return 0;
}

#define makefname(__X__) { #__X__ , &lib_##__X__ }

static const luaL_Reg lib[] = {
    makefname(setCustomMoveSizeEnable),
    makefname(setCustomMinimizeButtonRect),
    makefname(setCustomCloseButtonRect),
    makefname(setCustomMoveButtonRect),
    {NULL, NULL},
};

void LuaSTGPlus::LuaWrapper::WindowWrapper::Register(lua_State* L)noexcept
{
    luaL_Reg empty_lib[1]{};
    luaL_register(L, LUASTG_LUA_LIBNAME, empty_lib); // ? t
    // Window
    lua_pushstring(L, "Window");                  // ? t k
    lua_newtable(L);                              // ? t k t
    luaL_register(L, NULL, lib);                  // ? t k t
    lua_settable(L, -3);                          // ? t
    lua_pop(L, 1);                                // ?
};
