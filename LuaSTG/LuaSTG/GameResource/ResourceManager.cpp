#include "GameResource/ResourceManager.h"

namespace luastg
{
	ResourceMgr::ResourceMgr()
		: m_GlobalResourcePool(this, ResourcePoolType::Global)
		, m_StageResourcePool(this, ResourcePoolType::Stage)
	{
	}

	// 资源池管理

	void ResourceMgr::ClearAllResource() noexcept {
		m_GlobalResourcePool.Clear();
		m_StageResourcePool.Clear();
		m_ActivedPool = ResourcePoolType::Global;
		m_GlobalImageScaleFactor = 1.0f;
	}

	ResourcePoolType ResourceMgr::GetActivedPoolType() noexcept {
		return m_ActivedPool;
	}

	void ResourceMgr::SetActivedPoolType(ResourcePoolType t) noexcept {
		m_ActivedPool = t;
	}

	ResourcePool* ResourceMgr::GetActivedPool() noexcept {
		return GetResourcePool(m_ActivedPool);
	}

	ResourcePool* ResourceMgr::GetResourcePool(ResourcePoolType t) noexcept {
		switch (t) {
			case ResourcePoolType::Global:
				return &m_GlobalResourcePool;
			case ResourcePoolType::Stage:
				return &m_StageResourcePool;
			default:
				return nullptr;
		}
	}

	// 自动查找资源池资源

	core::SmartReference<IResourceTexture> ResourceMgr::FindTexture(const char* name) noexcept {

		core::SmartReference<IResourceTexture> tRet;
		if (!(tRet = m_StageResourcePool.GetTexture(name)))
			tRet = m_GlobalResourcePool.GetTexture(name);
		return tRet;
	}

	core::SmartReference<IResourceSprite> ResourceMgr::FindSprite(const char* name) noexcept {
		core::SmartReference<IResourceSprite> tRet;
		if (!(tRet = m_StageResourcePool.GetSprite(name)))
			tRet = m_GlobalResourcePool.GetSprite(name);
		return tRet;
	}

	core::SmartReference<IResourceAnimation> ResourceMgr::FindAnimation(const char* name) noexcept {
		core::SmartReference<IResourceAnimation> tRet;
		if (!(tRet = m_StageResourcePool.GetAnimation(name)))
			tRet = m_GlobalResourcePool.GetAnimation(name);
		return tRet;
	}

	core::SmartReference<IResourceMusic> ResourceMgr::FindMusic(const char* name) noexcept {
		core::SmartReference<IResourceMusic> tRet;
		if (!(tRet = m_StageResourcePool.GetMusic(name)))
			tRet = m_GlobalResourcePool.GetMusic(name);
		return tRet;
	}

	core::SmartReference<IResourceSoundEffect> ResourceMgr::FindSound(const char* name) noexcept {
		core::SmartReference<IResourceSoundEffect> tRet;
		if (!(tRet = m_StageResourcePool.GetSound(name)))
			tRet = m_GlobalResourcePool.GetSound(name);
		return tRet;
	}

	core::SmartReference<IResourceParticle> ResourceMgr::FindParticle(const char* name) noexcept {
		core::SmartReference<IResourceParticle> tRet;
		if (!(tRet = m_StageResourcePool.GetParticle(name)))
			tRet = m_GlobalResourcePool.GetParticle(name);
		return tRet;
	}

	core::SmartReference<IResourceFont> ResourceMgr::FindSpriteFont(const char* name) noexcept {
		core::SmartReference<IResourceFont> tRet;
		if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
			tRet = m_GlobalResourcePool.GetSpriteFont(name);
		return tRet;
	}

	core::SmartReference<IResourceFont> ResourceMgr::FindTTFFont(const char* name) noexcept {
		core::SmartReference<IResourceFont> tRet;
		if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
			tRet = m_GlobalResourcePool.GetTTFFont(name);
		return tRet;
	}

	core::SmartReference<IResourcePostEffectShader> ResourceMgr::FindFX(const char* name) noexcept {
		core::SmartReference<IResourcePostEffectShader> tRet;
		if (!(tRet = m_StageResourcePool.GetFX(name)))
			tRet = m_GlobalResourcePool.GetFX(name);
		return tRet;
	}

	core::SmartReference<IResourceModel> ResourceMgr::FindModel(const char* name) noexcept
	{
		core::SmartReference<IResourceModel> tRet;
		if (!(tRet = m_StageResourcePool.GetModel(name)))
			tRet = m_GlobalResourcePool.GetModel(name);
		return tRet;
	}

	core::SmartReference<IResourceSpineAtlas> ResourceMgr::FindSpineAtlas(const char* name) noexcept
	{
		core::SmartReference<IResourceSpineAtlas> tRet;
		if (!(tRet = m_StageResourcePool.GetSpineAtlas(name)))
			tRet = m_GlobalResourcePool.GetSpineAtlas(name);
		return tRet;
	}

	core::SmartReference<IResourceSpineSkeleton> ResourceMgr::FindSpineSkeleton(const char* name) noexcept
	{
		core::SmartReference<IResourceSpineSkeleton> tRet;
		if (!(tRet = m_StageResourcePool.GetSpineSkeleton(name)))
			tRet = m_GlobalResourcePool.GetSpineSkeleton(name);
		return tRet;
	}

	// 其他资源操作

	bool ResourceMgr::GetTextureSize(const char* name, core::Vector2U& out) noexcept {
		core::SmartReference<IResourceTexture> tRet = FindTexture(name);
		if (!tRet)
			return false;
		out = tRet->GetTexture()->getSize();
		return true;
	}

	void ResourceMgr::CacheTTFFontString(const char* name, const char* text, size_t len) noexcept {
		core::SmartReference<IResourceFont> f = FindTTFFont(name);
		if (f)
			f->GetGlyphManager()->cacheString(core::StringView(text, len));
		else
			spdlog::error("[luastg] CacheTTFFontString: 缓存字形时未找到指定字体'{}'", name);
	}

	void ResourceMgr::UpdateSound()
	{
		for (auto& snd : m_GlobalResourcePool.m_SoundSpritePool)
		{
			snd.second->FlushCommand();
		}
		for (auto& snd : m_StageResourcePool.m_SoundSpritePool)
		{
			snd.second->FlushCommand();
		}
	}

	// 其他

	#ifdef LDEVVERSION
	bool ResourceMgr::g_ResourceLoadingLog = true;
	#else
	bool ResourceMgr::g_ResourceLoadingLog = false;
	#endif

	void ResourceMgr::SetResourceLoadingLog(bool b) { g_ResourceLoadingLog = b; }

	bool ResourceMgr::GetResourceLoadingLog() { return g_ResourceLoadingLog; }
}
