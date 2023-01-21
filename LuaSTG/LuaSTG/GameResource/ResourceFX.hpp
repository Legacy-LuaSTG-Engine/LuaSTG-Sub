#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus
{
    // shader包装
    class ResFX : public ResourceBase
    {
    private:
        Core::ScopeObject<Core::Graphics::IPostEffectShader> m_shader;
    public:
        Core::Graphics::IPostEffectShader* GetPostEffectShader() noexcept { return *m_shader; }
    public:
        ResFX(const char* name, const char* path);
        virtual ~ResFX();
    };
}
