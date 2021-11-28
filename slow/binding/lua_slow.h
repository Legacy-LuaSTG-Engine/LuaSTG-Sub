#pragma once
#include "lua.hpp"

#ifndef _LUA_SLOW_DLL_API
#define _LUA_SLOW_DLL_API
#endif

_LUA_SLOW_DLL_API int luaopen_slow(lua_State* L);
