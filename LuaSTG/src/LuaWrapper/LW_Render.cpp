#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::RenderWrapper::Register(lua_State* L) LNOEXCEPT
{
    struct Wrapper
    {
        static int RenderText(lua_State* L)LNOEXCEPT
        {
            ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Center;
            ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Middle;
            if (lua_gettop(L) == 6)
                TranslateAlignMode(L, 6, halign, valign);
            if (!LAPP.RenderText(
                luaL_checkstring(L, 1),
                luaL_checkstring(L, 2),
                (float)luaL_checknumber(L, 3),
                (float)luaL_checknumber(L, 4),
                (float)(luaL_optnumber(L, 5, 1.0) * LRES.GetGlobalImageScaleFactor()),
                halign,
                valign
                ))
            {
                return luaL_error(L, "can't draw text '%s'.", luaL_checkstring(L, 1));
            }	
            return 0;
        }
        static int RenderTTF(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.RenderTTF(
                luaL_checkstring(L, 1),
                luaL_checkstring(L, 2),
                (float)luaL_checknumber(L, 3),
                (float)luaL_checknumber(L, 4),
                (float)luaL_checknumber(L, 5),
                (float)luaL_checknumber(L, 6),
                LRES.GetGlobalImageScaleFactor() * (float)luaL_optnumber(L, 9, 1.0),
                luaL_checkinteger(L, 7),
                *LuaWrapper::ColorWrapper::Cast(L, 8)
            ))
            {
                return luaL_error(L, "can't render font '%s'.", luaL_checkstring(L, 1));
            }	
            return 0;
        }
        //EX
        static int Snapshot(lua_State* L)LNOEXCEPT
        {
            const char* path = luaL_checkstring(L, 1);
            LAPP.SnapShot(path);
            return 0;
        }
        static int SaveTexture(lua_State* L)LNOEXCEPT
        {
            const char* tex_name = luaL_checkstring(L, 1);
            const char* path = luaL_checkstring(L, 2);
            LAPP.SaveTexture(tex_name, path);
            return 0;
        }
        //EX+
        static int DrawCollider(lua_State*)LNOEXCEPT
        {
            LPOOL.DrawCollider();
            return 1;
        }
        //ETC
        static int RenderGroupCollider(lua_State* L)LNOEXCEPT
        {
            // group color
            LPOOL.DrawGroupCollider2(
                luaL_checkinteger(L, 1),
                *LuaWrapper::ColorWrapper::Cast(L, 2)
            );
            return 0;
        }
    };
    
    luaL_Reg lib[] = {
        ////EX+
        { "DrawCollider", &Wrapper::DrawCollider },
        ////ETC
        { "RenderGroupCollider", &Wrapper::RenderGroupCollider },
        
        { "RenderText", &Wrapper::RenderText },
        { "RenderTTF", &Wrapper::RenderTTF },
        //EX
        { "Snapshot", &Wrapper::Snapshot },
        { "SaveTexture", &Wrapper::SaveTexture },
        // END
        { NULL, NULL },
    };
    
    luaL_register(L, LUASTG_LUA_LIBNAME, lib);
    lua_pop(L, 1);
}
