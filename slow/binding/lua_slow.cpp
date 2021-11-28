#include "lua_slow.h"
#include "lua.hpp"
#include "slow/ByteArray.hpp"

namespace slow
{
    struct luaByteArray
    {
        IByteArray* obj;
        
        #define selfname "slow.ByteArray"
        #define getself luaByteArray* self = luaCast(L, 1);
        
        static int __tostring(lua_State* L)
        {
            getself;
            lua_pushstring(L, "slow.ByteArray");
            return 1;
        }
        static int __gc(lua_State* L)
        {
            getself;
            if (self->obj)
                self->obj->release();
            self->obj = nullptr;
            return 0;
        }
        
        static int data(lua_State* L)
        {
            getself;
            lua_pushlightuserdata(L, self->obj->data());
            return 1;
        }
        static int size(lua_State* L)
        {
            getself;
            lua_pushinteger(L, (lua_Integer)self->obj->size());
            return 1;
        }
        static int resizable(lua_State* L)
        {
            getself;
            lua_pushboolean(L, self->obj->resizable());
            return 1;
        }
        static int align(lua_State* L)
        {
            getself;
            lua_pushinteger(L, (lua_Integer)self->obj->align());
            return 1;
        }
        static int resize(lua_State* L)
        {
            getself;
            lua_pushboolean(L, self->obj->resize(
                (u32)luaL_checkinteger(L, 2),
                (u32)luaL_checkinteger(L, 3)
            ));
            return 1;
        }
        
        static int create(lua_State* L)
        {
            lua_Integer size = luaL_checkinteger(L, 1);     // ???
            lua_Integer align = luaL_checkinteger(L, 2);    // ???
            bool resizable = lua_toboolean(L, 3);           // ???
            luaByteArray* self = luaCreate(L);              // ??? T
            if (!createByteArray(&self->obj, (u32)size, (u32)align, resizable))
            {
                lua_pushnil(L);                             // ??? T nil
            }
            return 1;
        }
        
        #undef getself
        
        static void luaRegister(lua_State* L)
        {
            const luaL_Reg mtlib[] = {
                {"__tostring", &__tostring},
                {"__gc", &__gc},
                {NULL, NULL},
            };
            const luaL_Reg clslib[] = {
                {"data", &data},
                {"size", &size},
                {"resizable", &resizable},
                {"align", &align},
                {"resize", &resize},
                {NULL, NULL},
            };
            luaL_newmetatable(L, selfname); // ??? mt
            luaL_register(L, NULL, mtlib);  // ??? mt
            lua_newtable(L);                // ??? mt cls
            luaL_register(L, NULL, clslib); // ??? mt cls
            lua_setfield(L, -2, "__index"); // ??? mt
            lua_pop(L, 1);                  // ???
        }
        static luaByteArray* luaRef(lua_State* L, IByteArray* other)
        {
            luaByteArray* self = (luaByteArray*)lua_newuserdata(L, sizeof(luaByteArray));
            self->obj = other;
            if (self->obj)
                self->obj->reserve();
            luaL_getmetatable(L, selfname);
            lua_setmetatable(L, -2);
            return self;
        }
        static luaByteArray* luaCreate(lua_State* L)
        {
            luaByteArray* self = (luaByteArray*)lua_newuserdata(L, sizeof(luaByteArray));
            self->obj = nullptr;
            luaL_getmetatable(L, selfname);
            lua_setmetatable(L, -2);
            return self;
        }
        static luaByteArray* luaCast(lua_State* L, int idx)
        {
            luaByteArray* self = (luaByteArray*)luaL_checkudata(L, idx, selfname);
            return self;
        }
        
        #undef selfname
    };
}

int luaopen_slow(lua_State* L)
{
    slow::luaByteArray::luaRegister(L);
    
    const luaL_Reg lib[] = {
        {"ByteArray", &slow::luaByteArray::create},
        {NULL, NULL},
    };
    luaL_register(L, "slow", lib);
    return 1;
}
