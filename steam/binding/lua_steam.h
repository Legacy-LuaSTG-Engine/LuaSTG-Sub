#pragma once
#include <cstdint>
#include "lua.hpp"

bool lua_steam_check(uint32_t appid);
int lua_steam_open(lua_State* L);
