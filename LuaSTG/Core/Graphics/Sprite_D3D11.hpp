#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace Core::Graphics
{
	class Sprite_D3D11 : public Object<ISprite>
	{
	private:
		ScopeObject<IRenderer> m_renderer;
		ScopeObject<ITexture2D> m_texture;
		RectF m_rect;
		RectF m_pos_rc;
		RectF m_uv;
		Vector2F m_center;
		float m_z;
		float m_scale;
		Color4B m_color[4];

		void updateRect();

	public:
		ITexture2D* getTexture() { return m_texture.get(); }
		void setTexture(ITexture2D* p_texture) { m_texture = p_texture; }

		void setTextureRect(RectF const& rc);
		RectF getTextureRect() { return m_rect; }

		void setTextureCenter(Vector2F const& pt);
		Vector2F getTextureCenter() { return m_center; }

		void setUnitsPerPixel(float v);
		float getUnitsPerPixel() { return m_scale; }

		void setZ(float v) { m_z = v; }
		float getZ() { return m_z; }

		void setColor(Color4B color)
		{
			m_color[0] = color;
			m_color[1] = color;
			m_color[2] = color;
			m_color[3] = color;
		}
		void setColor(Color4B const* color)
		{
			m_color[0] = color[0];
			m_color[1] = color[1];
			m_color[2] = color[2];
			m_color[3] = color[3];
		}
		void getColor(Color4B* color)
		{
			color[0] = m_color[0];
			color[1] = m_color[1];
			color[2] = m_color[2];
			color[3] = m_color[3];
		}

		void draw(RectF const& rc);
		void draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4);
		void draw(Vector2F const& pos);
		void draw(Vector2F const& pos, float scale);
		void draw(Vector2F const& pos, float scale, float rotation);
		void draw(Vector2F const& pos, Vector2F const& scale);
		void draw(Vector2F const& pos, Vector2F const& scale, float rotation);

		bool clone(ISprite** pp_sprite);

	public:
		Sprite_D3D11(IRenderer* p_renderer, ITexture2D* p_texture);
		~Sprite_D3D11();
	};
}
