﻿#pragma once
#include "Core/Type.hpp"
#include "Core/Graphics/Device.hpp"

namespace Core::Graphics
{
	struct IPostEffectShader : public IObject
	{
	};

	struct IModel : public IObject
	{
		virtual void setAmbient(Vector3F const& color, float brightness) = 0;
		virtual void setDirectionalLight(Vector3F const& direction, Vector3F const& color, float brightness) = 0;

		virtual void setScaling(Vector3F const& scale) = 0;
		virtual void setPosition(Vector3F const& pos) = 0;
		virtual void setRotationRollPitchYaw(float roll, float pitch, float yaw) = 0;
		virtual void setRotationQuaternion(Vector4F const& quat) = 0;
	};

	struct IRenderer : public IObject
	{
		enum class VertexColorBlendState : uint8_t
		{
			MIN_INDEX = 0,

			Zero = MIN_INDEX,
			One,
			Add,
			Mul,

			MAX_INDEX = Mul,
			MAX_COUNT,
		};
		enum class FogState : uint8_t
		{
			MIN_INDEX = 0,

			Disable = MIN_INDEX,
			Linear,
			Exp,
			Exp2,

			MAX_INDEX = Exp2,
			MAX_COUNT,
		};
		enum class DepthState : uint8_t
		{
			MIN_INDEX = 0,

			Disable = MIN_INDEX,
			Enable,

			MAX_INDEX = Enable,
			MAX_COUNT,
		};
		enum class BlendState : uint8_t
		{
			MIN_INDEX = 0,

			Disable = MIN_INDEX,
			Alpha,
			One,
			Min,
			Max,
			Mul,
			Screen,
			Add,
			Sub,
			RevSub,
			Inv,

			MAX_INDEX = Inv,
			MAX_COUNT,
		};
		enum class SamplerState : uint8_t
		{
			MIN_INDEX = 0,

			PointWrap = MIN_INDEX,
			PointClamp,
			PointBorderBlack,
			PointBorderWhite,
			LinearWrap,
			LinearClamp,
			LinearBorderBlack,
			LinearBorderWhite,

			MAX_INDEX = LinearBorderWhite,
			MAX_COUNT,
		};
		enum class TextureAlphaType : uint8_t
		{
			MIN_INDEX = 0,

			Normal = MIN_INDEX,
			PremulAlpha,

			MAX_INDEX = PremulAlpha,
			MAX_COUNT,
		};

		struct DrawVertex
		{
			float x, y, z;
			uint32_t color;
			float u, v;

			DrawVertex() : x(0.0f), y(0.0f), z(0.0f), u(0.0f), v(0.0f), color(0u) {}
			DrawVertex(float const x_, float const y_, float const z_, float const u_, float const v_, uint32_t const c_)
				: x(x_), y(y_), z(z_), u(u_), v(v_), color(c_) {}
			DrawVertex(float const x_, float const y_, float const z_, float const u_, float const v_)
				: x(x_), y(y_), z(z_), u(u_), v(v_), color(0xFFFFFFFFu) {}
			DrawVertex(float const x_, float const y_, float const u_, float const v_)
				: x(x_), y(y_), z(0.5f), u(u_), v(v_), color(0xFFFFFFFFu) {} // TODO: z = 0.0f or z = 0.5f ?
		};
		using DrawIndex = uint16_t;

		virtual bool beginBatch() = 0;
		virtual bool endBatch() = 0;
		virtual bool flush() = 0;

		virtual void clearRenderTarget(Color4B const& color) = 0;
		virtual void clearDepthBuffer(float zvalue) = 0;
		virtual void setRenderAttachment(IRenderTarget* p_rt, IDepthStencilBuffer* p_ds) = 0;

		virtual void setOrtho(BoxF const& box) = 0;
		virtual void setPerspective(Vector3F const& eye, Vector3F const& lookat, Vector3F const& headup, float fov, float aspect, float znear, float zfar) = 0;

		virtual void setViewport(BoxF const& box) = 0;
		virtual void setScissorRect(RectF const& rect) = 0;
		virtual void setViewportAndScissorRect() = 0;

		virtual void setVertexColorBlendState(VertexColorBlendState state) = 0;
		virtual void setSamplerState(SamplerState state) = 0;
		virtual void setFogState(FogState state, Color4B const& color, float density_or_znear, float zfar) = 0;
		virtual void setDepthState(DepthState state) = 0;
		virtual void setBlendState(BlendState state) = 0;
		virtual void setTexture(ITexture2D* texture) = 0;

		virtual bool drawTriangle(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3) = 0;
		virtual bool drawTriangle(DrawVertex const* pvert) = 0;
		virtual bool drawQuad(DrawVertex const& v1, DrawVertex const& v2, DrawVertex const& v3, DrawVertex const& v4) = 0;
		virtual bool drawQuad(DrawVertex const* pvert) = 0;
		virtual bool drawRaw(DrawVertex const* pvert, uint16_t nvert, DrawIndex const* pidx, uint16_t nidx) = 0;
		virtual bool drawRequest(uint16_t nvert, uint16_t nidx, DrawVertex** ppvert, DrawIndex** ppidx, uint16_t* idxoffset) = 0;

		virtual bool createPostEffectShader(StringView path, IPostEffectShader** pp_effect) = 0;
		virtual bool drawPostEffect(
			IPostEffectShader* p_effect,
			BlendState blend,
			ITexture2D* p_tex, SamplerState rtsv,
			Vector4F const* cv, size_t cv_n,
			ITexture2D* const* p_tex_arr, SamplerState const* sv, size_t tv_sv_n) = 0;

		virtual bool createModel(StringView path, IModel** pp_model) = 0;
		virtual bool drawModel(IModel* p_model) = 0;

		virtual ISamplerState* getKnownSamplerState(SamplerState state) = 0;

		static bool create(IDevice* p_device, IRenderer** pp_renderer);
	};
}
