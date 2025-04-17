#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace luastg
{
    struct IResourcePostEffectShader : public IResourceBase
    {
        virtual core::Graphics::IPostEffectShader* GetPostEffectShader() = 0;
    };
}
