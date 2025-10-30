#include "ResourceSpineAdaptor.hpp"
#include "AppFrame.h"
#include "Core/FileSystem.hpp"

namespace spine
{
	using SamplerStateEnum = core::Graphics::IRenderer::SamplerState;
	// 沟槽的采样模式映射
	static SamplerStateEnum mapSpineToD3D11(
		TextureFilter minFilter, TextureFilter magFilter,
		TextureWrap uWrap, TextureWrap vWrap,
		bool& enableMipmap)
	{
		bool isLinear = false;
		if (minFilter == TextureFilter_Linear || minFilter == TextureFilter_MipMapLinearNearest ||
			minFilter == TextureFilter_MipMap || magFilter == TextureFilter_Linear) 
			isLinear = true;
		
		if (minFilter == TextureFilter_MipMapNearestNearest || magFilter == TextureFilter_MipMapNearestNearest ||
			minFilter == TextureFilter_MipMapLinearNearest || magFilter == TextureFilter_MipMapLinearNearest ||
			minFilter == TextureFilter_MipMapNearestLinear || magFilter == TextureFilter_MipMapNearestLinear ||
			minFilter == TextureFilter_MipMapLinearLinear || magFilter == TextureFilter_MipMapLinearLinear)
			enableMipmap = true;

		SamplerStateEnum mode;
		if (uWrap == TextureWrap_Repeat)
			mode = isLinear ? SamplerStateEnum::LinearWrap : SamplerStateEnum::PointWrap;
		else if (uWrap == TextureWrap_ClampToEdge)
			mode = isLinear ? SamplerStateEnum::LinearClamp : SamplerStateEnum::PointClamp;
		else 
			mode = SamplerStateEnum::LinearWrap;
		
		return mode;
	}

	LuaSTGAtlasAttachmentLoader::LuaSTGAtlasAttachmentLoader(Atlas* atlas) : AtlasAttachmentLoader(atlas) {}
	LuaSTGAtlasAttachmentLoader::~LuaSTGAtlasAttachmentLoader() {}
	void LuaSTGAtlasAttachmentLoader::configureAttachment(Attachment* attachment) {}

	LuaSTGTextureLoader::LuaSTGTextureLoader() : TextureLoader() {}
	LuaSTGTextureLoader::~LuaSTGTextureLoader() {}
	void LuaSTGTextureLoader::load(AtlasPage& page, const spine::String& path)
	{
		bool enableMipmap = false;
		SamplerStateEnum state = mapSpineToD3D11(page.minFilter, page.magFilter, page.uWrap, page.uWrap, enableMipmap);

		core::Graphics::ITexture2D* p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(path.buffer(), enableMipmap, &p_texture))
        {
            spdlog::error("[luastg] 从 '{}' 创建Spine纹理失败", path.buffer());
            return;
        }
		core::Graphics::ISamplerState* p_sampler = LAPP.GetRenderer2D()->getKnownSamplerState(state);
		p_texture->setSamplerState(p_sampler);
		
		auto [w, h] = p_texture->getSize();
		
		page.texture = p_texture;
		page.width = w;
		page.height = h;
	}
	void LuaSTGTextureLoader::unload(void* texture)
	{
		if (!texture) return;
		core::Graphics::ITexture2D* p_texture = (core::Graphics::ITexture2D*)texture;
		p_texture->release();
	}
	LuaSTGTextureLoader& LuaSTGTextureLoader::Instance()
	{
		static LuaSTGTextureLoader _instance;
		return _instance;
	}

	LuaSTGExtension::LuaSTGExtension() : DefaultSpineExtension() {}
	LuaSTGExtension::~LuaSTGExtension() {}
	char* LuaSTGExtension::_readFile(const spine::String& path, int* length) {
		core::IData* data;
		if(!core::FileSystemManager::readFile(path.buffer(), &data)) return nullptr;

		*length = static_cast<int>(data->size());
		char* bytes = SpineExtension::alloc<char>(*length, __FILE__, __LINE__);
		std::memcpy(bytes, data->data(), *length);
		data->release();
		return bytes;
	}
	LuaSTGExtension& LuaSTGExtension::Instance() {
		static LuaSTGExtension _instance;
		return _instance;
	}

	LuaSTGSpineInstance::LuaSTGSpineInstance(const std::string_view& name, spine::SkeletonData* skeldata, spine::AnimationStateData* anidata)
		: resname(name)
		, skeleton(new spine::Skeleton(skeldata))
		, anistate(new spine::AnimationState(anidata))
	{
		// name -> bone mapping
		auto bones = skeleton->getBones();
		auto bone_size = bones.size();
		for (int i = 0; i < bone_size; i++) bonecache[bones[i]->getData().getName().buffer()] = bones[i];
	};
	const std::string_view& LuaSTGSpineInstance::getName() { return resname; }
	const std::unordered_map<std::string_view, Bone*>& LuaSTGSpineInstance::getAllBones() { return bonecache; }
	Skeleton* LuaSTGSpineInstance::getSkeleton() { return skeleton.get(); }
	AnimationState* LuaSTGSpineInstance::getAnimationState() { return anistate.get(); }
	Bone* LuaSTGSpineInstance::findBone(const char* name) { return bonecache.contains(name) ? bonecache[name] : nullptr; }

	SpineExtension* spine::getDefaultExtension() { return &LuaSTGExtension::Instance(); }
}