#pragma once
#include "GameResource/ResourceSpineAtlas.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include <spine/spine.h>

namespace luastg
{
	class ResourceSpineAtlasImpl : public ResourceBaseImpl<IResourceSpineAtlas>
	{
	private:
		std::unique_ptr<spine::Atlas> atlas;
		std::unique_ptr<spine::SkeletonData> skeldata;
	
	public:
		ResourceSpineAtlasImpl(const char* name, const char* atlasPath, spine::TextureLoader* textureLoader);
	};
}