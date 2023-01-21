#include "lua_steam.h"

extern "C" __declspec(dllexport) int luaopen_steam(lua_State* L)
{
    return lua_steam_open(L);
}
