#pragma once
#include "GameResource/ResourcePostEffectShader.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
    class ResourcePostEffectShaderImpl : public ResourceBaseImpl<IResourcePostEffectShader>
    {
    private:
        Core::ScopeObject<Core::Graphics::IPostEffectShader> m_shader;
    public:
        Core::Graphics::IPostEffectShader* GetPostEffectShader() noexcept { return *m_shader; }
    public:
        ResourcePostEffectShaderImpl(const char* name, const char* path);
    };
}
