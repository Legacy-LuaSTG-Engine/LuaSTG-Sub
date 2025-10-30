#pragma once
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

	class LuaSTGSpineInstance
	{
	private:
		std::string_view resname;
		std::unique_ptr<Skeleton> skeleton;
		std::unique_ptr<AnimationState> anistate;
		std::unordered_map<std::string_view, Bone*> bonecache;
		std::unordered_map<std::string_view, Animation*> animationcache;
	public:
		const std::string_view& getName();
		const std::unordered_map<std::string_view, Bone*>& getAllBones();
		const std::unordered_map<std::string_view, Animation*>& getAllAnimations();
		Skeleton* getSkeleton();
		AnimationState* getAnimationState();
		Bone* findBone(const char* name);
		Animation* findAnimation(const char* name);
	public:
		LuaSTGSpineInstance(const std::string_view& name, spine::SkeletonData* skeldata, spine::AnimationStateData* anidata);
	};
}
