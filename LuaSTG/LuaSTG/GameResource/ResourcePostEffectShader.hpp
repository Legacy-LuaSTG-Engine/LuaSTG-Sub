#pragma once
#include "GameResource/ResourceBase.hpp"
#include "core/Graphics/Renderer.hpp"

namespace luastg
{
    struct IResourcePostEffectShader : public IResourceBase
    {
        virtual core::Graphics::IPostEffectShader* GetPostEffectShader() = 0;
    };
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourcePostEffectShader
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourcePostEffectShader>() { return UUID::parse("b8f16675-08d6-50f3-9049-a1f90277a86a"); }
}
