#include "GameResource/Implement/ResourceSpineImpl.hpp"
#include "AppFrame.h"
#include "Core/FileSystem.hpp"

namespace luastg
{
	ResourceSpineImpl::ResourceSpineImpl(const char* name, const char* path)
		: ResourceBaseImpl(ResourceType::Spine, name)
	{

	}
}

namespace spine
{
	LuaSTGAtlasAttachmentLoader::LuaSTGAtlasAttachmentLoader(Atlas* atlas) : AtlasAttachmentLoader(atlas) {}
	LuaSTGAtlasAttachmentLoader::~LuaSTGAtlasAttachmentLoader() {}
	void LuaSTGAtlasAttachmentLoader::configureAttachment(Attachment* attachment) {}

	LuaSTGTextureLoader::LuaSTGTextureLoader() : TextureLoader() {}
	LuaSTGTextureLoader::~LuaSTGTextureLoader() {}
	void LuaSTGTextureLoader::load(AtlasPage& page, const spine::String& path)
	{
        core::Graphics::ITexture2D* p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(path.buffer(), true, &p_texture))
        {
            spdlog::error("[luastg] 从 '{}' 创建Spine纹理失败", path.buffer());
            return;
        }

		p_texture->retain();
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

	LuaSTGExtension::LuaSTGExtension() : DefaultSpineExtension() {}

	LuaSTGExtension::~LuaSTGExtension() {}

	char* LuaSTGExtension::_readFile(const spine::String& path, int* length) {
		core::IData* data;
		if(!core::FileSystemManager::readFile(path.buffer(), &data)) return nullptr;

		*length = static_cast<int>(data->size());
		char* bytes = SpineExtension::alloc<char>(*length, __FILE__, __LINE__);
		std::memcpy(bytes, data->data(), *length);
		return bytes;
	}

	SpineExtension* spine::getDefaultExtension() { return new LuaSTGExtension(); }
}