#include "ResourceMgr.h"

#include <string>
#include <string_view>
#include <filesystem>

#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "utility/path.hpp"

#include "AppFrame.h"

using namespace std;
using namespace LuaSTGPlus;

ResourceMgr::ResourceMgr() :
    m_GlobalResourcePool(this, ResourcePoolType::Global),
    m_StageResourcePool(this, ResourcePoolType::Stage) {
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

fcyRefPointer<ResTexture> ResourceMgr::FindTexture(const char* name) noexcept {
    fcyRefPointer<ResTexture> tRet;
    if (!(tRet = m_StageResourcePool.GetTexture(name)))
        tRet = m_GlobalResourcePool.GetTexture(name);
    return tRet;
}

fcyRefPointer<ResSprite> ResourceMgr::FindSprite(const char* name) noexcept {
    fcyRefPointer<ResSprite> tRet;
    if (!(tRet = m_StageResourcePool.GetSprite(name)))
        tRet = m_GlobalResourcePool.GetSprite(name);
    return tRet;
}

fcyRefPointer<ResAnimation> ResourceMgr::FindAnimation(const char* name) noexcept {
    fcyRefPointer<ResAnimation> tRet;
    if (!(tRet = m_StageResourcePool.GetAnimation(name)))
        tRet = m_GlobalResourcePool.GetAnimation(name);
    return tRet;
}

fcyRefPointer<ResMusic> ResourceMgr::FindMusic(const char* name) noexcept {
    fcyRefPointer<ResMusic> tRet;
    if (!(tRet = m_StageResourcePool.GetMusic(name)))
        tRet = m_GlobalResourcePool.GetMusic(name);
    return tRet;
}

fcyRefPointer<ResSound> ResourceMgr::FindSound(const char* name) noexcept {
    fcyRefPointer<ResSound> tRet;
    if (!(tRet = m_StageResourcePool.GetSound(name)))
        tRet = m_GlobalResourcePool.GetSound(name);
    return tRet;
}

fcyRefPointer<ResParticle> ResourceMgr::FindParticle(const char* name) noexcept {
    fcyRefPointer<ResParticle> tRet;
    if (!(tRet = m_StageResourcePool.GetParticle(name)))
        tRet = m_GlobalResourcePool.GetParticle(name);
    return tRet;
}

fcyRefPointer<ResFont> ResourceMgr::FindSpriteFont(const char* name) noexcept {
    fcyRefPointer<ResFont> tRet;
    if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
        tRet = m_GlobalResourcePool.GetSpriteFont(name);
    return tRet;
}

fcyRefPointer<ResFont> ResourceMgr::FindTTFFont(const char* name) noexcept {
    fcyRefPointer<ResFont> tRet;
    if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
        tRet = m_GlobalResourcePool.GetTTFFont(name);
    return tRet;
}

fcyRefPointer<ResFX> ResourceMgr::FindFX(const char* name) noexcept {
    fcyRefPointer<ResFX> tRet;
    if (!(tRet = m_StageResourcePool.GetFX(name)))
        tRet = m_GlobalResourcePool.GetFX(name);
    return tRet;
}

// 其他资源操作

bool ResourceMgr::GetTextureSize(const char* name, fcyVec2& out) noexcept {
    fcyRefPointer<ResTexture> tRet = FindTexture(name);
    if (!tRet)
        return false;
    out.x = static_cast<float>(tRet->GetTexture()->GetWidth());
    out.y = static_cast<float>(tRet->GetTexture()->GetHeight());
    return true;
}

void ResourceMgr::CacheTTFFontString(const char* name, const char* text, size_t len) noexcept {
    fcyRefPointer<ResFont> f = FindTTFFont(name);
    if (f)
        f->GetFontProvider()->CacheStringU8(text, len);
    else
        spdlog::error("[luastg] CacheTTFFontString: 缓存字形时未找到指定字体'{}'", name);
}

// 其他

#ifdef LDEVVERSION
bool ResourceMgr::g_ResourceLoadingLog = true;
#else
bool ResourceMgr::g_ResourceLoadingLog = false;
#endif

void ResourceMgr::SetResourceLoadingLog(bool b) { g_ResourceLoadingLog = b; }

bool ResourceMgr::GetResourceLoadingLog() { return g_ResourceLoadingLog; }
