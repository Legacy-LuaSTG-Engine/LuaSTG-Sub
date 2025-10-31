#pragma once
#ifdef LUASTG_SUPPORTS_SPINE
#include "GameResource/ResourceSpineAtlas.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include <spine/spine.h>

namespace luastg
{
	class ResourceSpineAtlasImpl : public ResourceBaseImpl<IResourceSpineAtlas>
	{
	private:
		std::shared_ptr<spine::Atlas> atlas;
	
	public:
		const std::shared_ptr<spine::Atlas>& getAtlas();
	public:
		ResourceSpineAtlasImpl(const char* name, const char* atlasPath, spine::TextureLoader* textureLoader);
	};
}
#endif // LUASTG_SUPPORTS_SPINE