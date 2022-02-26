#pragma once
#include "lua.hpp"

#if (LUA_VERSION_NUM >= 504)
    inline void _luaL_reglib(lua_State* L, const char* name)
    {
        struct Wrapper
        {
            static int require(lua_State* L)
            {
                lua_newtable(L);
                return 1;
            }
        };
        luaL_requiref(L, name, &Wrapper::require, true);
    }
    inline int _luaL_len(lua_State* L, int idx)
    {
        return (int)luaL_len(L, idx);
    }
    inline size_t _luaL_szlen(lua_State* L, int idx)
    {
        return (size_t)luaL_len(L, idx);
    }
    inline void _luaL_setfuncs(lua_State* L, const luaL_Reg *l)
    {
        luaL_setfuncs(L, l, 0);
    }
#elif (LUA_VERSION_NUM >= 501)
    inline void _luaL_reglib(lua_State* L, const char* name)
    {
        const luaL_Reg fun[1] = {
            {NULL, NULL}
        };
        luaL_register(L, name, fun);
    }
    inline int _luaL_len(lua_State* L, int idx)
    {
        return (int)lua_objlen(L, idx);
    }
    inline size_t _luaL_szlen(lua_State* L, int idx)
    {
        return lua_objlen(L, idx);
    }
    inline void _luaL_setfuncs(lua_State* L, const luaL_Reg *l)
    {
        luaL_register(L, NULL, l);
    }
#else
#error "not support"
#endif
