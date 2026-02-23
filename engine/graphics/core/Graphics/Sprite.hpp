#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Graphics/Renderer.hpp"

namespace core::Graphics
{
	struct ISprite : IReferenceCounted {
		virtual ITexture2D* getTexture() = 0;
		virtual void setTexture(ITexture2D* texture) = 0;
		virtual void setTextureRect(RectF const& rc) = 0;
		virtual RectF getTextureRect() = 0;
		virtual void setTextureCenter(Vector2F const& pt) = 0;
		virtual Vector2F getTextureCenter() = 0;
		virtual void setUnitsPerPixel(float v) = 0;
		virtual float getUnitsPerPixel() = 0;
		virtual bool clone(ISprite** out_sprite) = 0;

		static bool create(ITexture2D* texture, ISprite** out_sprite);
	};

	struct ISpriteRenderer : IReferenceCounted {
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

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.ISprite
	template<> constexpr InterfaceId getInterfaceId<Graphics::ISprite>() { return UUID::parse("291cd55f-be6c-5abf-a014-cbd47ab655a3"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.ISpriteRenderer
	template<> constexpr InterfaceId getInterfaceId<Graphics::ISpriteRenderer>() { return UUID::parse("928af8d9-f387-557b-a7d9-d1395b9bc0f1"); }
}
