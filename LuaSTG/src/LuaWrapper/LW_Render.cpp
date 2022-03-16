#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::RenderWrapper::Register(lua_State* L) LNOEXCEPT
{
    struct Wrapper
    {
        static int SetTextureSamplerState(lua_State* L)LNOEXCEPT
        {
            bool ret;
            if (lua_gettop(L) == 2) {
                // string string
                const char* s = luaL_checkstring(L, 1);
                if (strcmp(s, "address") == 0) {
                    // "address" string
                    fcyColor color(0,0,0,0);
                    ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureAddress(TranslateTextureSamplerAddress(L, 2), color));
                    if (!ret) {
                        return luaL_error(L, "Failed to set texture sampler address mode.");
                    }
                }
                else if (strcmp(s, "filter") == 0) {
                    // "filter" string
                    ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureFilter(TranslateTextureSamplerFilter(L, 2)));
                    if (!ret) {
                        return luaL_error(L, "Failed to set texture sampler filter type.");
                    }
                }
                else {
                    return luaL_error(L, "Invalid argument '%m'.",s);
                }
            }
            else if (lua_gettop(L) == 3) {
                // "address" string color
                fcyColor* p = static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
                ret = (FCYERR_OK == LAPP.GetRenderDev()->SetTextureAddress(TranslateTextureSamplerAddress(L, 2), *p));
                if (!ret) {
                    return luaL_error(L, "Failed to set texture sampler address mode.");
                }
            }
            return 0;
        }
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
                *static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR))
            ))
            {
                return luaL_error(L, "can't render font '%s'.", luaL_checkstring(L, 1));
            }	
            return 0;
        }
        static int PushRenderTarget(lua_State* L)LNOEXCEPT
        {
            ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
            if (!p)
                return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
            if (!p->IsRenderTarget())
                return luaL_error(L, "'%s' is a texture.", luaL_checkstring(L, 1));

            if (!LAPP.PushRenderTarget(p))
                return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
            return 0;
        }
        static int PopRenderTarget(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.PopRenderTarget())
                return luaL_error(L, "pop rendertarget failed.");
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
        static int SetZBufferEnable(lua_State* L)LNOEXCEPT
        {
            LAPP.SetZBufferEnable(luaL_checkinteger(L, 1) != 0);
            return 0;
        }
        static int ClearZBuffer(lua_State* L)LNOEXCEPT
        {
            LAPP.ClearZBuffer(luaL_optnumber(L, 1, 1.0f));
            return 0;
        }
        static int RenderModel(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.RenderModel(
                luaL_checkstring(L, 1),
                static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4)),
                static_cast<float>(luaL_optnumber(L, 5, 0)),
                static_cast<float>(luaL_optnumber(L, 6, 0)),
                static_cast<float>(luaL_optnumber(L, 7, 0)),
                static_cast<float>(luaL_optnumber(L, 8, 0)),
                static_cast<float>(luaL_optnumber(L, 9, 0)),
                static_cast<float>(luaL_optnumber(L, 10, 0))
            ))
            {
                return luaL_error(L, "can't render '%s'.", luaL_checkstring(L, 1));
            }
            return 0;
        }
        static int DrawCollider(lua_State* L)LNOEXCEPT
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
                fcyColor(static_cast<fcyColor*>(luaL_checkudata(L, 2, LUASTG_LUA_TYPENAME_COLOR))->argb)
            );
            return 0;
        }
        static int RenderTextureSector(lua_State* L)LNOEXCEPT
        {
            lua_pushboolean(L, LAPP.RenderSector(
                luaL_checkstring(L, 1),
                fcyRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)),
                lua_toboolean(L, 6),
                TranslateBlendMode(L, 7),
                *static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR)),
                *static_cast<fcyColor*>(luaL_checkudata(L, 9, LUASTG_LUA_TYPENAME_COLOR)),
                fcyVec2(luaL_checknumber(L, 10), luaL_checknumber(L, 11)),
                luaL_checknumber(L, 12) * LDEGREE2RAD,
                luaL_checknumber(L, 13) * LDEGREE2RAD,
                luaL_checknumber(L, 14),
                luaL_checknumber(L, 15),
                luaL_checkinteger(L, 16)
            ));
            return 1;
        }
        static int RenderTextureAnnulus(lua_State* L)LNOEXCEPT
        {
            lua_pushboolean(L, LAPP.RenderAnnulus(
                luaL_checkstring(L, 1),
                fcyRect(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)),
                lua_toboolean(L, 6),
                TranslateBlendMode(L, 7),
                *static_cast<fcyColor*>(luaL_checkudata(L, 8, LUASTG_LUA_TYPENAME_COLOR)),
                *static_cast<fcyColor*>(luaL_checkudata(L, 9, LUASTG_LUA_TYPENAME_COLOR)),
                fcyVec2(luaL_checknumber(L, 10), luaL_checknumber(L, 11)),
                luaL_checknumber(L, 12) * LDEGREE2RAD,
                luaL_checknumber(L, 13),
                luaL_checknumber(L, 14),
                luaL_checkinteger(L, 15),
                luaL_checkinteger(L, 16)
            ));
            return 1;
        }
    };
    
    luaL_Reg lib[] = {
        //{ "SetTextureSamplerState", &Wrapper::SetTextureSamplerState },
        //{ "PushRenderTarget", &Wrapper::PushRenderTarget },
        //{ "PopRenderTarget", &Wrapper::PopRenderTarget },
        ////EX+
        //{ "SetZBufferEnable", &Wrapper::SetZBufferEnable },
        //{ "ClearZBuffer", &Wrapper::ClearZBuffer },
        //{ "RenderModel", &Wrapper::RenderModel },
        //{ "DrawCollider", &Wrapper::DrawCollider },
        ////ETC
        //{ "RenderGroupCollider", &Wrapper::RenderGroupCollider },
        //{ "RenderTextureSector", &Wrapper::RenderTextureSector },
        //{ "RenderTextureAnnulus", &Wrapper::RenderTextureAnnulus },

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
