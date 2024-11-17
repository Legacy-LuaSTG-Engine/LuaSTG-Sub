#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

int luaopen_sqlite3(lua_State* L);

#ifdef __cplusplus
}
#endif
