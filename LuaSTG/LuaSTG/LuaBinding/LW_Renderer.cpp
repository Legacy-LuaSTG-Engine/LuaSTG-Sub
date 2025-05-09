#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "LuaBinding/PostEffectShader.hpp"
#include "AppFrame.h"
#include "GameResource/LegacyBlendStateHelper.hpp"

namespace luastg {
	inline core::Graphics::IRenderer* LR2D() { return LAPP.GetAppModel()->getRenderer(); }
	inline ResourceMgr& LRESMGR() { return LAPP.GetResourceMgr(); }

#ifndef NDEBUG
#define check_rendertarget_usage(P_TEXTURE) assert(!LAPP.GetRenderTargetManager()->CheckRenderTargetInUse(P_TEXTURE.get()));
#else
#define check_rendertarget_usage(P_TEXTURE)
#endif

#define validate_render_scope() if (!LR2D()->isBatchScope()) return luaL_error(L, "invalid render operation");

	enum class RenderError {
		None,
		SpriteNotFound,
		SpriteSequenceNotFound,
	};

	inline void rotate_float2(float& x, float& y, const float r) {
		float const sinv = std::sinf(r);
		float const cosv = std::cosf(r);
		float const tx = x * cosv - y * sinv;
		float const ty = x * sinv + y * cosv;
		x = tx;
		y = ty;
	}
	inline void rotate_float2x4(float& x1, float& y1, float& x2, float& y2, float& x3, float& y3, float& x4, float& y4, const float r) {
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
	inline void translate_blend(core::Graphics::IRenderer*, const luastg::BlendMode blend) {
		LAPP.updateGraph2DBlendMode(blend);
	}
	inline core::Graphics::IRenderer::BlendState translate_blend_3d(const luastg::BlendMode blend) {
		[[maybe_unused]] auto const [v, b] = translateLegacyBlendState(blend);
		return b;
	}

	inline RenderError api_drawSprite(IResourceSprite* pimg2dres, float const x, float const y, float const rot, float const hscale, float const vscale, float const z) {
		pimg2dres->Render(x, y, rot, hscale, vscale, z);
		return RenderError::None;
	}
	inline RenderError api_drawSprite(char const* name, float const x, float const y, float const rot, float const hscale, float const vscale, float const z) {
		core::SmartReference<IResourceSprite> pimg2dres = LRESMGR().FindSprite(name);
		if (!pimg2dres) {
			spdlog::error("[luastg] lstg.Renderer.drawSprite failed, can't find sprite '{}'", name);
			return RenderError::SpriteNotFound;
		}
		return api_drawSprite(*pimg2dres, x, y, rot, hscale, vscale, z);
	}
	inline RenderError api_drawSpriteRect(IResourceSprite* pimg2dres, float const l, float const r, float const b, float const t, float const z) {
		pimg2dres->RenderRect(l, r, b, t, z);
		return RenderError::None;
	}
	inline RenderError api_drawSpriteRect(char const* name, float const l, float const r, float const b, float const t, float const z) {
		core::SmartReference<IResourceSprite> pimg2dres = LRESMGR().FindSprite(name);
		if (!pimg2dres) {
			spdlog::error("[luastg] lstg.Renderer.drawSpriteRect failed, can't find sprite '{}'", name);
			return RenderError::SpriteNotFound;
		}
		return api_drawSpriteRect(*pimg2dres, l, r, b, t, z);
	}
	inline RenderError api_drawSprite4V(IResourceSprite* pimg2dres, float const x1, float const y1, float const z1, float const x2, float const y2, float const z2, float const x3, float const y3, float const z3, float const x4, float const y4, float const z4) {
		pimg2dres->Render4V(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
		return RenderError::None;
	}
	inline RenderError api_drawSprite4V(char const* name, float const x1, float const y1, float const z1, float const x2, float const y2, float const z2, float const x3, float const y3, float const z3, float const x4, float const y4, float const z4) {
		core::SmartReference<IResourceSprite> pimg2dres = LRESMGR().FindSprite(name);
		if (!pimg2dres) {
			spdlog::error("[luastg] lstg.Renderer.drawSprite4V failed, can't find sprite '{}'", name);
			return RenderError::SpriteNotFound;
		}
		return api_drawSprite4V(*pimg2dres, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
	}

	inline RenderError api_drawSpriteSequence(IResourceAnimation* pani2dres, int const ani_timer, float const x, float const y, float const rot, float const hscale, float const vscale, float const z) {
		pani2dres->Render(ani_timer, x, y, rot, hscale, vscale, z);
		return RenderError::None;
	}
	inline RenderError api_drawSpriteSequence(char const* name, int const ani_timer, float const x, float const y, float const rot, float const hscale, float const vscale, float const z) {
		core::SmartReference<IResourceAnimation> pani2dres = LRESMGR().FindAnimation(name);
		if (!pani2dres) {
			spdlog::error("[luastg] lstg.Renderer.drawSpriteSequence failed, can't find sprite sequence '{}'", name);
			return RenderError::SpriteSequenceNotFound;
		}
		return api_drawSpriteSequence(*pani2dres, ani_timer, x, y, rot, hscale, vscale, z);
	}

	static void api_setFogState(float start, float end, core::Color4B color) {
		auto* ctx = LR2D();
		if (start != end) {
			if (start == -1.0f) {
				ctx->setFogState(core::Graphics::IRenderer::FogState::Exp, color, end, 0.0f);
			}
			else if (start == -2.0f) {
				ctx->setFogState(core::Graphics::IRenderer::FogState::Exp2, color, end, 0.0f);
			}
			else {
				ctx->setFogState(core::Graphics::IRenderer::FogState::Linear, color, start, end);
			}
		}
		else {
			ctx->setFogState(core::Graphics::IRenderer::FogState::Disable, core::Color4B(), 0.0f, 0.0f);
		}
	}

	static int lib_beginScene(lua_State* L)noexcept {
		if (!LR2D()->beginBatch())
			return luaL_error(L, "[luastg] lstg.Renderer.BeginScene failed");
		return 0;
	}
	static int lib_endScene(lua_State* L)noexcept {
		if (!LR2D()->endBatch())
			return luaL_error(L, "[luastg] lstg.Renderer.endScene failed");
		return 0;
	}

	static int lib_clearRenderTarget(lua_State* L)noexcept {
		core::Color4B color;
		if (lua_isnumber(L, 1)) {
			color = core::Color4B((uint32_t)lua_tonumber(L, 1));
		}
		else {
			color = *binding::Color::Cast(L, 1);
		}

		if (!LAPP.GetRenderTargetManager()->IsRenderTargetStackEmpty()) {
			uint16_t const r = color.a * color.r;
			color.r = static_cast<uint8_t>((r + ((r + 257) >> 8)) >> 8);
			uint16_t const g = color.a * color.g;
			color.g = static_cast<uint8_t>((g + ((g + 257) >> 8)) >> 8);
			uint16_t const b = color.a * color.b;
			color.b = static_cast<uint8_t>((b + ((b + 257) >> 8)) >> 8);
		}
		LR2D()->clearRenderTarget(color);
		return 0;
	}
	static int lib_clearDepthBuffer(lua_State* L)noexcept {
		LR2D()->clearDepthBuffer((float)luaL_checknumber(L, 1));
		return 0;
	}

	static int lib_setOrtho(lua_State* L)noexcept {
		core::BoxF box;
		if (lua_gettop(L) < 6) {
			box = core::BoxF(
				(float)luaL_checknumber(L, 1),
				(float)luaL_checknumber(L, 4),
				0.0f,
				(float)luaL_checknumber(L, 2),
				(float)luaL_checknumber(L, 3),
				1.0f
			);
		}
		else {
			box = core::BoxF(
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
	static int lib_setPerspective(lua_State* L)noexcept {
		core::Vector3F eye;
		eye.x = (float)luaL_checknumber(L, 1);
		eye.y = (float)luaL_checknumber(L, 2);
		eye.z = (float)luaL_checknumber(L, 3);
		core::Vector3F lookat;
		lookat.x = (float)luaL_checknumber(L, 4);
		lookat.y = (float)luaL_checknumber(L, 5);
		lookat.z = (float)luaL_checknumber(L, 6);
		core::Vector3F headup;
		headup.x = (float)luaL_checknumber(L, 7);
		headup.y = (float)luaL_checknumber(L, 8);
		headup.z = (float)luaL_checknumber(L, 9);
		core::Vector2F zrange;
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

	static int lib_setViewport(lua_State* L)noexcept {
		core::BoxF box;
		if (lua_gettop(L) < 6) {
			box = core::BoxF(
				(float)luaL_checknumber(L, 1),
				(float)luaL_checknumber(L, 2),
				0.0f,
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 4),
				1.0f
			);
		}
		else {
			box = core::BoxF(
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
	static int lib_setScissorRect(lua_State* L)noexcept {
		LR2D()->setScissorRect(core::RectF(
			(float)luaL_checknumber(L, 1),
			(float)luaL_checknumber(L, 2),
			(float)luaL_checknumber(L, 3),
			(float)luaL_checknumber(L, 4)
		));
		return 0;
	}

	static int lib_setVertexColorBlendState(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->setVertexColorBlendState((core::Graphics::IRenderer::VertexColorBlendState)luaL_checkinteger(L, 1));
		return 0;
	}
	static int lib_setFogState(lua_State* L)noexcept {
		validate_render_scope();
		core::Color4B color;
		if (lua_isnumber(L, 2)) {
			color = core::Color4B((uint32_t)lua_tonumber(L, 2));
		}
		else {
			color = *binding::Color::Cast(L, 2);
		}
		LR2D()->setFogState(
			(core::Graphics::IRenderer::FogState)luaL_checkinteger(L, 1),
			color,
			(float)luaL_checknumber(L, 3),
			(float)luaL_optnumber(L, 4, 0.0));
		return 0;
	}
	static int lib_setDepthState(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->setDepthState((core::Graphics::IRenderer::DepthState)luaL_checkinteger(L, 1));
		return 0;
	}
	static int lib_setBlendState(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->setBlendState((core::Graphics::IRenderer::BlendState)luaL_checkinteger(L, 1));
		return 0;
	}
	static int lib_setTexture(lua_State* L)noexcept {
		validate_render_scope();
		char const* name = luaL_checkstring(L, 1);
		core::SmartReference<IResourceTexture> p = LRESMGR().FindTexture(name);
		if (!p) {
			spdlog::error("[luastg] lstg.Renderer.setTexture failed: can't find texture '{}'", name);
			return luaL_error(L, "can't find texture '%s'", name);
		}
		check_rendertarget_usage(p);
		LR2D()->setTexture(p->GetTexture());
		return 0;
	}

	static int lib_drawTriangle(lua_State* L) {
		validate_render_scope();

		core::Graphics::IRenderer::DrawVertex vertex[3];

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
	static int lib_drawQuad(lua_State* L) {
		validate_render_scope();

		core::Graphics::IRenderer::DrawVertex vertex[4];

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

	static int lib_drawSprite(lua_State* L) {
		validate_render_scope();
		float const hscale = (float)luaL_optnumber(L, 5, 1.0);
		RenderError re = api_drawSprite(
			luaL_checkstring(L, 1),
			(float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3),
			(float)(luaL_optnumber(L, 4, 0.0) * L_DEG_TO_RAD),
			hscale * LRESMGR().GetGlobalImageScaleFactor(), (float)luaL_optnumber(L, 6, hscale) * LRESMGR().GetGlobalImageScaleFactor(),
			(float)luaL_optnumber(L, 7, 0.5));
		if (re == RenderError::SpriteNotFound) {
			return luaL_error(L, "can't find sprite '%s'", luaL_checkstring(L, 1));
		}
		return 0;
	}
	static int lib_drawSpriteRect(lua_State* L) {
		validate_render_scope();
		RenderError re = api_drawSpriteRect(
			luaL_checkstring(L, 1),
			(float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3),
			(float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5),
			(float)luaL_optnumber(L, 6, 0.5));
		if (re == RenderError::SpriteNotFound) {
			return luaL_error(L, "can't find sprite '%s'", luaL_checkstring(L, 1));
		}
		return 0;
	}
	static int lib_drawSprite4V(lua_State* L) {
		validate_render_scope();
		RenderError re = api_drawSprite4V(
			luaL_checkstring(L, 1),
			(float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4),
			(float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), (float)luaL_checknumber(L, 7),
			(float)luaL_checknumber(L, 8), (float)luaL_checknumber(L, 9), (float)luaL_checknumber(L, 10),
			(float)luaL_checknumber(L, 11), (float)luaL_checknumber(L, 12), (float)luaL_checknumber(L, 13));
		if (re == RenderError::SpriteNotFound) {
			return luaL_error(L, "can't find sprite '%s'", luaL_checkstring(L, 1));
		}
		return 0;
	}

	static int lib_drawSpriteSequence(lua_State* L) {
		validate_render_scope();
		float const hscale = (float)luaL_optnumber(L, 6, 1.0);
		RenderError re = api_drawSpriteSequence(
			luaL_checkstring(L, 1),
			(int)luaL_checkinteger(L, 2),
			(float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4),
			(float)(luaL_optnumber(L, 5, 0.0) * L_DEG_TO_RAD),
			hscale * LRESMGR().GetGlobalImageScaleFactor(), (float)luaL_optnumber(L, 7, hscale) * LRESMGR().GetGlobalImageScaleFactor(),
			(float)luaL_optnumber(L, 8, 0.5));
		if (re == RenderError::SpriteNotFound) {
			return luaL_error(L, "can't find animation '%s'", luaL_checkstring(L, 1));
		}
		return 0;
	}

	static int lib_drawTexture(lua_State* L) noexcept {
		validate_render_scope();

		const char* name = luaL_checkstring(L, 1);
		auto const blend = TranslateBlendMode(L, 2);
		core::Graphics::IRenderer::DrawVertex vertex[4];

		for (int i = 0; i < 4; ++i) {
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
			if (lua_isnumber(L, -1)) {
				vertex[i].color = (uint32_t)lua_tonumber(L, -1);
			}
			else {
				vertex[i].color = binding::Color::Cast(L, -1)->color();
			}

			lua_pop(L, 6);
		}

		auto* ctx = LR2D();

		translate_blend(ctx, blend);

		core::SmartReference<IResourceTexture> ptex2dres = LRESMGR().FindTexture(name);
		if (!ptex2dres) {
			spdlog::error("[luastg] lstg.Renderer.drawTexture failed: can't find texture '{}'", name);
			return luaL_error(L, "can't find texture '%s'", name);
		}
		check_rendertarget_usage(ptex2dres);
		core::Graphics::ITexture2D* ptex2d = ptex2dres->GetTexture();
		float const uscale = 1.0f / (float)ptex2d->getSize().x;
		float const vscale = 1.0f / (float)ptex2d->getSize().y;
		for (int i = 0; i < 4; ++i) {
			vertex[i].u *= uscale;
			vertex[i].v *= vscale;
		}
		ctx->setTexture(ptex2d);

		ctx->drawQuad(vertex[0], vertex[1], vertex[2], vertex[3]);

		return 0;
	}

	static int lib_drawModel(lua_State* L) {
		const char* name = luaL_checkstring(L, 1);

		float const x = (float)luaL_checknumber(L, 2);
		float const y = (float)luaL_checknumber(L, 3);
		float const z = (float)luaL_checknumber(L, 4);

		float const roll = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 5, 0.0));
		float const pitch = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 6, 0.0));
		float const yaw = (float)(L_DEG_TO_RAD * luaL_optnumber(L, 7, 0.0));

		float const sx = (float)luaL_optnumber(L, 8, 1.0);
		float const sy = (float)luaL_optnumber(L, 9, 1.0);
		float const sz = (float)luaL_optnumber(L, 10, 1.0);

		core::SmartReference<IResourceModel> pmodres = LRESMGR().FindModel(name);
		if (!pmodres) {
			spdlog::error("[luastg] lstg.Renderer.drawModel failed: can't find model '{}'", name);
			return false;
		}

		pmodres->GetModel()->setScaling(core::Vector3F(sx, sy, sz));
		pmodres->GetModel()->setRotationRollPitchYaw(roll, pitch, yaw);
		pmodres->GetModel()->setPosition(core::Vector3F(x, y, z));
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

	static int compat_SetViewport(lua_State* L)noexcept {
		core::BoxF box;
		if (lua_gettop(L) >= 6) {
			box = core::BoxF(
				(float)luaL_checknumber(L, 1),
				(float)luaL_checknumber(L, 4),
				(float)luaL_checknumber(L, 5),
				(float)luaL_checknumber(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 6)
			);
		}
		else {
			box = core::BoxF(
				(float)luaL_checknumber(L, 1),
				(float)luaL_checknumber(L, 4),
				0.0f,
				(float)luaL_checknumber(L, 2),
				(float)luaL_checknumber(L, 3),
				1.0f
			);
		}
		core::Vector2U const backbuf_size = LAPP.GetRenderTargetManager()->GetTopRenderTargetSize();
		box.a.y = (float)backbuf_size.y - box.a.y;
		box.b.y = (float)backbuf_size.y - box.b.y;
		LR2D()->setViewport(box);
		return 0;
	}
	static int compat_SetScissorRect(lua_State* L)noexcept {
		core::RectF rect(
			(float)luaL_checknumber(L, 1),
			(float)luaL_checknumber(L, 4),
			(float)luaL_checknumber(L, 2),
			(float)luaL_checknumber(L, 3)
		);
		core::Vector2U const backbuf_size = LAPP.GetRenderTargetManager()->GetTopRenderTargetSize();
		rect.a.y = (float)backbuf_size.y - rect.a.y;
		rect.b.y = (float)backbuf_size.y - rect.b.y;
		LR2D()->setScissorRect(rect);
		return 0;
	}
	static int compat_SetFog(lua_State* L)noexcept {
		int const argc = lua_gettop(L);
		if (argc >= 3) {
			api_setFogState(
				static_cast<float>(luaL_checknumber(L, 1)),
				static_cast<float>(luaL_checknumber(L, 2)),
				*binding::Color::Cast(L, 3)
			);
		}
		else if (argc == 2) {
			api_setFogState(
				static_cast<float>(luaL_checknumber(L, 1)),
				static_cast<float>(luaL_checknumber(L, 2)),
				core::Color4B(0xFF000000)
			);
		}
		else {
			api_setFogState(0.0f, 0.0f, core::Color4B(0x00000000));
		}
		return 0;
	}
	static int compat_SetZBufferEnable(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->setDepthState((core::Graphics::IRenderer::DepthState)luaL_checkinteger(L, 1));
		return 0;
	}
	static int compat_ClearZBuffer(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->clearDepthBuffer((float)luaL_optnumber(L, 1, 1.0));
		return 0;
	}
	static int compat_PushRenderTarget(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->flush();
		core::SmartReference<IResourceTexture> p = LRES.FindTexture(luaL_checkstring(L, 1));
		if (!p)
			return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
		if (!p->IsRenderTarget())
			return luaL_error(L, "'%s' is not a rendertarget.", luaL_checkstring(L, 1));

		if (!LAPP.GetRenderTargetManager()->PushRenderTarget(p.get()))
			return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
		LR2D()->setViewportAndScissorRect();
		return 0;
	}
	static int compat_PopRenderTarget(lua_State* L)noexcept {
		validate_render_scope();
		LR2D()->flush();
		if (!LAPP.GetRenderTargetManager()->PopRenderTarget())
			return luaL_error(L, "pop rendertarget failed.");
		LR2D()->setViewportAndScissorRect();
		return 0;
	}
	static int compat_PostEffect(lua_State* L) {
		validate_render_scope();

		// PostEffectShader 对象风格
		if (lua_isuserdata(L, 1)) {
			auto* p_effect = binding::PostEffectShader::Cast(L, 1);
			const core::Graphics::IRenderer::BlendState blend = translate_blend_3d(TranslateBlendMode(L, 2));
			LR2D()->drawPostEffect(p_effect, blend);
			return 0;
		}

		// 传统风格
		if (lua_type(L, 1) == LUA_TSTRING && lua_type(L, 2) == LUA_TSTRING && lua_type(L, 3) == LUA_TSTRING && lua_type(L, 3) != LUA_TNUMBER) {
			const char* rt_name = luaL_checkstring(L, 1);
			const char* ps_name = luaL_checkstring(L, 2);
			const core::Graphics::IRenderer::BlendState blend = translate_blend_3d(TranslateBlendMode(L, 3));

			core::SmartReference<IResourceTexture> prt = LRES.FindTexture(rt_name);
			if (!prt)
				return luaL_error(L, "texture '%s' not found.", rt_name);
			check_rendertarget_usage(prt);

			core::SmartReference<IResourcePostEffectShader> pfx = LRES.FindFX(ps_name);
			if (!pfx)
				return luaL_error(L, "posteffect '%s' not found.", ps_name);

			core::Graphics::IPostEffectShader* p_effect = pfx->GetPostEffectShader();

			p_effect->setTexture2D("screen_texture", prt->GetTexture());

			auto const rt_size = prt->GetTexture()->getSize();
			p_effect->setFloat4("screen_texture_size", core::Vector4F(float(rt_size.x), float(rt_size.y), 0.0f, 0.0f));

			auto const vp = LR2D()->getViewport();
			p_effect->setFloat4("viewport", core::Vector4F(vp.a.x, vp.a.y, vp.b.x, vp.b.y));

			if (lua_istable(L, 4)) {
				lua_pushnil(L);  // ... t ... nil
				while (0 != lua_next(L, 4)) {
					// ... t ... key value
					const char* key = luaL_checkstring(L, -2);
					if (lua_isnumber(L, -1)) {
						p_effect->setFloat(key, (float)lua_tonumber(L, -1));
					}
					else if (lua_isstring(L, -1)) {
						core::SmartReference<IResourceTexture> ptex = LRES.FindTexture(lua_tostring(L, -1));
						if (!ptex)
							return luaL_error(L, "texture '%s' not found.", rt_name);
						check_rendertarget_usage(ptex);
						p_effect->setTexture2D(key, ptex->GetTexture());
					}
					else if (lua_isuserdata(L, -1)) {
						core::Color4B color = *binding::Color::Cast(L, -1);
						p_effect->setFloat4(key, core::Vector4F(
							float(color.r) / 255.0f,
							float(color.g) / 255.0f,
							float(color.b) / 255.0f,
							float(color.a) / 255.0f
						));
					}
					else {
						return luaL_error(L, "PostEffect: invalid data type.");
					}
					lua_pop(L, 1);  // ... t ... key
				}
			}

			LR2D()->drawPostEffect(pfx->GetPostEffectShader(), blend);

			return 0;
		}

		// 下面是傻逼风格

		const char* ps_name = luaL_checkstring(L, 1);
		const char* rt_name = luaL_checkstring(L, 2);
		const core::Graphics::IRenderer::SamplerState rtsv = (core::Graphics::IRenderer::SamplerState)luaL_checkinteger(L, 3);
		const core::Graphics::IRenderer::BlendState blend = translate_blend_3d(TranslateBlendMode(L, 4));

		core::SmartReference<IResourcePostEffectShader> pfx = LRES.FindFX(ps_name);
		if (!pfx)
			return luaL_error(L, "posteffect '%s' not found.", ps_name);

		core::SmartReference<IResourceTexture> prt = LRES.FindTexture(rt_name);
		if (!prt)
			return luaL_error(L, "texture '%s' not found.", rt_name);
		check_rendertarget_usage(prt);

		core::Vector4F cbdata[8] = {};
		core::Graphics::ITexture2D* tdata[4] = {};
		core::Graphics::IRenderer::SamplerState tsdata[4] = {};

		size_t cbdata_n = lua_objlen(L, 5);
		cbdata_n = (cbdata_n <= 8) ? cbdata_n : 8;
		for (int i = 1; i <= (int)cbdata_n; i += 1) {
			lua_rawgeti(L, 5, i);  // ??? t
			luaL_argcheck(L, lua_istable(L, -1), 5, "shader constant values must be an array of lua table, each table contains 4 lua numbers");
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
		for (int i = 1; i <= (int)tdata_n; i += 1) {
			lua_rawgeti(L, 6, i);  // ??? t
			luaL_argcheck(L, lua_istable(L, -1), 6, "shader resources must be an array of lua table, each table contains the name of texture and sampler type");
			lua_rawgeti(L, -1, 1); // ??? t tex
			lua_rawgeti(L, -2, 2); // ??? t tex sampler
			const char* tx_name = luaL_checkstring(L, -2);
			core::SmartReference<IResourceTexture> ptex = LRES.FindTexture(tx_name);
			if (!ptex)
				return luaL_error(L, "texture '%s' not found.", tx_name);
			check_rendertarget_usage(ptex);
			tdata[i - 1] = ptex->GetTexture();
			tsdata[i - 1] = (core::Graphics::IRenderer::SamplerState)luaL_checkinteger(L, -1);
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
		{ "RenderModel", &lib_drawModel },
		{ "SetFog", &compat_SetFog },
		{ "SetZBufferEnable", &compat_SetZBufferEnable },
		{ "ClearZBuffer", &compat_ClearZBuffer },
		{ "PushRenderTarget", &compat_PushRenderTarget },
		{ "PopRenderTarget", &compat_PopRenderTarget },
		{ "PostEffect", &compat_PostEffect },
		{ NULL, NULL },
	};
}

void luastg::binding::Renderer::Register(lua_State* L)noexcept {
	luaL_register(L, LUASTG_LUA_LIBNAME, lib_compat);           // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Renderer", lib_func); // ??? lstg lstg.Renderer
	lua_setfield(L, -1, "Renderer");                            // ??? lstg
	lua_pop(L, 1);                                              // ???
}
