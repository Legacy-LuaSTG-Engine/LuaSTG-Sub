#include "common/Sprite.hpp"

namespace core {
	void Sprite::setTextureRect(RectF const& rc) {
		m_rect = rc;
	}
	void Sprite::setTextureCenter(Vector2F const& pt) {
		m_center = pt;
	}
	void Sprite::setUnitsPerPixel(float const v) {
		m_scale = v;
	}

	bool Sprite::clone(ISprite** const pp_sprite) {
		auto const right = new Sprite(m_texture.get());
		right->m_rect = m_rect;
		right->m_center = m_center;
		right->m_scale = m_scale;
		*pp_sprite = right;
		return true;
	}

	Sprite::Sprite(ITexture2D* const p_texture) : m_texture(p_texture), m_scale(1.0f) {
		Vector2U const size = m_texture->getSize();
		m_rect = RectF(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y));
		m_center = Vector2F(m_rect.b.x * 0.5f, m_rect.b.y * 0.5f);
	}
	Sprite::~Sprite() = default;
}

namespace core::Graphics {
	bool ISprite::create(ITexture2D* const p_texture, ISprite** const pp_sprite) {
		*pp_sprite = new Sprite(p_texture);
		return true;
	}
}
