#include "GameResource/SharedSpriteRenderer.hpp"
#include "core/SmartReference.hpp"

namespace {
    core::SmartReference<core::Graphics::ISpriteRenderer> s_renderer;
}

namespace luastg {
    bool SharedSpriteRenderer::create() {
        return core::Graphics::ISpriteRenderer::create(s_renderer.put());
    }
    void SharedSpriteRenderer::destroy() {
        s_renderer.reset();
    }
    core::Graphics::ISpriteRenderer* SharedSpriteRenderer::getInstance() {
        return s_renderer.get();
    }
}
