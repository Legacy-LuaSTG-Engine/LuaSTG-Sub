#pragma once
#include "Core/Graphics/Renderer.hpp"

namespace Core::Graphics
{
	struct ISprite : public IObject
	{
		virtual ITexture2D* getTexture() = 0;
		virtual void setTexture(ITexture2D* p_texture) = 0;

		virtual void setTextureRect(RectF const& rc) = 0;
		virtual RectF getTextureRect() = 0;

		virtual void setTextureCenter(Vector2F const& pt) = 0;
		virtual Vector2F getTextureCenter() = 0;

		virtual void setUnitsPerPixel(float v) = 0;
		virtual float getUnitsPerPixel() = 0;

		virtual void setZ(float v) = 0;
		virtual float getZ() = 0;

		virtual void setColor(Color4B color) = 0;
		virtual void setColor(Color4B const* color) = 0;
		virtual void getColor(Color4B* color) = 0;

		virtual void draw(RectF const& rc) = 0;
		virtual void draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) = 0;
		virtual void draw(Vector2F const& pos) = 0;
		virtual void draw(Vector2F const& pos, float scale) = 0;
		virtual void draw(Vector2F const& pos, float scale, float rotation) = 0;
		virtual void draw(Vector2F const& pos, Vector2F const& scale) = 0;
		virtual void draw(Vector2F const& pos, Vector2F const& scale, float rotation) = 0;

		virtual bool clone(ISprite** pp_sprite) = 0;

		static bool create(IRenderer* p_renderer, ITexture2D* p_texture, ISprite** pp_sprite);
	};
}
