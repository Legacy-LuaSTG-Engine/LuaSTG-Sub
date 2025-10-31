#pragma once
#ifdef LUASTG_SUPPORTS_SPINE
#include <spine/spine.h>

namespace spine
{
	void LuaSTGdummyOnAnimationEventFunc(AnimationState* state, spine::EventType type, TrackEntry* entry, Event* event);

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
		static LuaSTGTextureLoader& Instance();
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

	class LuaSTGSkeletonRenderer
	{
	public:
		static SkeletonRenderer& Instance();
	};
}

#endif