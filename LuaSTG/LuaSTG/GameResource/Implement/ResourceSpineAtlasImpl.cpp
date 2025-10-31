#ifdef LUASTG_SUPPORTS_SPINE
#include "GameResource/Implement/ResourceSpineAtlasImpl.hpp"
#include "GameResource/Implement/ResourceTextureImpl.hpp"
#include "AppFrame.h"
#include "Core/FileSystem.hpp"
#include "Core/SmartReference.hpp"

namespace luastg
{
	const std::shared_ptr<spine::Atlas>& ResourceSpineAtlasImpl::getAtlas() { return atlas; }
	ResourceSpineAtlasImpl::ResourceSpineAtlasImpl(const char* name, const char* atlas_path, spine::TextureLoader* textureLoader)
		: ResourceBaseImpl(ResourceType::SpineAtlas, name),
		atlas(new spine::Atlas(atlas_path, textureLoader))
	{
	
	}

}
#endif // LUASTG_SUPPORTS_SPINE