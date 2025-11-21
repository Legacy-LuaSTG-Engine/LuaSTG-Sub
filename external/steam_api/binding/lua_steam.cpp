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
    char buffer[21];
    int len = snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(v));
    if (len < 0 || len >= static_cast<int>(sizeof(buffer))) {
        return luaL_error(L, "uint64_t to string conversion failed");
    }
    lua_pushlstring(L, buffer, len);  
    return 1;
};
uint64_t lua_to_uint64(lua_State* L, const int n, const char* name)
{
    if (!lua_isstring(L, n))
    {
        luaL_typerror(L, n, name);
        return 0;
    }
    const char* str = lua_tostring(L, n);
    uint64_t result = 0;

    if (sscanf(str, "%llu", &result) != 1) {
        luaL_error(L, "Invalid uint64_t value: '%s'", str);
        return 0;
    }

    return result;
};

#define lua_push_to_uin64_t(_T)\
    inline int lua_push_##_T (lua_State* L, const _T v) { return lua_push_uint64(L, (uint64_t)v); };\
    inline _T lua_to_##_T (lua_State* L, const int n) { return (_T)lua_to_uint64(L, n, #_T); };

lua_push_to_uin64_t(SteamAPICall_t);

lua_push_to_uin64_t(InputActionSetHandle_t);
lua_push_to_uin64_t(InputAnalogActionHandle_t);
lua_push_to_uin64_t(InputDigitalActionHandle_t);
lua_push_to_uin64_t(InputHandle_t);

lua_push_to_uin64_t(PublishedFileId_t);

lua_push_to_uin64_t(UGCQueryHandle_t);
lua_push_to_uin64_t(UGCUpdateHandle_t);
lua_push_to_uin64_t(UGCHandle_t);

lua_push_to_uin64_t(SteamLeaderboard_t);
lua_push_to_uin64_t(SteamLeaderboardEntries_t);

#define xfbinding(_X) { #_X , & _X }

static void* xSteamLuaKey = nullptr;

#include "lua_steam_SteamCallbackWrapper.inl"
#include "lua_steam_SteamAPI.inl"
#include "lua_steam_SteamInput.inl"
#include "lua_steam_SteamUserStats.inl"
#include "lua_steam_SteamUtils.inl"
#include "lua_steam_SteamFriends.inl"
#include "lua_steam_SteamUGC.inl"
#include "lua_steam_SteamUser.inl"
#include "lua_steam_SteamApps.inl"

bool lua_steam_check(uint32_t appid)
{
    return !SteamAPI_RestartAppIfNecessary(appid);
}
int lua_steam_open(lua_State* L)
{
    // create and register steam lib
    const luaL_Reg lib[] = { {NULL, NULL} };
    luaL_register(L, "steam", lib);         // t
    lua_pushlightuserdata(L, &xSteamLuaKey); // t k
    lua_pushvalue(L, -2);                   // t k t
    lua_settable(L, LUA_REGISTRYINDEX);     // t
    // register steam modules
    SteamCallbackWrapper::xRegister(L);
    xSteamAPI::xRegister(L);
    xSteamInput::xRegister(L);
    xSteamUserStats::xRegister(L);
    xSteamUtils::xRegister(L);
    xSteamFriends::xRegister(L);
    xSteamUGC::xRegister(L);
    xSteamUser::xRegister(L);
    xSteamApps::xRegister(L);
    return 1;
}