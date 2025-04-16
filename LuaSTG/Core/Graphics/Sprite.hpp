#pragma once
#include "Core/Graphics/Renderer.hpp"

namespace core::Graphics
{
	struct ISprite : IObject
	{
		virtual ITexture2D* getTexture() = 0;
		virtual void setTexture(ITexture2D* p_texture) = 0;

		virtual void setTextureRect(RectF const& rc) = 0;
		virtual RectF getTextureRect() = 0;

		virtual void setTextureCenter(Vector2F const& pt) = 0;
		virtual Vector2F getTextureCenter() = 0;

		virtual void setUnitsPerPixel(float v) = 0;
		virtual float getUnitsPerPixel() = 0;

		/* TODO: remove */ virtual void setZ(float v) = 0;
		/* TODO: remove */ virtual float getZ() = 0;

		/* TODO: remove */ virtual void setColor(Color4B color) = 0;
		/* TODO: remove */ virtual void setColor(Color4B const* color) = 0;
		/* TODO: remove */ virtual void getColor(Color4B* color) = 0;

		/* TODO: remove */ virtual void draw(RectF const& rc) = 0;
		/* TODO: remove */ virtual void draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) = 0;
		/* TODO: remove */ virtual void draw(Vector2F const& pos) = 0;
		/* TODO: remove */ virtual void draw(Vector2F const& pos, float scale) = 0;
		/* TODO: remove */ virtual void draw(Vector2F const& pos, float scale, float rotation) = 0;
		/* TODO: remove */ virtual void draw(Vector2F const& pos, Vector2F const& scale) = 0;
		/* TODO: remove */ virtual void draw(Vector2F const& pos, Vector2F const& scale, float rotation) = 0;

		virtual bool clone(ISprite** pp_sprite) = 0;

		static bool create(IRenderer* p_renderer, ITexture2D* p_texture, ISprite** pp_sprite);
	};

	struct ISpriteRenderer : IObject {
		virtual void setTransform(RectF const& rect) = 0;
		virtual void setTransform(Vector2F const& p1, Vector2F const& p2, Vector2F const& p3, Vector2F const& p4) = 0;
		virtual void setTransform(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) = 0;
		virtual void setTransform(Vector2F const& position, Vector2F const& scale, float rotation) = 0;
		virtual void setSprite(ISprite* sprite) = 0;
		virtual void setColor(Color4B color) = 0;
		virtual void setColor(Color4B c1, Color4B c2, Color4B c3, Color4B c4) = 0;
		virtual void setZ(float z) = 0;
		virtual void setLegacyBlendState(IRenderer::VertexColorBlendState vertex_color_blend_state, IRenderer::BlendState blend_state) = 0;
		virtual void draw(IRenderer* renderer) = 0;

		static bool create(ISpriteRenderer** output);
	};
}
