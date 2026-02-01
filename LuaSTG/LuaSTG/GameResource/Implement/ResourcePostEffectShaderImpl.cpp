#include "GameResource/Implement/ResourcePostEffectShaderImpl.hpp"
#include "AppFrame.h"

namespace luastg
{
	ResourcePostEffectShaderImpl::ResourcePostEffectShaderImpl(const char* name, const char* path)
		: ResourceBaseImpl(ResourceType::FX, name)
	{
		LAPP.getRenderer2D()->createPostEffectShader(path, m_shader.put());
	}
}
