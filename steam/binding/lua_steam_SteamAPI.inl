
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
        lua_pushboolean(L, ret);
        return 1;
    };
    static int Shutdown(lua_State* L)
    {
        SteamAPI_Shutdown();
        return 0;
    };
    static int RunCallbacks(lua_State* L)
    {
        const char* M_NAME = SteamCallbackWrapper::MEMBER_NAME;
        lua_pushlightuserdata(L, &xSteamLuaKey); // k
        lua_gettable(L, LUA_REGISTRYINDEX);      // t
        lua_getfield(L, 1, M_NAME);              // t ?
        if (!lua_isuserdata(L, 2))
        { 
            lua_pop(L, 1);                       // t
            SteamCallbackWrapper::xCreate(L);    // t c
            lua_pushvalue(L, 2);                 // t c c
            lua_setfield(L, 1, M_NAME);          // t c
        }
        SteamCallbackWrapper* self = SteamCallbackWrapper::xCast(L, 2);
        if (self)
        {
            self->xRunCallbacks(L);
        }
        else
        {
            SteamAPI_RunCallbacks(); // fallback
        }
        lua_pop(L, 2);
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
