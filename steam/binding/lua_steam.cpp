#include "lua_steam.h"
#include "steam_api.h"

static_assert(sizeof(lua_Integer) >= sizeof(int));
static_assert(sizeof(lua_Integer) >= sizeof(int32_t));
static_assert(sizeof(lua_Integer) >= sizeof(intptr_t));

inline bool str_equal(const char* a, const char* b, const size_t l = SIZE_MAX)
{
    if (l > 0)
    {
        for (size_t i = 0; i < l; i++)
        {
            if (a[i] != b[i])
            {
                return false;
            }
        }
    }
    return true;
};
void lua_push_uint32(lua_State* L, const uint32_t v)
{
    if (sizeof(lua_Integer) >= 8 || v < 0x80000000)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }
    else if (sizeof(lua_Number) >= 8)
    {
        lua_pushnumber(L, (lua_Number)v);
    }
    else
    {
        luaL_error(L, "unable to storage uint32_t value (%u), size of lua_Integer or lua_Number too small", v);
    }
}
unsigned int lua_to_uint(lua_State* L, const int n)
{
    if (sizeof(lua_Integer) >= 8)
    {
        return (unsigned int)luaL_checkinteger(L, n);
    }
    else if (sizeof(unsigned int) <= 4 && sizeof(lua_Number) >= 8)
    {
        return (unsigned int)luaL_checknumber(L, n);
    }
    else
    {
        luaL_error(L, "unable to read unsigned int argument of #%d, size of lua_Integer or lua_Number too small", n);
        return 0;
    }
}
uint32_t lua_to_uint32(lua_State* L, const int n)
{
    if (sizeof(lua_Integer) >= 8)
    {
        return (uint32_t)luaL_checkinteger(L, n);
    }
    else if (sizeof(lua_Number) >= 8)
    {
        return (unsigned int)luaL_checknumber(L, n);
    }
    else
    {
        luaL_error(L, "unable to read uint32_t argument of #%d, size of lua_Integer or lua_Number too small", n);
        return 0;
    }
}
int lua_push_uint64(lua_State* L, const uint64_t v)
{
    const int a = (0xFFFF000000000000 & v) >> 48;
    const int b = (0x0000FFFF00000000 & v) >> 32;
    const int c = (0x00000000FFFF0000 & v) >> 16;
    const int d = (0x000000000000FFFF & v);
    lua_createtable(L, 4, 0); // ? t
    lua_pushinteger(L, a);    // ? t a
    lua_pushinteger(L, b);    // ? t a b
    lua_pushinteger(L, c);    // ? t a b c
    lua_pushinteger(L, d);    // ? t a b c d
    lua_rawseti(L, -5, 1);    // ? t a b c
    lua_rawseti(L, -4, 2);    // ? t a b
    lua_rawseti(L, -3, 3);    // ? t a
    lua_rawseti(L, -2, 4);    // ? t
    return 1;
};
uint64_t lua_to_uint64(lua_State* L, const int n, const char* name)
{
    if (!lua_istable(L, n))
    {
        luaL_typerror(L, n, name);
        return 0;
    }
    lua_rawgeti(L, n, 4); // ? t a
    lua_rawgeti(L, n, 3); // ? t a b
    lua_rawgeti(L, n, 2); // ? t a b c
    lua_rawgeti(L, n, 1); // ? t a b c d
    if (!lua_isnumber(L, -4) || !lua_isnumber(L, -3) || !lua_isnumber(L, -2) || !lua_isnumber(L, -1))
    {
        lua_pop(L, 4);
        luaL_typerror(L, n, name);
        return 0;
    }
    const uint64_t a = (uint64_t)(lua_tointeger(L, -4) & 0xFFFF);
    const uint64_t b = (uint64_t)(lua_tointeger(L, -3) & 0xFFFF);
    const uint64_t c = (uint64_t)(lua_tointeger(L, -2) & 0xFFFF);
    const uint64_t d = (uint64_t)(lua_tointeger(L, -1) & 0xFFFF);
    lua_pop(L, 4);
    return (a << 48) | (b << 32) | (c << 16) | d;
};

#define lua_push_to_uin64_t(_T)\
    inline int lua_push_##_T (lua_State* L, const _T v) { return lua_push_uint64(L, (uint64_t)v); };\
    inline _T lua_to_##_T (lua_State* L, const int n) { return (_T)lua_to_uint64(L, n, #_T); };

lua_push_to_uin64_t(SteamAPICall_t);

lua_push_to_uin64_t(InputActionSetHandle_t);
lua_push_to_uin64_t(InputAnalogActionHandle_t);
lua_push_to_uin64_t(InputDigitalActionHandle_t);
lua_push_to_uin64_t(InputHandle_t);

#define xfbinding(_X) { #_X , & _X }

static void* xSteamLuaKey = nullptr;

#include "lua_steam_SteamCallbackWrapper.inl"
#include "lua_steam_SteamAPI.inl"
#include "lua_steam_SteamInput.inl"
#include "lua_steam_SteamUserStats.inl"

bool lua_steam_check(uint32_t appid)
{
    return !SteamAPI_RestartAppIfNecessary(appid);
}
int lua_steam_open(lua_State* L)
{
    // create and register steam lib
    const luaL_Reg lib[] = {{NULL, NULL}};
    luaL_register(L, "steam", lib);         // t
    lua_pushlightuserdata(L, xSteamLuaKey); // t k
    lua_pushvalue(L, -2);                   // t k t
    lua_settable(L, LUA_REGISTRYINDEX);     // t
    // register steam modules
    SteamCallbackWrapper::xRegister(L);
    xSteamAPI::xRegister(L);
    xSteamInput::xRegister(L);
    xSteamUserStats::xRegister(L);
    return 1;
}
