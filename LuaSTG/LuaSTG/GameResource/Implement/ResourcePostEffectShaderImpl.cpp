#include "GameResource/Implement/ResourcePostEffectShaderImpl.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	ResourcePostEffectShaderImpl::ResourcePostEffectShaderImpl(const char* name, const char* path)
		: ResourceBaseImpl(ResourceType::FX, name)
	{
		LAPP.GetAppModel()->getRenderer()->createPostEffectShader(path, ~m_shader);
	}
}
