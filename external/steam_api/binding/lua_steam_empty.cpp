#include "lua_steam.h"
#include <tuple>

bool lua_steam_check(uint32_t appid)
{
    std::ignore = appid;
    return true;
}
int lua_steam_open(lua_State* L)
{
    const luaL_Reg lib[] = {{NULL, NULL}};
    luaL_register(L, "steam", lib);
    return 1;
}
