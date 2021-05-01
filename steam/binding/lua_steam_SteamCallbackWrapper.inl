
class SteamCallbackWrapper
{
public:
    static const char NAME[];
private:
    lua_State* L;
    uint32 _appid;
    struct UserDataHandle
    {
        SteamCallbackWrapper* data;
    };
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
    static int __gc(lua_State* L)
    {
        UserDataHandle* self = (UserDataHandle*)luaL_checkudata(L, 1, NAME);
        if (self->data)
        {
            delete self->data;
            self->data = nullptr;
        }
        return 0;
    };
    
    int xRunCallbacks(lua_State* L2)
    {
        lua_pushlightuserdata(L, xSteamLuaKey); // ... k
        lua_gettable(L, LUA_REGISTRYINDEX);     // ... t
        L = L2;
        SteamAPI_RunCallbacks();
        L = NULL;
        lua_pop(L, 1);                          // ...
        return 0;
    };
    
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg mt[] = {
            {"__gc", &__gc},
            {NULL, NULL},
        };
        luaL_newmetatable(L, NAME);   // ... t mt
        luaL_register(L, NULL, mt);   // ... t mt
        lua_pop(L, 1);                // ... t
        
        struct xSteamCallback
        {
            static int empty(lua_State*)
            {
                return 0;
            };
        };
        static const luaL_Reg lib[] = {
            {"OnNumberOfCurrentPlayers", &xSteamCallback::empty},
            {"OnUserAchievementStored", &xSteamCallback::empty},
            {"OnUserStatsReceived", &xSteamCallback::empty},
            {"OnUserStatsStored", &xSteamCallback::empty},
            {"OnUserStatsUnloaded", &xSteamCallback::empty},
            {NULL, NULL},
        };
        luaL_register(L, NULL, lib);  // ... t
        return 0;
    };
    static int xCreate(lua_State* L)
    {
        UserDataHandle* self = (UserDataHandle*)lua_newuserdata(L, sizeof(UserDataHandle)); // udata
        self->data = nullptr;
        luaL_getmetatable(L, NAME);                                                         // udata mt
        lua_setmetatable(L, -2);                                                            // udata
        try
        {
            self->data = new SteamCallbackWrapper;
        }
        catch (...)
        {
            return luaL_error(L, "unable to create \'steam.SteamCallbackWrapper\' instance");
        }
        return 1;
    };
    static SteamCallbackWrapper* xCast(lua_State* L, int index)
    {
        UserDataHandle* self = (UserDataHandle*)luaL_checkudata(L, index, NAME);
        return self->data;
    };
public:
    SteamCallbackWrapper() : L(NULL)
    {
        _appid = SteamUtils()->GetAppID();
    };
    ~SteamCallbackWrapper() {};
};

const char SteamCallbackWrapper::NAME[] = "steam.SteamCallbackWrapper";

#define MYFUNCTION(_T) void SteamCallbackWrapper::On##_T (_T##_t* pParam)
MYFUNCTION(NumberOfCurrentPlayers)
{
    if (L)
    {
        lua_getfield(L, -1, "OnNumberOfCurrentPlayers");                    // ... t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 2);                                       // ... t f arg
            lua_pushinteger(L, pParam->m_bSuccess);                         // ... t f arg i
            lua_setfield(L, -2, "m_bSuccess");                              // ... t f arg
            lua_pushinteger(L, pParam->m_cPlayers);                         // ... t f arg i
            lua_setfield(L, -2, "m_cPlayers");                              // ... t f arg
            lua_call(L, 1, 0);                                              // ... t
        }
    }
};
MYFUNCTION(UserAchievementStored)
{
    if (L)
    {
        lua_getfield(L, -1, "OnUserAchievementStored");                     // ... t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 5);                                       // ... t f arg
            lua_push_uint64(L, pParam->m_nGameID);                          // ... t f arg i
            lua_setfield(L, -2, "m_nGameID");                               // ... t f arg
            lua_pushboolean(L, pParam->m_bGroupAchievement);                // ... t f arg b
            lua_setfield(L, -2, "m_bGroupAchievement");                     // ... t f arg
            lua_pushstring(L, pParam->m_rgchAchievementName);               // ... t f arg s
            lua_setfield(L, -2, "m_rgchAchievementName");                   // ... t f arg
            lua_push_uint32(L, pParam->m_nCurProgress);                     // ... t f arg u
            lua_setfield(L, -2, "m_nCurProgress");                          // ... t f arg
            lua_push_uint32(L, pParam->m_nMaxProgress);                     // ... t f arg u
            lua_setfield(L, -2, "m_nMaxProgress");                          // ... t f arg
            lua_call(L, 1, 0);                                              // ... t
        }
    }
};
MYFUNCTION(UserStatsReceived)
{
    if (L)
    {
        lua_getfield(L, -1, "OnUserStatsReceived");                         // ... t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 3);                                       // ... t f arg
            lua_push_uint64(L, pParam->m_nGameID);                          // ... t f arg i
            lua_setfield(L, -2, "m_nGameID");                               // ... t f arg
            lua_pushinteger(L, (lua_Integer)pParam->m_eResult);             // ... t f arg i
            lua_setfield(L, -2, "m_eResult");                               // ... t f arg
            lua_push_uint64(L, pParam->m_steamIDUser.ConvertToUint64());    // ... t f arg i
            lua_setfield(L, -2, "m_steamIDUser");                           // ... t f arg
            lua_call(L, 1, 0);                                              // ... t
        }
    }
};
MYFUNCTION(UserStatsStored)
{
    if (L)
    {
        lua_getfield(L, -1, "OnUserStatsStored");                           // ... t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 2);                                       // ... t f arg
            lua_push_uint64(L, pParam->m_nGameID);                          // ... t f arg i
            lua_setfield(L, -2, "m_nGameID");                               // ... t f arg
            lua_pushinteger(L, (lua_Integer)pParam->m_eResult);             // ... t f arg i
            lua_setfield(L, -2, "m_eResult");                               // ... t f arg
            lua_call(L, 1, 0);                                              // ... t
        }
    }
};
MYFUNCTION(UserStatsUnloaded)
{
    if (L)
    {
        lua_getfield(L, -1, "OnUserStatsUnloaded");                         // ... t f
        if (lua_isfunction(L, -1) || lua_iscfunction(L, -1))
        {
            lua_createtable(L, 0, 1);                                       // ... t f arg
            lua_push_uint64(L, pParam->m_steamIDUser.ConvertToUint64());    // ... t f arg i
            lua_setfield(L, -2, "m_steamIDUser");                           // ... t f arg
            lua_call(L, 1, 0);                                              // ... t
        }
    }
};
#undef MYFUNCTION
