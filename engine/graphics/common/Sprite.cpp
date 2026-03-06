#include "common/Sprite.hpp"

namespace core {
    void Sprite::setTextureRect(const RectF& rc) {
        m_rect = rc;
    }
    void Sprite::setTextureCenter(const Vector2F& pt) {
        m_center = pt;
    }
    void Sprite::setUnitsPerPixel(const float v) {
        m_scale = v;
    }

    bool Sprite::clone(ISprite** const out_sprite) {
        if (out_sprite == nullptr) {
            assert(false); return false;
        }
        const auto right = new Sprite(m_texture.get());
        right->m_rect = m_rect;
        right->m_center = m_center;
        right->m_scale = m_scale;
        *out_sprite = right;
        return true;
    }

    Sprite::Sprite(ITexture2D* const texture) : m_texture(texture), m_scale(1.0f) {
        const auto size = m_texture->getSize();
        m_rect = RectF(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y));
        m_center = Vector2F(m_rect.b.x * 0.5f, m_rect.b.y * 0.5f);
    }
    Sprite::~Sprite() = default;
}

namespace core::Graphics {
    bool ISprite::create(ITexture2D* const texture, ISprite** const out_sprite) {
        if (texture == nullptr) {
            assert(false); return false;
        }
        if (out_sprite == nullptr) {
            assert(false); return false;
        }
        *out_sprite = new Sprite(texture);
        return true;
    }
}
