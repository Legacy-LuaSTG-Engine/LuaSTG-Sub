#include "lua_imgui_type.hpp"
#include "lua_imgui_hash.hpp"

struct __ImGuiTextBuffer
{
    ImGuiTextBuffer* data = nullptr;
    ptrdiff_t reft = false;
};

void imgui_binding_lua_register_ImGuiTextBuffer(lua_State* L)
{
    struct Binding
    {
        static int size(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            const int r = self->size();
            lua_pushinteger(L, (lua_Integer)r);
            return 1;
        }
        static int empty(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            const bool r = self->empty();
            lua_pushboolean(L, r);
            return 1;
        }
        static int clear(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            self->clear();
            return 0;
        }
        static int reserve(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            const int capacity = (int)luaL_checkinteger(L, 2);
            self->reserve(capacity);
            return 0;
        }
        static int c_str(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            lua_pushstring(L, self->c_str());
            return 1;
        }
        static int append(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            size_t len = 0;
            const char* str = luaL_checklstring(L, 2, &len);
            self->append(str, str + len);
            return 0;
        }
        
        static int __index(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            switch(lua_type(L, 2))
            {
            case LUA_TNUMBER:
                {
                    const int idx = (int)lua_tointeger(L, 2);
                    if (idx < 0 || idx >= self->size())
                    {
                        return luaL_error(L, "invalid index number, out of range");
                    }
                    const char c[2] = { self->Buf[idx], 0 };
                    lua_pushstring(L, c);
                }
                break;
            case LUA_TSTRING:
                {
                    const char* key = lua_tostring(L, 2);
                    using E = imgui_binding_lua_PropertiesHash;
                    switch (imgui_binding_lua_ComputePropertiesHash(key))
                    {
                    case E::size:
                        lua_pushcfunction(L, &size);
                        break;
                    case E::empty:
                        lua_pushcfunction(L, &empty);
                        break;
                    case E::clear:
                        lua_pushcfunction(L, &clear);
                        break;
                    case E::reserve:
                        lua_pushcfunction(L, &reserve);
                        break;
                    case E::c_str:
                        lua_pushcfunction(L, &c_str);
                        break;
                    case E::append:
                        lua_pushcfunction(L, &append);
                        break;
                    default:
                        return luaL_error(L, "invalid index key");
                    }
                }
                break;
            default:
                return luaL_error(L, "invalid index");
            }
            return 1;
        }
        static int __newindex(lua_State* L)
        {
            ImGuiTextBuffer* self = imgui_binding_lua_to_ImGuiTextBuffer(L, 1);
            switch(lua_type(L, 2))
            {
            case LUA_TNUMBER:
                {
                    const int idx = (int)lua_tointeger(L, 2);
                    if (idx < 0 || idx >= self->size())
                    {
                        return luaL_error(L, "invalid index number, out of range");
                    }
                    const char* c = luaL_checkstring(L, 3);
                    if (c[1] != 0)
                    {
                        return luaL_error(L, "invalid char value");
                    }
                    self->Buf[idx] = c[0];
                }
                break;
            default:
                return luaL_error(L, "invalid index");
            }
            return 0;
        }
        static int __gc(lua_State* L)
        {
            __ImGuiTextBuffer* p = (__ImGuiTextBuffer*)luaL_checkudata(L, 1, imgui_binding_lua_class_ImGuiTextBuffer);
            if (!p->reft)
            {
                delete p->data;
                p->data = nullptr;
            }
            p->reft = false;
            return 0;
        };
        static int __tostring(lua_State* L)
        {
            lua_pushstring(L, imgui_binding_lua_class_ImGuiTextBuffer);
            return 1;
        };
        
        static int create(lua_State* L)
        {
            imgui_binding_lua_new_ImGuiTextBuffer(L);
            return 1;
        };
    };
    
    const luaL_Reg mt_lib[] = {
        {"__index", &Binding::__index},
        {"__newindex", &Binding::__newindex},
        {"__gc", &Binding::__gc},
        {"__tostring", &Binding::__tostring},
        {NULL, NULL},
    };
    
    luaL_newmetatable(L, imgui_binding_lua_class_ImGuiTextBuffer);
    luaL_setfuncs(L, mt_lib, 0);
    lua_pop(L, 1);
    
    const luaL_Reg cls_lib[] = {
        {"ImGuiTextBuffer", &Binding::create},
        {NULL, NULL},
    };
    
    luaL_setfuncs(L, cls_lib, 0);
}
ImGuiTextBuffer* imgui_binding_lua_new_ImGuiTextBuffer(lua_State* L)
{
    __ImGuiTextBuffer* p = (__ImGuiTextBuffer*)lua_newuserdata(L, sizeof(__ImGuiTextBuffer));
    p->data = new ImGuiTextBuffer;
    p->reft = false;
    luaL_getmetatable(L, imgui_binding_lua_class_ImGuiTextBuffer);
    lua_setmetatable(L, -2);
    return p->data;
}
ImGuiTextBuffer* imgui_binding_lua_ref_ImGuiTextBuffer(lua_State* L, ImGuiTextBuffer* v)
{
    __ImGuiTextBuffer* p = (__ImGuiTextBuffer*)lua_newuserdata(L, sizeof(__ImGuiTextBuffer));
    p->data = v;
    p->reft = true;
    luaL_getmetatable(L, imgui_binding_lua_class_ImGuiTextBuffer);
    lua_setmetatable(L, -2);
    return p->data;
}
ImGuiTextBuffer* imgui_binding_lua_to_ImGuiTextBuffer(lua_State* L, int idx)
{
    __ImGuiTextBuffer* p = (__ImGuiTextBuffer*)luaL_checkudata(L, idx, imgui_binding_lua_class_ImGuiTextBuffer);
    return p->data;
}
