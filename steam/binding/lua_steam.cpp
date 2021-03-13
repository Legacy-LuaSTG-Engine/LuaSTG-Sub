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

class SteamCallbackWrapper
{
public:
    static const int ID = 0;
    static const int STEAM = 0;
    static bool Installed;
public:
    lua_State* L;
private:
    uint32 _appid;
private:
    #define MYCALLBACK(_T) STEAM_CALLBACK(SteamCallbackWrapper, On##_T, _T##_t)
    //MYCALLBACK(GlobalAchievementPercentagesReady);
    //MYCALLBACK(GlobalStatsReceived);
    //MYCALLBACK(LeaderboardFindResult);
    //MYCALLBACK(LeaderboardScoresDownloaded);
    //MYCALLBACK(LeaderboardScoreUploaded);
    //MYCALLBACK(LeaderboardUGCSet);
    MYCALLBACK(NumberOfCurrentPlayers);
    //MYCALLBACK(UserAchievementIconFetched);
    MYCALLBACK(UserAchievementStored);
    MYCALLBACK(UserStatsReceived);
    MYCALLBACK(UserStatsStored);
    MYCALLBACK(UserStatsUnloaded);
    #undef MYCALLBACK
public:
    static int CommonCallback(lua_State* _)
    {
        (void)_;
        return 0;
    };
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg lib[] = {
            {"OnNumberOfCurrentPlayers", &CommonCallback},
            {"OnUserAchievementStored", &CommonCallback},
            {"OnUserStatsReceived", &CommonCallback},
            {"OnUserStatsStored", &CommonCallback},
            {"OnUserStatsUnloaded", &CommonCallback},
            {NULL, NULL},
        };
        luaL_register(L, NULL, lib);
        return 0;
    };
public:
    SteamCallbackWrapper() : L(NULL)
    {
        _appid = SteamUtils()->GetAppID();
    };
    ~SteamCallbackWrapper() {};
};
bool SteamCallbackWrapper::Installed = false;
#define MYFUNCTION(_T) void SteamCallbackWrapper::On##_T (_T##_t* pParam)
MYFUNCTION(NumberOfCurrentPlayers)
{
    if (L)
    {
        lua_pushlightuserdata(L, (void*)&STEAM);                            // ? p
        lua_gettable(L, LUA_REGISTRYINDEX);                                 // ? t
        lua_getfield(L, -1, "OnNumberOfCurrentPlayers");                    // ? t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 2);                                       // ? t f a
            lua_pushinteger(L, pParam->m_bSuccess);                         // ? t f a i
            lua_setfield(L, -2, "m_bSuccess");                              // ? t f a
            lua_pushinteger(L, pParam->m_cPlayers);                         // ? t f a i
            lua_setfield(L, -2, "m_cPlayers");                              // ? t f a
            lua_call(L, 1, 0);                                              // ? t
        }
        lua_pop(L, 1);                                                      // ?
    }
};
MYFUNCTION(UserAchievementStored)
{
    if (L)
    {
        lua_pushlightuserdata(L, (void*)&STEAM);                            // ? p
        lua_gettable(L, LUA_REGISTRYINDEX);                                 // ? t
        lua_getfield(L, -1, "OnUserAchievementStored");                     // ? t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 5);                                       // ? t f a
            lua_push_uint64(L, pParam->m_nGameID);                          // ? t f a i
            lua_setfield(L, -2, "m_nGameID");                               // ? t f a
            lua_pushboolean(L, pParam->m_bGroupAchievement);                // ? t f a b
            lua_setfield(L, -2, "m_bGroupAchievement");                     // ? t f a
            lua_pushstring(L, pParam->m_rgchAchievementName);               // ? t f a s
            lua_setfield(L, -2, "m_rgchAchievementName");                   // ? t f a
            lua_push_uint32(L, pParam->m_nCurProgress);                     // ? t f a u
            lua_setfield(L, -2, "m_nCurProgress");                          // ? t f a
            lua_push_uint32(L, pParam->m_nMaxProgress);                     // ? t f a u
            lua_setfield(L, -2, "m_nMaxProgress");                          // ? t f a
            lua_call(L, 1, 0);                                              // ? t
        }
        lua_pop(L, 1);                                                      // ?
    }
};
MYFUNCTION(UserStatsReceived)
{
    if (L)
    {
        lua_pushlightuserdata(L, (void*)&STEAM);                            // ? p
        lua_gettable(L, LUA_REGISTRYINDEX);                                 // ? t
        lua_getfield(L, -1, "OnUserStatsReceived");                         // ? t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 3);                                       // ? t f a
            lua_push_uint64(L, pParam->m_nGameID);                          // ? t f a i
            lua_setfield(L, -2, "m_nGameID");                               // ? t f a
            lua_pushinteger(L, (lua_Integer)pParam->m_eResult);             // ? t f a i
            lua_setfield(L, -2, "m_eResult");                               // ? t f a
            lua_push_uint64(L, pParam->m_steamIDUser.ConvertToUint64());    // ? t f a i
            lua_setfield(L, -2, "m_steamIDUser");                           // ? t f a
            lua_call(L, 1, 0);                                              // ? t
        }
        lua_pop(L, 1);                                                      // ?
    }
};
MYFUNCTION(UserStatsStored)
{
    if (L)
    {
        lua_pushlightuserdata(L, (void*)&STEAM);                            // ? p
        lua_gettable(L, LUA_REGISTRYINDEX);                                 // ? t
        lua_getfield(L, -1, "OnUserStatsStored");                           // ? t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 2);                                       // ? t f a
            lua_push_uint64(L, pParam->m_nGameID);                          // ? t f a i
            lua_setfield(L, -2, "m_nGameID");                               // ? t f a
            lua_pushinteger(L, (lua_Integer)pParam->m_eResult);             // ? t f a i
            lua_setfield(L, -2, "m_eResult");                               // ? t f a
            lua_call(L, 1, 0);                                              // ? t
        }
        lua_pop(L, 1);                                                      // ?
    }
};
MYFUNCTION(UserStatsUnloaded)
{
    if (L)
    {
        lua_pushlightuserdata(L, (void*)&STEAM);                            // ? p
        lua_gettable(L, LUA_REGISTRYINDEX);                                 // ? t
        lua_getfield(L, -1, "OnUserStatsUnloaded");                         // ? t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 1);                                       // ? t f a
            lua_push_uint64(L, pParam->m_steamIDUser.ConvertToUint64());    // ? t f a i
            lua_setfield(L, -2, "m_steamIDUser");                           // ? t f a
            lua_call(L, 1, 0);                                              // ? t
        }
        lua_pop(L, 1);                                                      // ?
    }
};
#undef MYFUNCTION

#define lua_push_to_uin64_t(_T)\
    inline int lua_push_##_T (lua_State* L, const _T v) { return lua_push_uint64(L, (uint64_t)v); };\
    inline _T lua_to_##_T (lua_State* L, const int n) { return (_T)lua_to_uint64(L, n, #_T); };

lua_push_to_uin64_t(SteamAPICall_t);

lua_push_to_uin64_t(InputActionSetHandle_t);
lua_push_to_uin64_t(InputAnalogActionHandle_t);
lua_push_to_uin64_t(InputDigitalActionHandle_t);
lua_push_to_uin64_t(InputHandle_t);

#define xfbinding(_X) { #_X , & _X }

struct xSteamAPI
{
    static int RestartAppIfNecessary(lua_State* L)
    {
        const uint32_t unOwnAppID = lua_to_uint32(L, 1);
        const bool ret = SteamAPI_RestartAppIfNecessary(unOwnAppID);
        lua_pushboolean(L, ret);
        return 1;
    };
    static int Init(lua_State* L)
    {
        const bool ret = SteamAPI_Init();
        if (ret && !SteamCallbackWrapper::Installed)
        {
            SteamCallbackWrapper* udata = new SteamCallbackWrapper;
            lua_pushlightuserdata(L, (void*)&SteamCallbackWrapper::ID);
            lua_pushlightuserdata(L, udata);
            lua_settable(L, LUA_REGISTRYINDEX);
            SteamCallbackWrapper::Installed = true;
        }
        lua_pushboolean(L, ret);
        return 1;
    };
    static int Shutdown(lua_State* L)
    {
        if (SteamCallbackWrapper::Installed)
        {
            lua_pushlightuserdata(L, (void*)&SteamCallbackWrapper::ID);
            lua_gettable(L, LUA_REGISTRYINDEX);
            if (lua_islightuserdata(L, -1))
            {
                SteamCallbackWrapper* udata = (SteamCallbackWrapper*)lua_touserdata(L, -1);
                delete udata;
            }
            lua_pop(L, 1);
            lua_pushlightuserdata(L, (void*)&SteamCallbackWrapper::ID);
            lua_pushnil(L);
            lua_settable(L, LUA_REGISTRYINDEX);
            SteamCallbackWrapper::Installed = false;
        }
        SteamAPI_Shutdown();
        return 0;
    };
    static int RunCallbacks(lua_State* L)
    {
        SteamCallbackWrapper* cb = NULL;
        lua_pushlightuserdata(L, (void*)&SteamCallbackWrapper::ID);
        lua_gettable(L, LUA_REGISTRYINDEX);
        if (lua_islightuserdata(L, -1))
            cb = (SteamCallbackWrapper*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        if (cb)
            cb->L = L;
        SteamAPI_RunCallbacks();
        if (cb)
            cb->L = NULL;
        return 0;
    };
    static int ReleaseCurrentThreadMemory(lua_State* L)
    {
        SteamAPI_ReleaseCurrentThreadMemory();
        return 0;
    };
    
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg lib[] = {
            xfbinding(RestartAppIfNecessary),
            xfbinding(Init),
            xfbinding(Shutdown),
            xfbinding(RunCallbacks),
            xfbinding(ReleaseCurrentThreadMemory),
            {NULL, NULL},
        };
        lua_pushstring(L, "SteamAPI");
        lua_createtable(L, 0, 5);
        luaL_register(L, NULL, lib);
        lua_settable(L, -3);
        return 0;
    };
};

#include "lua_steam_SteamInput.inl"
#include "lua_steam_SteamUserStats.inl"

bool lua_steam_check(uint32_t appid)
{
    return !SteamAPI_RestartAppIfNecessary(appid);
}
int lua_steam_open(lua_State* L)
{
    const luaL_Reg lib[] = {{NULL, NULL}};
    luaL_register(L, "steam", lib);
    lua_pushlightuserdata(L, (void*)&SteamCallbackWrapper::STEAM);
    lua_pushvalue(L, -2);
    lua_settable(L, LUA_REGISTRYINDEX);
    SteamCallbackWrapper::xRegister(L);
    xSteamAPI::xRegister(L);
    xSteamInput::xRegister(L);
    xSteamUserStats::xRegister(L);
    return 1;
}
