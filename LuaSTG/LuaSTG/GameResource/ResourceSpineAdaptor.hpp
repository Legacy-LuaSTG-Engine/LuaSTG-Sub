#pragma once
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
	private:
	public:
		static LuaSTGExtension& Instance();
		LuaSTGExtension();
		virtual ~LuaSTGExtension();

	protected:
		virtual char *_readFile(const String &path, int *length);
	};
}
