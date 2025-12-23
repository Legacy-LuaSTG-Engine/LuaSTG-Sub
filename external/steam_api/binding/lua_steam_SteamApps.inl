#pragma once

struct xSteamApps {
    static int GetCurrentGameLanguage(lua_State* L) {
        const char* lang = SteamApps()->GetCurrentGameLanguage();
        lua_pushstring(L, lang);
        return 1;
    }
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg lib[] = {
            xfbinding(GetCurrentGameLanguage),
            {NULL, NULL},
        };
        lua_pushstring(L, "SteamApps");
        lua_createtable(L, 0, 2);
        luaL_register(L, NULL, lib);
        lua_settable(L, -3);
        return 0;
    };
};

