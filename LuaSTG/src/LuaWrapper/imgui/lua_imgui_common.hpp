#pragma once
#include "lua.hpp"

#if (LUA_VERSION_NUM == 501)
    inline void _imgui_binding_lua_new_library(lua_State *L, const char* name)
    {
        const luaL_Reg fun[1] = { {NULL, NULL} };
        luaL_register(L, name, fun);
    }
    inline int _imgui_binding_lua_len(lua_State *L, int idx)
    {
        return (int)lua_objlen(L, idx);
    }
    inline size_t _imgui_binding_lua_len_size_t(lua_State *L, int idx)
    {
        return lua_objlen(L, idx);
    }
#elif (LUA_VERSION_NUM == 504)
    inline void _imgui_binding_lua_new_library(lua_State *L, const char* name)
    {
        struct _require_wrapper
        {
            static int create(lua_State* L)
            {
                lua_newtable(L);
                return 1;
            }
        };
        luaL_requiref(L, name, &_require_wrapper::create, false);
    }
    inline int _imgui_binding_lua_len(lua_State *L, int idx)
    {
        lua_len(L, idx);
        const int len = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        return len;
    }
    inline size_t _imgui_binding_lua_len_size_t(lua_State *L, int idx)
    {
        lua_len(L, idx);
        const size_t len = (size_t)lua_tointeger(L, -1);
        lua_pop(L, 1);
        return len;
    }
#else
#error "not support"
#endif
