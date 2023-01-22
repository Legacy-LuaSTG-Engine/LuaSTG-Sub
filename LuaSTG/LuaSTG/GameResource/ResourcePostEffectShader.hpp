#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus
{
    struct IResourcePostEffectShader : public IResourceBase
    {
        virtual Core::Graphics::IPostEffectShader* GetPostEffectShader() = 0;
    };
}
