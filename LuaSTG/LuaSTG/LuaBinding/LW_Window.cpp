#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "utility/encoding.hpp"
#include "AppFrame.h"

inline Core::Graphics::IWindow* _get_window()
{
    return LuaSTGPlus::AppFrame::GetInstance().GetAppModel()->getWindow();
}

#define getwindow(__NAME__) auto* __NAME__ = _get_window()

static int lib_setMouseEnable(lua_State* L)
{
    getwindow(window);
    bool const enable = lua_toboolean(L, 1);
    if (enable)
        window->setCursor(Core::Graphics::WindowCursor::Arrow);
    else
        window->setCursor(Core::Graphics::WindowCursor::None);
    return 0;
}
static int lib_setCursorStyle(lua_State* L)
{
    getwindow(window);
    Core::Graphics::WindowCursor const style = (Core::Graphics::WindowCursor)luaL_checkinteger(L, 1);
    window->setCursor(style);
    return 0;
}
static int lib_setTitle(lua_State* L)
{
    getwindow(window);
    std::string_view const text = luaL_check_string_view(L, 1);
    window->setTitleText(text);
    return 0;
}
static int lib_setCentered(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        uint32_t const index = (uint32_t)luaL_checkinteger(L, 1);
        window->setMonitorCentered(index);
    }
    else
    {
        window->setCentered();
    }
    return 0;
}
static int lib_setFullScreen(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        uint32_t const index = (uint32_t)luaL_checkinteger(L, 1);
        window->setMonitorFullScreen(index);
    }
    else
    {
        window->setFullScreen();
    }
    return 0;
}
static int lib_getFullScreenSize(lua_State* L)
{
    getwindow(window);
    if (lua_gettop(L) > 0)
    {
        uint32_t const index = (uint32_t)luaL_checkinteger(L, 1);
        Core::RectI const rc = window->getMonitorRect(index);
        lua_pushinteger(L, rc.b.x - rc.a.x);
        lua_pushinteger(L, rc.b.y - rc.a.y);
    }
    else
    {
        Core::Vector2I const size = window->getMonitorSize();
        lua_pushinteger(L, size.x);
        lua_pushinteger(L, size.y);
    }
    return 2;
}
static int lib_setStyle(lua_State* L)
{
    getwindow(window);
    Core::Graphics::WindowFrameStyle style = (Core::Graphics::WindowFrameStyle)luaL_checkinteger(L, 1);
    window->setFrameStyle(style);
    LAPP.SetDefaultWindowStyle(style); // compat
    return 0;
}
static int lib_setSize(lua_State* L)
{
    getwindow(window);
    int32_t const width = (int32_t)luaL_checkinteger(L, 1);
    int32_t const height = (int32_t)luaL_checkinteger(L, 2);
    bool const result = window->setSize(Core::Vector2I(width, height));
    lua_pushboolean(L, result);
    return 1;
}
static int lib_setTopMost(lua_State* L)
{
    getwindow(window);
    bool const topmost = lua_toboolean(L, 1);
    if (topmost)
        window->setLayer(Core::Graphics::WindowLayer::TopMost);
    else
        window->setLayer(Core::Graphics::WindowLayer::Normal);
    return 0;
}
static int lib_setIMEEnable(lua_State* L)
{
    getwindow(window);
    bool const enable = lua_toboolean(L, 1);
    window->setIMEState(enable);
    return 0;
}
static int lib_getDPIScaling(lua_State* L)
{
    getwindow(window);
    float const dpi_scale = window->getDPIScaling();
    lua_pushnumber(L, dpi_scale);
    return 1;
}

static int lib_setTextInputEnable(lua_State*)
{
    //const bool enable = lua_toboolean(L, 1);
    return 0;
}
static int lib_getTextInput(lua_State* L)
{
    lua_pushstring(L, "");
    return 1;
}
static int lib_clearTextInput(lua_State*)
{
    return 0;
}

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

static int compat_SetDefaultWindowStyle(lua_State* L)
{
    LAPP.SetDefaultWindowStyle((Core::Graphics::WindowFrameStyle)luaL_checkinteger(L, 1));
    return 0;
}
static int compat_SetSplash(lua_State* L)
{
    LAPP.SetSplash(lua_toboolean(L, 1));
    return 0;
}
static int compat_SetTitle(lua_State* L)
{
    LAPP.SetTitle(luaL_checkstring(L, 1));
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
    makefname(setCursorStyle),
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
    lua_pushinteger(L, (lua_Integer)window->getMonitorCount());
    return 1;
}
static int molib_getPos(lua_State* L)
{
    getwindow(window);
    uint32_t const index = (uint32_t)luaL_checkinteger(L, 1);
    Core::RectI const rc = window->getMonitorRect(index);
    lua_pushinteger(L, rc.a.x);
    lua_pushinteger(L, rc.a.y);
    return 2;
}
static int molib_getSize(lua_State* L)
{
    getwindow(window);
    uint32_t const index = (uint32_t)luaL_checkinteger(L, 1);
    Core::RectI const rc = window->getMonitorRect(index);
    lua_pushinteger(L, rc.b.x - rc.a.x);
    lua_pushinteger(L, rc.b.y - rc.a.y);
    return 2;
}

static const luaL_Reg molib[] = {
    { "getCount", &molib_getCount },
    { "getPos"  , &molib_getPos   },
    { "getSize" , &molib_getSize  },
    {NULL, NULL},
};

void LuaSTGPlus::LuaWrapper::WindowWrapper::Register(lua_State* L)noexcept
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
