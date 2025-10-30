#pragma once
#include "GameResource/ResourceSpine.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"
#include <spine/spine.h>

namespace spine
{
	class LuaSTGAtlasAttachmentLoader : public AtlasAttachmentLoader
	{
	public:
		LuaSTGAtlasAttachmentLoader(Atlas *atlas);
		virtual ~LuaSTGAtlasAttachmentLoader();
		virtual void configureAttachment(Attachment *attachment);
	};

	class LuaSTGTextureLoader : public TextureLoader
	{
	public:
		LuaSTGTextureLoader();
		virtual ~LuaSTGTextureLoader();
		virtual void load(AtlasPage &page, const String &path);
		virtual void unload(void *texture);
	};

	class LuaSTGExtension : public DefaultSpineExtension
	{
	public:
		LuaSTGExtension();
		virtual ~LuaSTGExtension();

	protected:
		virtual char *_readFile(const String &path, int *length);
	};
}

namespace luastg
{
	class ResourceSpineImpl : public ResourceBaseImpl<IResourceSpine>
	{
	private:
		inline static spine::LuaSTGTextureLoader textureLoader{};
		spine::Atlas* atlas;
		spine::SkeletonData* skeldata;
	
	public:
		ResourceSpineImpl(const char* name, const char* atlasPath, const char* skelPath);
	};
}