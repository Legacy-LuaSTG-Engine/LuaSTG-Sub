#include "lua_steam.h"

int luaopen_steam(lua_State* L)
{
    const luaL_Reg lib[] = {{NULL, NULL}};
    luaL_register(L, "steam", lib);
    return 1;
}
