#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "AppFrame.h"

inline Core::Graphics::IRenderer* LR2D() { return LAPP.GetAppModel()->getRenderer(); }
inline LuaSTGPlus::ResourceMgr& LRESMGR() { return LAPP.GetResourceMgr(); }

#ifdef _DEBUG
#define check_rendertarget_usage(PTEXTURE) assert(!LuaSTGPlus::AppFrame::GetInstance().CheckRenderTargetInUse(PTEXTURE));
#else
#define check_rendertarget_usage(PTEXTURE)
#endif // _DEBUG

inline void rotate_float2(float& x, float& y, const float r)
{
    float const sinv = std::sinf(r);
    float const cosv = std::cosf(r);
    float const tx = x * cosv - y * sinv;
    float const ty = x * sinv + y * cosv;
    x = tx;
    y = ty;
}
inline void rotate_float2x4(float& x1, float& y1, float& x2, float& y2, float& x3, float& y3, float& x4, float& y4, const float r)
{
    float const sinv = std::sinf(r);
    float const cosv = std::cosf(r);
    {
        float const tx = x1 * cosv - y1 * sinv;
        float const ty = x1 * sinv + y1 * cosv;
        x1 = tx;
        y1 = ty;
    }
    {
        float const tx = x2 * cosv - y2 * sinv;
        float const ty = x2 * sinv + y2 * cosv;
        x2 = tx;
        y2 = ty;
    }
    {
        float const tx = x3 * cosv - y3 * sinv;
        float const ty = x3 * sinv + y3 * cosv;
        x3 = tx;
        y3 = ty;
    }
    {
        float const tx = x4 * cosv - y4 * sinv;
        float const ty = x4 * sinv + y4 * cosv;
        x4 = tx;
        y4 = ty;
    }
}
inline void translate_blend(Core::Graphics::IRenderer*, const LuaSTGPlus::BlendMode blend)
{
    LAPP.updateGraph2DBlendMode(blend);
}
static Core::Graphics::IRenderer::BlendState translate_blend_3d(const LuaSTGPlus::BlendMode blend)
{
    switch (blend)
    {
    default:
    case LuaSTGPlus::BlendMode::MulAlpha:
        return Core::Graphics::IRenderer::BlendState::Alpha;
    case LuaSTGPlus::BlendMode::MulAdd:
        return Core::Graphics::IRenderer::BlendState::Add;
    case LuaSTGPlus::BlendMode::MulRev:
        return Core::Graphics::IRenderer::BlendState::RevSub;
    case LuaSTGPlus::BlendMode::MulSub:
        return Core::Graphics::IRenderer::BlendState::Sub;
    case LuaSTGPlus::BlendMode::AddAlpha:
        return Core::Graphics::IRenderer::BlendState::Alpha;
    case LuaSTGPlus::BlendMode::AddAdd:
        return Core::Graphics::IRenderer::BlendState::Add;
    case LuaSTGPlus::BlendMode::AddRev:
        return Core::Graphics::IRenderer::BlendState::RevSub;
    case LuaSTGPlus::BlendMode::AddSub:
        return Core::Graphics::IRenderer::BlendState::Sub;
    case LuaSTGPlus::BlendMode::AlphaBal:
        return Core::Graphics::IRenderer::BlendState::Inv;
    case LuaSTGPlus::BlendMode::MulMin:
        return Core::Graphics::IRenderer::BlendState::Min;
    case LuaSTGPlus::BlendMode::MulMax:
        return Core::Graphics::IRenderer::BlendState::Max;
    case LuaSTGPlus::BlendMode::MulMutiply:
        return Core::Graphics::IRenderer::BlendState::Mul;
    case LuaSTGPlus::BlendMode::MulScreen:
        return Core::Graphics::IRenderer::BlendState::Screen;
    case LuaSTGPlus::BlendMode::AddMin:
        return Core::Graphics::IRenderer::BlendState::Min;
    case LuaSTGPlus::BlendMode::AddMax:
        return Core::Graphics::IRenderer::BlendState::Max;
    case LuaSTGPlus::BlendMode::AddMutiply:
        return Core::Graphics::IRenderer::BlendState::Mul;
    case LuaSTGPlus::BlendMode::AddScreen:
        return Core::Graphics::IRenderer::BlendState::Screen;
    case LuaSTGPlus::BlendMode::One:
        return Core::Graphics::IRenderer::BlendState::One;
    }
}

static void api_drawSprite(LuaSTGPlus::ResSprite* pimg2dres, float const x, float const y, float const rot, float const hscale, float const vscale, float const z)
{
    Core::Graphics::ISprite* p_sprite = pimg2dres->GetSprite();
    auto* ctx = LR2D();
    translate_blend(ctx, pimg2dres->GetBlendMode());
    p_sprite->setZ(z);
    p_sprite->draw(Core::Vector2F(x, y), Core::Vector2F(hscale, vscale), rot);
}
static void api_drawSprite(char const* name, float const x, float const y, float const rot, float const hscale, float const vscale, float const z)
{
    fcyRefPointer<LuaSTGPlus::ResSprite> pimg2dres = LRESMGR().FindSprite(name);
    if (!pimg2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawSprite failed, can't find sprite '{}'", name);
        return;
    }
    api_drawSprite(*pimg2dres, x, y, rot, hscale, vscale, z);
}
static void api_drawSpriteRect(LuaSTGPlus::ResSprite* pimg2dres, float const l, float const r, float const b, float const t, float const z)
{
    Core::Graphics::ISprite* p_sprite = pimg2dres->GetSprite();
    auto* ctx = LR2D();
    translate_blend(ctx, pimg2dres->GetBlendMode());
    p_sprite->setZ(z);
    p_sprite->draw(Core::RectF(l, t, r, b));
}
static void api_drawSpriteRect(char const* name, float const l, float const r, float const b, float const t, float const z)
{
    fcyRefPointer<LuaSTGPlus::ResSprite> pimg2dres = LRESMGR().FindSprite(name);
    if (!pimg2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawSpriteRect failed, can't find sprite '{}'", name);
        return;
    }
    api_drawSpriteRect(*pimg2dres, l, r, b, t, z);
}
static void api_drawSprite4V(LuaSTGPlus::ResSprite* pimg2dres, float const x1, float const y1, float const z1, float const x2, float const y2, float const z2, float const x3, float const y3, float const z3, float const x4, float const y4, float const z4)
{
    Core::Graphics::ISprite* p_sprite = pimg2dres->GetSprite();
    auto* ctx = LR2D();
    translate_blend(ctx, pimg2dres->GetBlendMode());
    p_sprite->draw(
        Core::Vector3F(x1, y1, z1),
        Core::Vector3F(x2, y2, z2),
        Core::Vector3F(x3, y3, z3),
        Core::Vector3F(x4, y4, z4)
    );
}
static void api_drawSprite4V(char const* name, float const x1, float const y1, float const z1, float const x2, float const y2, float const z2, float const x3, float const y3, float const z3, float const x4, float const y4, float const z4)
{
    fcyRefPointer<LuaSTGPlus::ResSprite> pimg2dres = LRESMGR().FindSprite(name);
    if (!pimg2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawSprite4V failed, can't find sprite '{}'", name);
        return;
    }
    api_drawSprite4V(*pimg2dres, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
}

static void api_drawSpriteSequence(LuaSTGPlus::ResAnimation* pani2dres, int const ani_timer, float const x, float const y, float const rot, float const hscale, float const vscale, float const z)
{
    Core::Graphics::ISprite* p_sprite = pani2dres->GetSpriteByTimer(ani_timer);
    auto* ctx = LR2D();
    translate_blend(ctx, pani2dres->GetBlendMode());
    p_sprite->setZ(z);
    p_sprite->draw(Core::Vector2F(x, y), Core::Vector2F(hscale, vscale), rot);
}
static void api_drawSpriteSequence(char const* name, int const ani_timer, float const x, float const y, float const rot, float const hscale, float const vscale, float const z)
{
    fcyRefPointer<LuaSTGPlus::ResAnimation> pani2dres = LRESMGR().FindAnimation(name);
    if (!pani2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawSpriteSequence failed, can't find sprite sequence '{}'", name);
        return;
    }
    api_drawSpriteSequence(*pani2dres, ani_timer, x, y, rot, hscale, vscale, z);
}

static void api_setFogState(float start, float end, Core::Color4B color)
{
    auto* ctx = LR2D();
    if (start != end)
    {
        if (start == -1.0f)
        {
            ctx->setFogState(Core::Graphics::IRenderer::FogState::Exp, color, end, 0.0f);
        }
        else if (start == -2.0f)
        {
            ctx->setFogState(Core::Graphics::IRenderer::FogState::Exp2, color, end, 0.0f);
        }
        else
        {
            ctx->setFogState(Core::Graphics::IRenderer::FogState::Linear, color, start, end);
        }
    }
    else
    {
        ctx->setFogState(Core::Graphics::IRenderer::FogState::Disable, Core::Color4B(), 0.0f, 0.0f);
    }
}

static int lib_beginScene(lua_State* L)noexcept
{
    if (!LR2D()->beginBatch())
        return luaL_error(L, "[luastg] lstg.Renderer.BeginScene failed");
    return 0;
}
static int lib_endScene(lua_State* L)noexcept
{
    if (!LR2D()->endBatch())
        return luaL_error(L, "[luastg] lstg.Renderer.endScene failed");
    return 0;
}

static int lib_clearRenderTarget(lua_State* L)noexcept
{
    Core::Color4B color;
    if (lua_isnumber(L, 1))
    {
        color = Core::Color4B((uint32_t)lua_tonumber(L, 1));
    }
    else
    {
        color = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, 1);
    }
    LR2D()->clearRenderTarget(color);
    return 0;
}
static int lib_clearDepthBuffer(lua_State* L)noexcept
{
    LR2D()->clearDepthBuffer((float)luaL_checknumber(L, 1));
    return 0;
}

static int lib_setOrtho(lua_State* L)noexcept
{
    Core::BoxF box;
    if (lua_gettop(L) < 6)
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 4),
            0.0f,
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            1.0f
        );
    }
    else
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 6)
        );
    }
    LR2D()->setOrtho(box);
    return 0;
}
static int lib_setPerspective(lua_State* L)noexcept
{
    Core::Vector3F eye;
    eye.x = (float)luaL_checknumber(L, 1);
    eye.y = (float)luaL_checknumber(L, 2);
    eye.z = (float)luaL_checknumber(L, 3);
    Core::Vector3F lookat;
    lookat.x = (float)luaL_checknumber(L, 4);
    lookat.y = (float)luaL_checknumber(L, 5);
    lookat.z = (float)luaL_checknumber(L, 6);
    Core::Vector3F headup;
    headup.x = (float)luaL_checknumber(L, 7);
    headup.y = (float)luaL_checknumber(L, 8);
    headup.z = (float)luaL_checknumber(L, 9);
    Core::Vector2F zrange;
    zrange.x = (float)luaL_checknumber(L, 12);
    zrange.y = (float)luaL_checknumber(L, 13);
    if (zrange.x <= 0.0f || zrange.y <= zrange.x)
        return luaL_error(L, "invalid parameters, require (0 < znear < far), receive (znear = %f, zfar = %f)", zrange.x, zrange.y);
    LR2D()->setPerspective(eye, lookat, headup,
        (float)luaL_checknumber(L, 10),
        (float)luaL_checknumber(L, 11),
        zrange.x,
        zrange.y);
    return 0;
}

static int lib_setViewport(lua_State* L)noexcept
{
    Core::BoxF box;
    if (lua_gettop(L) < 6)
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            0.0f,
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            1.0f
        );
    }
    else
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 5),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 6)
        );
    }
    LR2D()->setViewport(box);
    return 0;
}
static int lib_setScissorRect(lua_State* L)noexcept
{
    LR2D()->setScissorRect(Core::RectF(
        (float)luaL_checknumber(L, 1),
        (float)luaL_checknumber(L, 2),
        (float)luaL_checknumber(L, 3),
        (float)luaL_checknumber(L, 4)
    ));
    return 0;
}

static int lib_setVertexColorBlendState(lua_State* L)noexcept
{
    LR2D()->setVertexColorBlendState((Core::Graphics::IRenderer::VertexColorBlendState)luaL_checkinteger(L, 1));
    return 0;
}
static int lib_setFogState(lua_State* L)noexcept
{
    Core::Color4B color;
    if (lua_isnumber(L, 2))
    {
        color = Core::Color4B((uint32_t)lua_tonumber(L, 2));
    }
    else
    {
        color = *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, 2);
    }
    LR2D()->setFogState(
        (Core::Graphics::IRenderer::FogState)luaL_checkinteger(L, 1),
        color,
        (float)luaL_checknumber(L, 3),
        (float)luaL_optnumber(L, 4, 0.0));
    return 0;
}
static int lib_setDepthState(lua_State* L)noexcept
{
    LR2D()->setDepthState((Core::Graphics::IRenderer::DepthState)luaL_checkinteger(L, 1));
    return 0;
}
static int lib_setBlendState(lua_State* L)noexcept
{
    LR2D()->setBlendState((Core::Graphics::IRenderer::BlendState)luaL_checkinteger(L, 1));
    return 0;
}
static int lib_setTexture(lua_State* L)noexcept
{
    char const* name = luaL_checkstring(L, 1);
    fcyRefPointer<LuaSTGPlus::ResTexture> p = LRESMGR().FindTexture(name);
    if (!p)
    {
        spdlog::error("[luastg] lstg.Renderer.setTexture failed: can't find texture '{}'", name);
        return false;
    }
    check_rendertarget_usage(*p);
    LR2D()->setTexture(p->GetTexture());
    return 0;
}

static int lib_drawTriangle(lua_State* L)
{
    Core::Graphics::IRenderer::DrawVertex vertex[3];

    lua_rawgeti(L, 1, 1);
    lua_rawgeti(L, 1, 2);
    lua_rawgeti(L, 1, 3);
    lua_rawgeti(L, 1, 4);
    lua_rawgeti(L, 1, 5);
    lua_rawgeti(L, 1, 6);
    vertex[0].x = (float)luaL_checknumber(L, 4);
    vertex[0].y = (float)luaL_checknumber(L, 5);
    vertex[0].z = (float)luaL_checknumber(L, 6);
    vertex[0].u = (float)luaL_checknumber(L, 7);
    vertex[0].v = (float)luaL_checknumber(L, 8);
    vertex[0].color = (uint32_t)luaL_checknumber(L, 9);
    lua_pop(L, 6);

    lua_rawgeti(L, 2, 1);
    lua_rawgeti(L, 2, 2);
    lua_rawgeti(L, 2, 3);
    lua_rawgeti(L, 2, 4);
    lua_rawgeti(L, 2, 5);
    lua_rawgeti(L, 2, 6);
    vertex[1].x = (float)luaL_checknumber(L, 4);
    vertex[1].y = (float)luaL_checknumber(L, 5);
    vertex[1].z = (float)luaL_checknumber(L, 6);
    vertex[1].u = (float)luaL_checknumber(L, 7);
    vertex[1].v = (float)luaL_checknumber(L, 8);
    vertex[1].color = (uint32_t)luaL_checknumber(L, 9);
    lua_pop(L, 6);

    lua_rawgeti(L, 3, 1);
    lua_rawgeti(L, 3, 2);
    lua_rawgeti(L, 3, 3);
    lua_rawgeti(L, 3, 4);
    lua_rawgeti(L, 3, 5);
    lua_rawgeti(L, 3, 6);
    vertex[2].x = (float)luaL_checknumber(L, 4);
    vertex[2].y = (float)luaL_checknumber(L, 5);
    vertex[2].z = (float)luaL_checknumber(L, 6);
    vertex[2].u = (float)luaL_checknumber(L, 7);
    vertex[2].v = (float)luaL_checknumber(L, 8);
    vertex[2].color = (uint32_t)luaL_checknumber(L, 9);
    lua_pop(L, 6);

    LR2D()->drawTriangle(vertex[0], vertex[1], vertex[2]);
    return 0;
}
static int lib_drawQuad(lua_State* L)
{
    Core::Graphics::IRenderer::DrawVertex vertex[4];

    lua_rawgeti(L, 1, 1);
    lua_rawgeti(L, 1, 2);
    lua_rawgeti(L, 1, 3);
    lua_rawgeti(L, 1, 4);
    lua_rawgeti(L, 1, 5);
    lua_rawgeti(L, 1, 6);
    vertex[0].x = (float)luaL_checknumber(L, 5);
    vertex[0].y = (float)luaL_checknumber(L, 6);
    vertex[0].z = (float)luaL_checknumber(L, 7);
    vertex[0].u = (float)luaL_checknumber(L, 8);
    vertex[0].v = (float)luaL_checknumber(L, 9);
    vertex[0].color = (uint32_t)luaL_checknumber(L, 10);
    lua_pop(L, 6);

    lua_rawgeti(L, 2, 1);
    lua_rawgeti(L, 2, 2);
    lua_rawgeti(L, 2, 3);
    lua_rawgeti(L, 2, 4);
    lua_rawgeti(L, 2, 5);
    lua_rawgeti(L, 2, 6);
    vertex[1].x = (float)luaL_checknumber(L, 5);
    vertex[1].y = (float)luaL_checknumber(L, 6);
    vertex[1].z = (float)luaL_checknumber(L, 7);
    vertex[1].u = (float)luaL_checknumber(L, 8);
    vertex[1].v = (float)luaL_checknumber(L, 9);
    vertex[1].color = (uint32_t)luaL_checknumber(L, 10);
    lua_pop(L, 6);

    lua_rawgeti(L, 3, 1);
    lua_rawgeti(L, 3, 2);
    lua_rawgeti(L, 3, 3);
    lua_rawgeti(L, 3, 4);
    lua_rawgeti(L, 3, 5);
    lua_rawgeti(L, 3, 6);
    vertex[2].x = (float)luaL_checknumber(L, 5);
    vertex[2].y = (float)luaL_checknumber(L, 6);
    vertex[2].z = (float)luaL_checknumber(L, 7);
    vertex[2].u = (float)luaL_checknumber(L, 8);
    vertex[2].v = (float)luaL_checknumber(L, 9);
    vertex[2].color = (uint32_t)luaL_checknumber(L, 10);
    lua_pop(L, 6);

    lua_rawgeti(L, 4, 1);
    lua_rawgeti(L, 4, 2);
    lua_rawgeti(L, 4, 3);
    lua_rawgeti(L, 4, 4);
    lua_rawgeti(L, 4, 5);
    lua_rawgeti(L, 4, 6);
    vertex[3].x = (float)luaL_checknumber(L, 5);
    vertex[3].y = (float)luaL_checknumber(L, 6);
    vertex[3].z = (float)luaL_checknumber(L, 7);
    vertex[3].u = (float)luaL_checknumber(L, 8);
    vertex[3].v = (float)luaL_checknumber(L, 9);
    vertex[3].color = (uint32_t)luaL_checknumber(L, 10);
    lua_pop(L, 6);

    LR2D()->drawQuad(vertex[0], vertex[1], vertex[2], vertex[3]);
    return 0;
}

static int lib_drawSprite(lua_State* L)
{
    float const hscale = (float)luaL_optnumber(L, 5, 1.0);
    api_drawSprite(
        luaL_checkstring(L, 1),
        (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3),
        (float)(luaL_optnumber(L, 4, 0.0) * L_DEG_TO_RAD),
        hscale * LRESMGR().GetGlobalImageScaleFactor(), (float)luaL_optnumber(L, 6, hscale) * LRESMGR().GetGlobalImageScaleFactor(),
        (float)luaL_optnumber(L, 7, 0.5));
    return 0;
}
static int lib_drawSpriteRect(lua_State* L)
{
    api_drawSpriteRect(
        luaL_checkstring(L, 1),
        (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3),
        (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5),
        (float)luaL_optnumber(L, 6, 0.5));
    return 0;
}
static int lib_drawSprite4V(lua_State* L)
{
    api_drawSprite4V(
        luaL_checkstring(L, 1),
        (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4),
        (float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), (float)luaL_checknumber(L, 7),
        (float)luaL_checknumber(L, 8), (float)luaL_checknumber(L, 9), (float)luaL_checknumber(L, 10),
        (float)luaL_checknumber(L, 11), (float)luaL_checknumber(L, 12), (float)luaL_checknumber(L, 13));
    return 0;
}

static int lib_drawSpriteSequence(lua_State* L)
{
    float const hscale = (float)luaL_optnumber(L, 6, 1.0);
    api_drawSpriteSequence(
        luaL_checkstring(L, 1),
        (int)luaL_checkinteger(L, 2),
        (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4),
        (float)(luaL_optnumber(L, 5, 0.0) * L_DEG_TO_RAD),
        hscale * LRESMGR().GetGlobalImageScaleFactor(), (float)luaL_optnumber(L, 7, hscale) * LRESMGR().GetGlobalImageScaleFactor(),
        (float)luaL_optnumber(L, 8, 0.5));
    return 0;
}

static int lib_drawTexture(lua_State* L) noexcept
{
    const char* name = luaL_checkstring(L, 1);
    LuaSTGPlus::BlendMode blend = LuaSTGPlus::TranslateBlendMode(L, 2);
    Core::Graphics::IRenderer::DrawVertex vertex[4];

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
        if (lua_isnumber(L, -1))
        {
            vertex[i].color = (uint32_t)lua_tonumber(L, -1);
        }
        else
        {
            vertex[i].color = LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, -1)->color();
        }

        lua_pop(L, 6);
    }

    auto* ctx = LR2D();

    translate_blend(ctx, blend);

    fcyRefPointer<LuaSTGPlus::ResTexture> ptex2dres = LRESMGR().FindTexture(name);
    if (!ptex2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawTexture failed: can't find texture '{}'", name);
        return luaL_error(L, "can't find texture '%s'", name);
    }
    check_rendertarget_usage(*ptex2dres);
    Core::Graphics::ITexture2D* ptex2d = ptex2dres->GetTexture();
    float const uscale = 1.0f / (float)ptex2d->getSize().x;
    float const vscale = 1.0f / (float)ptex2d->getSize().y;
    for (int i = 0; i < 4; ++i)
    {
        vertex[i].u *= uscale;
        vertex[i].v *= vscale;
    }
    ctx->setTexture(ptex2d);

    ctx->drawQuad(vertex[0], vertex[1], vertex[2], vertex[3]);

    return 0;
}
static int lib_drawMesh(lua_State* L) noexcept
{
    std::string_view const tex_name = luaL_check_string_view(L, 1);
    LuaSTGPlus::BlendMode blend = LuaSTGPlus::TranslateBlendMode(L, 2);
    LuaSTGPlus::Mesh* mesh = LuaSTGPlus::LuaWrapper::MeshBinding::Cast(L, 3);

    auto* ctx = LR2D();

    translate_blend(ctx, blend);

    fcyRefPointer<LuaSTGPlus::ResTexture> ptex2dres = LRESMGR().FindTexture(tex_name.data());
    if (!ptex2dres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawMesh failed: can't find texture '{}'", tex_name);
        return luaL_error(L, "can't find texture '%s'", tex_name.data());
    }
    check_rendertarget_usage(*ptex2dres);
    ctx->setTexture(ptex2dres->GetTexture());

    mesh->draw(ctx);

    return 0;
}

static int lib_drawModel(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);

    float const x = (float)luaL_checknumber(L, 2);
    float const y = (float)luaL_checknumber(L, 3);
    float const z = (float)luaL_checknumber(L, 4);

    float const roll  = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 5, 0.0));
    float const pitch = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 6, 0.0));
    float const yaw   = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 7, 0.0));

    float const sx = (float)luaL_optnumber(L, 8, 1.0);
    float const sy = (float)luaL_optnumber(L, 9, 1.0);
    float const sz = (float)luaL_optnumber(L, 10, 1.0);

    fcyRefPointer<LuaSTGPlus::ResModel> pmodres = LRESMGR().FindModel(name);
    if (!pmodres)
    {
        spdlog::error("[luastg] lstg.Renderer.drawModel failed: can't find model '{}'", name);
        return false;
    }

    pmodres->GetModel()->setScaling(Core::Vector3F(sx, sy, sz));
    pmodres->GetModel()->setRotationRollPitchYaw(roll, pitch, yaw);
    pmodres->GetModel()->setPosition(Core::Vector3F(x, y, z));
    LR2D()->drawModel(pmodres->GetModel());

    return 0;
}

#define MKFUNC(X) {#X, &lib_##X}

static luaL_Reg const lib_func[] = {
    MKFUNC(beginScene),
    MKFUNC(endScene),

    MKFUNC(clearRenderTarget),
    MKFUNC(clearDepthBuffer),

    MKFUNC(setOrtho),
    MKFUNC(setPerspective),

    MKFUNC(setViewport),
    MKFUNC(setScissorRect),

    MKFUNC(setVertexColorBlendState),
    MKFUNC(setFogState),
    MKFUNC(setDepthState),
    MKFUNC(setBlendState),
    MKFUNC(setTexture),

    MKFUNC(drawTriangle),
    MKFUNC(drawQuad),

    MKFUNC(drawSprite),
    MKFUNC(drawSpriteRect),
    MKFUNC(drawSprite4V),

    MKFUNC(drawSpriteSequence),

    MKFUNC(drawTexture),

    { NULL, NULL },
};

static int compat_SetViewport(lua_State* L)noexcept
{
    Core::BoxF box;
    if (lua_gettop(L) >= 6)
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 6)
        );
    }
    else
    {
        box = Core::BoxF(
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 4),
            0.0f,
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            1.0f
        );
    }
    Core::Vector2U const backbuf_size = LAPP.GetCurrentRenderTargetSize();
    box.a.y = (float)backbuf_size.y - box.a.y;
    box.b.y = (float)backbuf_size.y - box.b.y;
    LR2D()->setViewport(box);
    return 0;
}
static int compat_SetScissorRect(lua_State* L)noexcept
{
    Core::RectF rect(
        (float)luaL_checknumber(L, 1),
        (float)luaL_checknumber(L, 4),
        (float)luaL_checknumber(L, 2),
        (float)luaL_checknumber(L, 3)
    );
    Core::Vector2U const backbuf_size = LAPP.GetCurrentRenderTargetSize();
    rect.a.y = (float)backbuf_size.y - rect.a.y;
    rect.b.y = (float)backbuf_size.y - rect.b.y;
    LR2D()->setScissorRect(rect);
    return 0;
}
static int compat_SetFog(lua_State* L)noexcept
{
    int const argc = lua_gettop(L);
    if (argc >= 3)
    {
        api_setFogState(
            static_cast<float>(luaL_checknumber(L, 1)),
            static_cast<float>(luaL_checknumber(L, 2)),
            *LuaSTGPlus::LuaWrapper::ColorWrapper::Cast(L, 3)
        );
    }
    else if (argc == 2)
    {
        api_setFogState(
            static_cast<float>(luaL_checknumber(L, 1)),
            static_cast<float>(luaL_checknumber(L, 2)),
            0xFF000000
        );
    }
    else
    {
        api_setFogState(0.0f, 0.0f, 0x00000000);
    }
    return 0;
}
static int compat_SetZBufferEnable(lua_State* L)noexcept
{
    LR2D()->setDepthState((Core::Graphics::IRenderer::DepthState)luaL_checkinteger(L, 1));
    return 0;
}
static int compat_ClearZBuffer(lua_State* L)noexcept
{
    LR2D()->clearDepthBuffer((float)luaL_optnumber(L, 1, 1.0));
    return 0;
}
static int compat_PushRenderTarget(lua_State* L)noexcept
{
    LR2D()->flush();
    LuaSTGPlus::ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
    if (!p)
        return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
    if (!p->IsRenderTarget())
        return luaL_error(L, "'%s' is a texture.", luaL_checkstring(L, 1));

    if (!LAPP.PushRenderTarget(p))
        return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
    LR2D()->setViewportAndScissorRect();
    return 0;
}
static int compat_PopRenderTarget(lua_State* L)noexcept
{
    LR2D()->flush();
    if (!LAPP.PopRenderTarget())
        return luaL_error(L, "pop rendertarget failed.");
    LR2D()->setViewportAndScissorRect();
    return 0;
}
static int compat_PostEffect(lua_State* L)
{
    const char* ps_name = luaL_checkstring(L, 1);
    const char* rt_name = luaL_checkstring(L, 2);
    const Core::Graphics::IRenderer::SamplerState rtsv = (Core::Graphics::IRenderer::SamplerState)luaL_checkinteger(L, 3);
    const Core::Graphics::IRenderer::BlendState blend = translate_blend_3d(LuaSTGPlus::TranslateBlendMode(L, 4));

    LuaSTGPlus::ResFX* pfx = LRES.FindFX(ps_name);
    if (!pfx)
        return luaL_error(L, "effect '%s' not found.", ps_name);
    
    LuaSTGPlus::ResTexture* prt = LRES.FindTexture(rt_name);
    if (!prt)
        return luaL_error(L, "texture '%s' not found.", rt_name);
    check_rendertarget_usage(prt);
    
    Core::Vector4F cbdata[8] = {};
    Core::Graphics::ITexture2D* tdata[4] = {};
    Core::Graphics::IRenderer::SamplerState tsdata[4] = {};
    
    size_t cbdata_n = lua_objlen(L, 5);
    cbdata_n = (cbdata_n <= 8) ? cbdata_n : 8;
    for (int i = 1; i <= (int)cbdata_n; i += 1)
    {
        lua_rawgeti(L, 5, i);  // ??? t
        lua_rawgeti(L, -1, 1); // ??? t f1
        lua_rawgeti(L, -2, 2); // ??? t f1 f2
        lua_rawgeti(L, -3, 3); // ??? t f1 f2 f3
        lua_rawgeti(L, -4, 4); // ??? t f1 f2 f3 f4
        cbdata[i - 1].x = (float)luaL_checknumber(L, -4);
        cbdata[i - 1].y = (float)luaL_checknumber(L, -3);
        cbdata[i - 1].z = (float)luaL_checknumber(L, -2);
        cbdata[i - 1].w = (float)luaL_checknumber(L, -1);
        lua_pop(L, 5);
    }
    size_t tdata_n = lua_objlen(L, 6);
    tdata_n = (tdata_n <= 8) ? tdata_n : 4;
    for (int i = 1; i <= (int)tdata_n; i += 1)
    {
        lua_rawgeti(L, 6, i);  // ??? t
        lua_rawgeti(L, -1, 1); // ??? t tex
        lua_rawgeti(L, -2, 2); // ??? t tex sampler
        const char* tx_name = luaL_checkstring(L, -2);
        LuaSTGPlus::ResTexture* ptex = LRES.FindTexture(tx_name);
        if (!ptex)
            return luaL_error(L, "texture '%s' not found.", tx_name);
        check_rendertarget_usage(ptex);
        tdata[i - 1] = ptex->GetTexture();
        tsdata[i - 1] = (Core::Graphics::IRenderer::SamplerState)luaL_checkinteger(L, -1);
    }

    LR2D()->drawPostEffect(pfx->GetPostEffectShader(), blend, prt->GetTexture(), rtsv, cbdata, cbdata_n, tdata, tsdata, tdata_n);

    return 0;
}

static luaL_Reg const lib_compat[] = {
    { "BeginScene", &lib_beginScene },
    { "EndScene", &lib_endScene },
    { "RenderClear", &lib_clearRenderTarget },
    { "SetViewport", &compat_SetViewport },
    { "SetScissorRect", &compat_SetScissorRect },
    { "SetOrtho", &lib_setOrtho },
    { "SetPerspective", &lib_setPerspective },
    { "Render", &lib_drawSprite },
    { "RenderRect", &lib_drawSpriteRect },
    { "Render4V", &lib_drawSprite4V },
    { "RenderAnimation", &lib_drawSpriteSequence },
    { "RenderTexture", &lib_drawTexture },
    { "RenderMesh", &lib_drawMesh },
    { "RenderModel", &lib_drawModel },
    { "SetFog", &compat_SetFog },
    { "SetZBufferEnable", &compat_SetZBufferEnable },
    { "ClearZBuffer", &compat_ClearZBuffer },
    { "PushRenderTarget", &compat_PushRenderTarget },
    { "PopRenderTarget", &compat_PopRenderTarget },
    { "PostEffect", &compat_PostEffect },
    { NULL, NULL },
};

void LuaSTGPlus::LuaWrapper::RendererWrapper::Register(lua_State* L)noexcept
{
    static luaL_Reg const lib_empty[] = {
        { NULL, NULL },
    };
    luaL_register(L, LUASTG_LUA_LIBNAME, lib_compat);           // ??? lstg
    luaL_register(L, LUASTG_LUA_LIBNAME ".Renderer", lib_func); // ??? lstg lstg.Renderer
    lua_setfield(L, -1, "Renderer");                            // ??? lstg
    lua_pop(L, 1);                                              // ???
}
