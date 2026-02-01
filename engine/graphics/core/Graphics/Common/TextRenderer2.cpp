#include "core/Graphics/Common/TextRenderer2.hpp"
#include <cassert>

namespace core::Graphics {
    void TextRenderer2::setTransform(RectF const& rect) {
        m_sprite_renderer->setTransform(rect);
    }
    void TextRenderer2::setTransform(Vector2F const& p1, Vector2F const& p2, Vector2F const& p3, Vector2F const& p4) {
        m_sprite_renderer->setTransform(p1, p2, p3, p4);
    }
    void TextRenderer2::setTransform(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) {
        m_sprite_renderer->setTransform(p1, p2, p3, p4);
    }
    void TextRenderer2::setTransform(Vector2F const& position, Vector2F const& scale, const float rotation) {
        if (!m_text_layout) {
            return;
        }
        if (!update()) {
            return;
        }
        m_sprite_renderer->setTransform(position, scale, rotation);
    }
    void TextRenderer2::setTextLayout(ITextLayout* const text_layout) {
        if (m_text_layout.get() != text_layout) {
            m_text_layout = text_layout;
            if (m_texture) {
                m_texture->setImage(nullptr);
            }
            m_last_version = 0;
        }
    }
    void TextRenderer2::setColor(const Color4B color) {
        m_sprite_renderer->setColor(color);
    }
    void TextRenderer2::setColor(const Color4B c1, const Color4B c2, const Color4B c3, const Color4B c4) {
        m_sprite_renderer->setColor(c1, c2, c3, c4);
    }
    void TextRenderer2::setAnchor(const Vector2F anchor) {
        m_anchor = anchor;
    }
    void TextRenderer2::setZ(const float z) {
        m_sprite_renderer->setZ(z);
    }
    void TextRenderer2::setLegacyBlendState(const IRenderer::VertexColorBlendState vertex_color_blend_state, const IRenderer::BlendState blend_state) {
        m_sprite_renderer->setLegacyBlendState(vertex_color_blend_state, blend_state);
    }
    void TextRenderer2::draw(IRenderer* renderer) {
        if (!m_text_layout) {
            return;
        }
        if (!update()) {
            return;
        }
        m_sprite_renderer->draw(renderer);
    }

    // TextRenderer2

    bool TextRenderer2::initialize(IGraphicsDevice* const device) {
        assert(device != nullptr);
        m_device = device;
        if (!ISpriteRenderer::create(m_sprite_renderer.put())) {
            return false;
        }
        return true;
    }
    bool TextRenderer2::update() {
        if (!m_text_layout) {
            return true;
        }

        const auto image = m_text_layout->getImage();
        if (!image) {
            return false;
        }

        const auto size = image->getSize();

        if (!m_texture) {
            if (!m_device->createTexture(size, m_texture.put())) {
                return false;
            }
            m_texture->setPremultipliedAlpha(true);
        }
        else if (m_texture->getSize() != size) {
            m_sprite_renderer->setSprite(nullptr);
            m_sprite.reset();
            m_last_version = 0;
            if (!m_texture->setSize(size)) {
                return false;
            }
        }

        if (!m_sprite) {
            if (!ISprite::create(m_texture.get(), m_sprite.put())) {
                return false;
            }
            m_sprite->setTextureCenter(m_anchor);
            m_sprite_renderer->setSprite(m_sprite.get());
        }

        if (m_text_layout->getVersion() > m_last_version) {
            ScopedImageMappedBuffer buffer{};
            if (!image->createScopedMap(buffer)) {
                return false;
            }

            if (!m_texture->update(core::RectU({}, size), buffer.data, buffer.stride)) {
                return false;
            }
            m_texture->setImage(image);
        }

        return true;
    }
}

namespace core::Graphics {
    bool ITextRenderer2::create(IGraphicsDevice* const device, ITextRenderer2** const output) {
        if (device == nullptr || output == nullptr) {
            assert(false);
            return false;
        }
        SmartReference<TextRenderer2> renderer;
        renderer.attach(new TextRenderer2);
        if (!renderer->initialize(device)) {
            return false;
        }
        *output = renderer.detach();
        return true;
    }
}
