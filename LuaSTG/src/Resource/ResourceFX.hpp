#pragma once
#include "ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus {
    // shader包装
    class ResFX : public Resource
    {
    private:
        LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IPostEffectShader> m_shader;
    public:
        LuaSTG::Core::Graphics::IPostEffectShader* GetPostEffectShader() noexcept { return *m_shader; }
    public:
        ResFX(const char* name, const char* path);
        virtual ~ResFX();
    };
}
