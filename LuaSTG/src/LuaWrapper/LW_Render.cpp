#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"
#include "LConfig.h"

void LuaSTGPlus::LuaWrapper::RenderWrapper::Register(lua_State* L) LNOEXCEPT
{
    struct Wrapper
    {
        static int BeginScene(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.BeginScene())
                return luaL_error(L, "can't invoke 'BeginScene'.");
            return 0;
        }
        static int EndScene(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.EndScene())
                return luaL_error(L, "can't invoke 'EndScene'.");
            return 0;
        }
        static int RenderClear(lua_State* L)LNOEXCEPT
        {
            fcyColor* c = static_cast<fcyColor*>(luaL_checkudata(L, 1, LUASTG_LUA_TYPENAME_COLOR));
            LAPP.ClearScreen(*c);
            return 0;
        }
        static int SetViewport(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.SetViewport(
                luaL_checknumber(L, 1),
                luaL_checknumber(L, 2),
                luaL_checknumber(L, 3),
                luaL_checknumber(L, 4)
            ))
            {
                return luaL_error(L, "invalid arguments for 'SetViewport'.");
            }
            return 0;
        }
        static int SetScissorRect(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.SetScissorRect(
                luaL_checknumber(L, 1),
                luaL_checknumber(L, 2),
                luaL_checknumber(L, 3),
                luaL_checknumber(L, 4)
            ))
            {
                return luaL_error(L, "invalid arguments for 'SetScissorRect'.");
            }
            return 0;
        }
        static int SetOrtho(lua_State* L)LNOEXCEPT
        {
            int top_n = lua_gettop(L);
            switch (top_n) {
            case 4:
                LAPP.SetOrtho(
                    static_cast<float>(luaL_checknumber(L, 1)),
                    static_cast<float>(luaL_checknumber(L, 2)),
                    static_cast<float>(luaL_checknumber(L, 3)),
                    static_cast<float>(luaL_checknumber(L, 4))
                );
                break;
            case 6:
                LAPP.SetOrtho(
                    static_cast<float>(luaL_checknumber(L, 1)),
                    static_cast<float>(luaL_checknumber(L, 2)),
                    static_cast<float>(luaL_checknumber(L, 3)),
                    static_cast<float>(luaL_checknumber(L, 4)),
                    static_cast<float>(luaL_checknumber(L, 5)),
                    static_cast<float>(luaL_checknumber(L, 6))
                );
                break;
            }
            return 0;
        }
        static int SetPerspective(lua_State* L)LNOEXCEPT
        {
            LAPP.SetPerspective(
                static_cast<float>(luaL_checknumber(L, 1)),
                static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4)),
                static_cast<float>(luaL_checknumber(L, 5)),
                static_cast<float>(luaL_checknumber(L, 6)),
                static_cast<float>(luaL_checknumber(L, 7)),
                static_cast<float>(luaL_checknumber(L, 8)),
                static_cast<float>(luaL_checknumber(L, 9)),
                static_cast<float>(luaL_checknumber(L, 10)),
                static_cast<float>(luaL_checknumber(L, 11)),
                static_cast<float>(luaL_checknumber(L, 12)),
                static_cast<float>(luaL_checknumber(L, 13))
            );
            return 0;
        }
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
        static int Render(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.Render(
                luaL_checkstring(L, 1),
                static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_optnumber(L, 4, 0.) * LDEGREE2RAD),
                static_cast<float>(luaL_optnumber(L, 5, 1.) * LRES.GetGlobalImageScaleFactor()),
                static_cast<float>(luaL_optnumber(L, 6, luaL_optnumber(L, 5, 1.)) * LRES.GetGlobalImageScaleFactor()),
                static_cast<float>(luaL_optnumber(L, 7, 0.5))
            ))
            {
                return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
            }
            return 0;
        }
        static int RenderAnimation(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.RenderAnimation(
                luaL_checkstring(L, 1),
                luaL_checkinteger(L,2),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4)),
                static_cast<float>(luaL_optnumber(L, 5, 0.) * LDEGREE2RAD),
                static_cast<float>(luaL_optnumber(L, 6, 1.) * LRES.GetGlobalImageScaleFactor()),
                static_cast<float>(luaL_optnumber(L, 7, luaL_optnumber(L, 6, 1.)) * LRES.GetGlobalImageScaleFactor())
            ))
            {
                return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
            }
            return 0;
        }
        static int RenderRect(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.RenderRect(
                luaL_checkstring(L, 1),
                static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 5)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4))
            ))
            {
                return luaL_error(L, "can't render '%s'", luaL_checkstring(L, 1));
            }
            return 0;
        }
        static int Render4V(lua_State* L)LNOEXCEPT
        {
            if (!LAPP.Render4V(
                luaL_checkstring(L, 1),
                static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4)),
                static_cast<float>(luaL_checknumber(L, 5)),
                static_cast<float>(luaL_checknumber(L, 6)),
                static_cast<float>(luaL_checknumber(L, 7)),
                static_cast<float>(luaL_checknumber(L, 8)),
                static_cast<float>(luaL_checknumber(L, 9)),
                static_cast<float>(luaL_checknumber(L, 10)),
                static_cast<float>(luaL_checknumber(L, 11)),
                static_cast<float>(luaL_checknumber(L, 12)),
                static_cast<float>(luaL_checknumber(L, 13))
            ))
            {
                return luaL_error(L, "can't render '%s'.", luaL_checkstring(L, 1));
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
        static int RenderTexture(lua_State* L)LNOEXCEPT
        {
            const char* tex_name = luaL_checkstring(L, 1);
            BlendMode blend = TranslateBlendMode(L, 2);
            f2dGraphics2DVertex vertex[4];

            for (int i = 0; i < 4; ++i)
            {
                lua_pushinteger(L, 1);
                lua_gettable(L, 3 + i);
                vertex[i].x = (float)lua_tonumber(L, -1);

                lua_pushinteger(L, 2);
                lua_gettable(L, 3 + i);
                vertex[i].y = (float)lua_tonumber(L, -1);
                
                lua_pushinteger(L, 3);
                lua_gettable(L, 3 + i);
                vertex[i].z = (float)lua_tonumber(L, -1);

                lua_pushinteger(L, 4);
                lua_gettable(L, 3 + i);
                vertex[i].u = (float)lua_tonumber(L, -1);

                lua_pushinteger(L, 5);
                lua_gettable(L, 3 + i);
                vertex[i].v = (float)lua_tonumber(L, -1);

                lua_pushinteger(L, 6);
                lua_gettable(L, 3 + i);
                vertex[i].color = static_cast<fcyColor*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_COLOR))->argb;

                lua_pop(L, 6);
            }

            if (!LAPP.RenderTexture(tex_name, blend, vertex))
                return luaL_error(L, "can't render texture '%s'.", tex_name);
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
        static int SetFog(lua_State* L)LNOEXCEPT
        {
            if (lua_gettop(L) == 3)
                LAPP.SetFog(
                    static_cast<float>(luaL_checknumber(L, 1)),
                    static_cast<float>(luaL_checknumber(L, 2)),
                    *(static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)))
                );
            else if (lua_gettop(L) == 2)
                LAPP.SetFog(
                    static_cast<float>(luaL_checknumber(L, 1)),
                    static_cast<float>(luaL_checknumber(L, 2)),
                    0xFF000000
                );
            else
                LAPP.SetFog(0.0f, 0.0f, 0x00FFFFFF);
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
        static int PostEffect(lua_State* L)LNOEXCEPT
        {
            const char* texture = luaL_checkstring(L, 1);
            const char* name = luaL_checkstring(L, 2);
            BlendMode blend = TranslateBlendMode(L, 3);

            // 获取纹理
            ResTexture* rt = LRES.FindTexture(luaL_checkstring(L, 1));
            if (!rt)
                return luaL_error(L, "texture '%s' not found.", texture);

            // 获取fx
            ResFX* p = LRES.FindFX(name);
            if (!p)
                return luaL_error(L, "PostEffect: can't find effect '%s'.", name);
            if (lua_istable(L, 4))
            {
                // 设置table上的参数到fx
                lua_pushnil(L);  // s s t ... nil
                while (0 != lua_next(L, 4))
                {
                    // s s t ... nil key value
                    const char* key = luaL_checkstring(L, -2);
                    if (lua_isnumber(L, -1))
                        p->SetValue(key, (float)lua_tonumber(L, -1));
                    else if (lua_isstring(L, -1))
                    {
                        ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
                        if (!pTex)
                            return luaL_error(L, "PostEffect: can't find texture '%s'.", lua_tostring(L, -1));
                        p->SetValue(key, pTex->GetTexture());
                    }
                    else if (lua_isuserdata(L, -1))
                    {
                        fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_COLOR));
                        p->SetValue(key, c);
                    }
                    else
                        return luaL_error(L, "PostEffect: invalid data type.");

                    lua_pop(L, 1);  // s s t ... nil key
                }
            }

            if (!LAPP.PostEffect(rt, p, blend))
                return luaL_error(L, "PostEffect failed.");
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
#ifndef LUASTG_GRAPHIC_API_D3D11
        { "BeginScene", &Wrapper::BeginScene },
        { "EndScene", &Wrapper::EndScene },
        { "RenderClear", &Wrapper::RenderClear },
        { "SetViewport", &Wrapper::SetViewport },
        { "SetScissorRect", &Wrapper::SetScissorRect },
        { "SetOrtho", &Wrapper::SetOrtho },
        { "SetPerspective", &Wrapper::SetPerspective },
        { "SetTextureSamplerState", &Wrapper::SetTextureSamplerState },
        { "Render", &Wrapper::Render },
        { "RenderAnimation", &Wrapper::RenderAnimation },
        { "RenderRect", &Wrapper::RenderRect },
        { "Render4V", &Wrapper::Render4V },
        { "RenderTexture", &Wrapper::RenderTexture },
        { "SetFog", &Wrapper::SetFog },
        { "PushRenderTarget", &Wrapper::PushRenderTarget },
        { "PopRenderTarget", &Wrapper::PopRenderTarget },
        { "PostEffect", &Wrapper::PostEffect },
        //EX+
        { "SetZBufferEnable", &Wrapper::SetZBufferEnable },
        { "ClearZBuffer", &Wrapper::ClearZBuffer },
        { "RenderModel", &Wrapper::RenderModel },
        { "DrawCollider", &Wrapper::DrawCollider },
        //ETC
        { "RenderGroupCollider", &Wrapper::RenderGroupCollider },
        { "RenderTextureSector", &Wrapper::RenderTextureSector },
        { "RenderTextureAnnulus", &Wrapper::RenderTextureAnnulus },
#endif
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
