#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/Graphics/Sprite.hpp"

namespace core {
    class Sprite final : public implement::ReferenceCounted<Graphics::ISprite> {
    public:
        // ISprite

        ITexture2D* getTexture() override { return m_texture.get(); }
        void setTexture(ITexture2D* const texture) override { m_texture = texture; }
        void setTextureRect(const RectF& rc) override;
        RectF getTextureRect() override { return m_rect; }
        void setTextureCenter(const Vector2F& pt) override;
        Vector2F getTextureCenter() override { return m_center; }
        void setUnitsPerPixel(float v) override;
        float getUnitsPerPixel() override { return m_scale; }

        bool clone(ISprite** out_sprite) override;

        // Sprite

        Sprite(ITexture2D* texture);
        Sprite(const Sprite&) = delete;
        Sprite(Sprite&&) = delete;
        ~Sprite();

        Sprite& operator=(const Sprite&) = delete;
        Sprite& operator=(Sprite&&) = delete;

    private:
        SmartReference<ITexture2D> m_texture;
        RectF m_rect;
        Vector2F m_center;
        float m_scale;
    };
}
