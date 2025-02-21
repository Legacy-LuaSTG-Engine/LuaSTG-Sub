#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace Core::Graphics {
	class Sprite final
		: public Object<ISprite>
	{
	public:
		// ISprite

		ITexture2D* getTexture() override { return m_texture.get(); }
		void setTexture(ITexture2D* const p_texture) override { m_texture = p_texture; }

		void setTextureRect(RectF const& rc) override;
		RectF getTextureRect() override { return m_rect; }

		void setTextureCenter(Vector2F const& pt) override;
		Vector2F getTextureCenter() override { return m_center; }

		void setUnitsPerPixel(float v) override;
		float getUnitsPerPixel() override { return m_scale; }

		void setZ(float const z) override { m_z = z; }
		float getZ() override { return m_z; }

		void setColor(Color4B const color) override {
			m_color[0] = color;
			m_color[1] = color;
			m_color[2] = color;
			m_color[3] = color;
		}
		void setColor(Color4B const* const color) override {
			m_color[0] = color[0];
			m_color[1] = color[1];
			m_color[2] = color[2];
			m_color[3] = color[3];
		}
		void getColor(Color4B* const color) override {
			color[0] = m_color[0];
			color[1] = m_color[1];
			color[2] = m_color[2];
			color[3] = m_color[3];
		}

		void draw(RectF const& rc) override;
		void draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) override;
		void draw(Vector2F const& pos) override;
		void draw(Vector2F const& pos, float scale) override;
		void draw(Vector2F const& pos, float scale, float rotation) override;
		void draw(Vector2F const& pos, Vector2F const& scale) override;
		void draw(Vector2F const& pos, Vector2F const& scale, float rotation) override;

		bool clone(ISprite** pp_sprite) override;

		// Sprite

		Sprite(IRenderer* p_renderer, ITexture2D* p_texture);
		Sprite(Sprite const&) = delete;
		Sprite(Sprite&&) = delete;
		Sprite& operator=(Sprite const&) = delete;
		Sprite& operator=(Sprite&&) = delete;
		~Sprite();

		void updateRect();

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
	};
}
