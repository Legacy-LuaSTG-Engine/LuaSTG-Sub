#pragma once
#include "core/Graphics/Sprite.hpp"

namespace luastg {
    class SharedSpriteRenderer {
    public:
        static bool create();
        static void destroy();
        static core::Graphics::ISpriteRenderer* getInstance();
    };
};
