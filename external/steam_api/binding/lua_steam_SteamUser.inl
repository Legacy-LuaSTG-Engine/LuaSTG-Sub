#pragma once

struct xSteamUser {
    static int GetSteamID(lua_State* L)
    {
        CSteamID steamID = SteamUser()->GetSteamID();
        lua_createtable(L, 0, 1);
        lua_push_uint32(L, steamID.GetAccountID());
        lua_setfield(L, -2, "AccountID");
        return 1;
    };
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg lib[] = {
            xfbinding(GetSteamID),
            {NULL, NULL},
        };
        lua_pushstring(L, "SteamUser");
        lua_createtable(L, 0, 2);
        luaL_register(L, NULL, lib);
        lua_settable(L, -3);
        return 0;
    };
};