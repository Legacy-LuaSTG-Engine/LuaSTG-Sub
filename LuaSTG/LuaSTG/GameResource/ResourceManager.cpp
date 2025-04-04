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

	Core::ScopeObject<IResourceTexture> ResourceMgr::FindTexture(const char* name) noexcept {

		Core::ScopeObject<IResourceTexture> tRet;
		if (!(tRet = m_StageResourcePool.GetTexture(name)))
			tRet = m_GlobalResourcePool.GetTexture(name);
		return tRet;
	}

	Core::ScopeObject<IResourceSprite> ResourceMgr::FindSprite(const char* name) noexcept {
		Core::ScopeObject<IResourceSprite> tRet;
		if (!(tRet = m_StageResourcePool.GetSprite(name)))
			tRet = m_GlobalResourcePool.GetSprite(name);
		return tRet;
	}

	Core::ScopeObject<IResourceAnimation> ResourceMgr::FindAnimation(const char* name) noexcept {
		Core::ScopeObject<IResourceAnimation> tRet;
		if (!(tRet = m_StageResourcePool.GetAnimation(name)))
			tRet = m_GlobalResourcePool.GetAnimation(name);
		return tRet;
	}

	Core::ScopeObject<IResourceMusic> ResourceMgr::FindMusic(const char* name) noexcept {
		Core::ScopeObject<IResourceMusic> tRet;
		if (!(tRet = m_StageResourcePool.GetMusic(name)))
			tRet = m_GlobalResourcePool.GetMusic(name);
		return tRet;
	}

	Core::ScopeObject<IResourceSoundEffect> ResourceMgr::FindSound(const char* name) noexcept {
		Core::ScopeObject<IResourceSoundEffect> tRet;
		if (!(tRet = m_StageResourcePool.GetSound(name)))
			tRet = m_GlobalResourcePool.GetSound(name);
		return tRet;
	}

	Core::ScopeObject<IResourceParticle> ResourceMgr::FindParticle(const char* name) noexcept {
		Core::ScopeObject<IResourceParticle> tRet;
		if (!(tRet = m_StageResourcePool.GetParticle(name)))
			tRet = m_GlobalResourcePool.GetParticle(name);
		return tRet;
	}

	Core::ScopeObject<IResourceFont> ResourceMgr::FindSpriteFont(const char* name) noexcept {
		Core::ScopeObject<IResourceFont> tRet;
		if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
			tRet = m_GlobalResourcePool.GetSpriteFont(name);
		return tRet;
	}

	Core::ScopeObject<IResourceFont> ResourceMgr::FindTTFFont(const char* name) noexcept {
		Core::ScopeObject<IResourceFont> tRet;
		if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
			tRet = m_GlobalResourcePool.GetTTFFont(name);
		return tRet;
	}

	Core::ScopeObject<IResourcePostEffectShader> ResourceMgr::FindFX(const char* name) noexcept {
		Core::ScopeObject<IResourcePostEffectShader> tRet;
		if (!(tRet = m_StageResourcePool.GetFX(name)))
			tRet = m_GlobalResourcePool.GetFX(name);
		return tRet;
	}

	Core::ScopeObject<IResourceModel> ResourceMgr::FindModel(const char* name) noexcept
	{
		Core::ScopeObject<IResourceModel> tRet;
		if (!(tRet = m_StageResourcePool.GetModel(name)))
			tRet = m_GlobalResourcePool.GetModel(name);
		return tRet;
	}

	// 其他资源操作

	bool ResourceMgr::GetTextureSize(const char* name, Core::Vector2U& out) noexcept {
		Core::ScopeObject<IResourceTexture> tRet = FindTexture(name);
		if (!tRet)
			return false;
		out = tRet->GetTexture()->getSize();
		return true;
	}

	void ResourceMgr::CacheTTFFontString(const char* name, const char* text, size_t len) noexcept {
		Core::ScopeObject<IResourceFont> f = FindTTFFont(name);
		if (f)
			f->GetGlyphManager()->cacheString(Core::StringView(text, len));
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
