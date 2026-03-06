#pragma once
#include "core/Graphics/Font.hpp"
#include "core/Graphics/Sprite.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/TextLayout.hpp"

namespace core {
    class TextRenderer2 final : public core::implement::ReferenceCounted<Graphics::ITextRenderer2> {
    public:
        // ITextRenderer2

        void setTransform(RectF const& rect) override;
        void setTransform(Vector2F const& p1, Vector2F const& p2, Vector2F const& p3, Vector2F const& p4) override;
        void setTransform(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) override;
        void setTransform(Vector2F const& position, Vector2F const& scale, float rotation) override;
        void setTextLayout(ITextLayout* text_layout) override;
        void setColor(Color4B color) override;
        void setColor(Color4B c1, Color4B c2, Color4B c3, Color4B c4) override;
        void setAnchor(Vector2F anchor) override;
        void setZ(float z) override;
        void setLegacyBlendState(Graphics::IRenderer::VertexColorBlendState vertex_color_blend_state, Graphics::IRenderer::BlendState blend_state) override;
        void draw(Graphics::IRenderer* renderer) override;

        // TextRenderer2

        bool initialize(IGraphicsDevice* device);
        bool update();

    private:
        SmartReference<ITextLayout> m_text_layout;
        SmartReference<IGraphicsDevice> m_device;
        SmartReference<ITexture2D> m_texture;
        SmartReference<Graphics::ISprite> m_sprite;
        SmartReference<Graphics::ISpriteRenderer> m_sprite_renderer;
        Vector2F m_anchor{};
        uint32_t m_last_version{};
    };
}
