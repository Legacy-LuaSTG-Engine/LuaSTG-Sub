#include "AppFrame.h"
#include "LuaWrapper.hpp"
#include "utility/encoding.hpp"

#ifdef LAPP
#undef LAPP
#endif
#define LAPP() (LuaSTGPlus::AppFrame::GetInstance())
#define LWIN() (LuaSTGPlus::AppFrame::GetInstance().GetWindow())

#define getwindow(__NAME__)\
    f2dWindow* __NAME__ = LuaSTGPlus::AppFrame::GetInstance().GetWindow();\
    if (!__NAME__) { return luaL_error(L, "lstg.Window is not available"); }

static int lib_setMouseEnable(lua_State* L)
{
    getwindow(window);
    const bool enable = lua_toboolean(L, 1);
    window->HideMouse(!enable);
    return 0;
}
static int lib_setTitle(lua_State* L)
{
    getwindow(window);
    const char* text = luaL_checkstring(L, 1);
    window->SetCaption(text);
    return 0;
}
static int lib_setCentered(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        const fuInt index = (fuInt)luaL_checkinteger(L, 1);
        window->MoveToMonitorCenter(index);
    }
    else
    {
        window->MoveToCenter();
    }
    return 0;
}
static int lib_setFullScreen(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        const fuInt index = (fuInt)luaL_checkinteger(L, 1);
        window->EnterMonitorFullScreen(index);
    }
    else
    {
        window->EnterFullScreen();
    }
    return 0;
}
static int lib_getFullScreenSize(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        const fuInt index = (fuInt)luaL_checkinteger(L, 1);
        const fcyRect rect = window->GetMonitorRect(index);
        lua_pushnumber(L, rect.GetWidth());
        lua_pushnumber(L, rect.GetHeight());
    }
    else
    {
        const fcyVec2 size = window->GetMonitorSize();
        lua_pushnumber(L, size.x);
        lua_pushnumber(L, size.y);
    }
    return 2;
}
static int lib_setStyle(lua_State* L)
{
    getwindow(window);
    const F2DWINBORDERTYPE style = (F2DWINBORDERTYPE)luaL_checkinteger(L, 1);
    window->SetBorderType(style);
    LAPP().SetDefaultWindowStyle(style); // compat
    return 0;
}
static int lib_setSize(lua_State* L)
{
    getwindow(window);
    const float width = (float)luaL_checkinteger(L, 1);
    const float height = (float)luaL_checkinteger(L, 2);
    const fcyRect rect(0.0f, 0.0f, width, height);
    const fResult result = window->SetClientRect(rect);
    lua_pushboolean(L, result == FCYERR_OK);
    return 1;
}
static int lib_setTopMost(lua_State* L)
{
    getwindow(window);
    const bool topmost = lua_toboolean(L, 1);
    window->SetTopMost(topmost);
    return 0;
}
static int lib_setIMEEnable(lua_State* L)
{
    getwindow(window);
    const bool enable = lua_toboolean(L, 1);
    window->SetIMEEnable(enable);
    return 0;
}
static int lib_getDPIScaling(lua_State* L)
{
    getwindow(window);
    const float dpi_scale = window->GetDPIScaling();
    lua_pushnumber(L, dpi_scale);
    return 1;
}

static int lib_setTextInputEnable(lua_State* L)
{
    const bool enable = lua_toboolean(L, 1);
    return 0;
}
static int lib_getTextInput(lua_State* L)
{
    lua_pushstring(L, "");
    return 1;
}
static int lib_clearTextInput(lua_State* L)
{
    return 0;
}

static int lib_setCustomMoveSizeEnable(lua_State* L)
{
    getwindow(window);
    window->SetCustomMoveSizeEnable(lua_toboolean(L, 1));
    return 0;
}
static int lib_setCustomMinimizeButtonRect(lua_State* L)
{
    getwindow(window);
    window->SetCustomMinimizeButtonRect(fcyRect(
        luaL_checknumber(L, 1),
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3),
        luaL_checknumber(L, 4)
    ));
    return 0;
}
static int lib_setCustomCloseButtonRect(lua_State* L)
{
    getwindow(window);
    window->SetCustomCloseButtonRect(fcyRect(
        luaL_checknumber(L, 1),
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3),
        luaL_checknumber(L, 4)
    ));
    return 0;
}
static int lib_setCustomMoveButtonRect(lua_State* L)
{
    getwindow(window);
    window->SetCustomMoveButtonRect(fcyRect(
        luaL_checknumber(L, 1),
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3),
        luaL_checknumber(L, 4)
    ));
    return 0;
}

static int compat_SetDefaultWindowStyle(lua_State* L)
{
    LAPP().SetDefaultWindowStyle((F2DWINBORDERTYPE)luaL_checkinteger(L, 1));
    return 0;
}
static int compat_SetSplash(lua_State* L)
{
    LAPP().SetSplash(lua_toboolean(L, 1));
    return 0;
}
static int compat_SetTitle(lua_State* L)
{
    LAPP().SetTitle(luaL_checkstring(L, 1));
    return 0;
}

#define makefname(__X__) { #__X__ , &lib_##__X__ }

static const luaL_Reg compat[] = {
    { "SetDefaultWindowStyle", &compat_SetDefaultWindowStyle },
    { "SetSplash", &compat_SetSplash },
    { "SetTitle" , &compat_SetTitle  },
    {NULL, NULL},
};

static const luaL_Reg lib[] = {
    makefname(setMouseEnable),
    makefname(setTitle),
    makefname(setCentered),
    makefname(setFullScreen),
    makefname(getFullScreenSize),
    makefname(setStyle),
    makefname(setSize),
    makefname(setTopMost),
    makefname(setIMEEnable),
    makefname(getDPIScaling),
    
    makefname(setTextInputEnable),
    makefname(getTextInput),
    makefname(clearTextInput),
    
    makefname(setCustomMoveSizeEnable),
    makefname(setCustomMinimizeButtonRect),
    makefname(setCustomCloseButtonRect),
    makefname(setCustomMoveButtonRect),
    
    {NULL, NULL},
};

static int molib_getCount(lua_State* L)
{
    getwindow(window);
    lua_pushinteger(L, (lua_Integer)window->GetMonitorCount());
    return 1;
}
static int molib_getPos(lua_State* L)
{
    getwindow(window);
    const fuInt index = (fuInt)luaL_checkinteger(L, 1);
    const fcyRect rect = window->GetMonitorRect(index);
    lua_pushnumber(L, rect.a.x);
    lua_pushnumber(L, rect.a.y);
    return 2;
}
static int molib_getSize(lua_State* L)
{
    getwindow(window);
    const fuInt index = (fuInt)luaL_checkinteger(L, 1);
    const fcyRect rect = window->GetMonitorRect(index);
    lua_pushnumber(L, rect.GetWidth());
    lua_pushnumber(L, rect.GetHeight());
    return 2;
}

static const luaL_Reg molib[] = {
    { "getCount", &molib_getCount },
    { "getPos"  , &molib_getPos   },
    { "getSize" , &molib_getSize  },
    {NULL, NULL},
};

void LuaSTGPlus::LuaWrapper::WindowWrapper::Register(lua_State* L) LNOEXCEPT
{
    luaL_register(L, LUASTG_LUA_LIBNAME, compat); // ? t
    // Window
    lua_pushstring(L, "Window");                  // ? t k
    lua_newtable(L);                              // ? t k t
    luaL_register(L, NULL, lib);                  // ? t k t
    lua_settable(L, -3);                          // ? t
    // Monitor
    lua_pushstring(L, "Monitor");                 // ? t k
    lua_newtable(L);                              // ? t k t
    luaL_register(L, NULL, molib);                // ? t k t
    lua_settable(L, -3);                          // ? t
    lua_pop(L, 1);                                // ?
};
