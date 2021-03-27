#pragma once
#include "Graphic/Device.h"

namespace slow::Graphic
{
    class SpriteRenderer
    {
    private:
        struct Implement;
        Implement* implememt = nullptr;
    public:
        bool bind(Device& device);
        void unbind();
    public:
        SpriteRenderer();
        ~SpriteRenderer();
        static SpriteRenderer& get();
    };
};
