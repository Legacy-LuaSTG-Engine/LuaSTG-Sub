#include "ResourceMgr.h"
#include "AppFrame.h"
#include "Utility.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define LSOUNDGLOBALFOCUS true // 谁会希望窗口失去焦点就没声音呢？

using namespace LuaSTGPlus;

// 总体管理

void ResourcePool::Clear() noexcept {
    m_TexturePool.clear();
    m_SpritePool.clear();
    m_AnimationPool.clear();
    m_MusicPool.clear();
    m_SoundSpritePool.clear();
    m_ParticlePool.clear();
    m_SpriteFontPool.clear();
    m_TTFFontPool.clear();
    m_FXPool.clear();
    spdlog::info("[luastg] {}已清空", getResourcePoolTypeName());
}

template<typename T>
void removeResource(Dictionary<fcyRefPointer<T>>& pool, const char* name) {
    auto i = pool.find(name);
    if (i == pool.end()) {
        spdlog::warn("[luastg] RemoveResource: 试图卸载一个不存在的资源'{}'", name);
        return;
    }
    pool.erase(i);
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] RemoveResource: 资源'{}'已卸载", name);
    }
}

const char* ResourcePool::getResourcePoolTypeName() {
    switch (m_iType) {
        case ResourcePoolType::Global:
            return "全局资源池";
        case ResourcePoolType::Stage:
            return "关卡资源池";
        default:
            return "未知资源池";
    }
}

void ResourcePool::RemoveResource(ResourceType t, const char* name) noexcept {
    switch (t) {
        case ResourceType::Texture:
            removeResource(m_TexturePool, name);
            break;
        case ResourceType::Sprite:
            removeResource(m_SpritePool, name);
            break;
        case ResourceType::Animation:
            removeResource(m_AnimationPool, name);
            break;
        case ResourceType::Music:
            removeResource(m_MusicPool, name);
            break;
        case ResourceType::SoundEffect:
            removeResource(m_SoundSpritePool, name);
            break;
        case ResourceType::Particle:
            removeResource(m_ParticlePool, name);
            break;
        case ResourceType::SpriteFont:
            removeResource(m_SpriteFontPool, name);
            break;
        case ResourceType::TrueTypeFont:
            removeResource(m_TTFFontPool, name);
            break;
        case ResourceType::FX:
            removeResource(m_FXPool, name);
            break;
        default:
            return;
    }
}

bool ResourcePool::CheckResourceExists(ResourceType t, const std::string& name) const noexcept {
    switch (t) {
        case ResourceType::Texture:
            return m_TexturePool.find(name.c_str()) != m_TexturePool.end();
        case ResourceType::Sprite:
            return m_SpritePool.find(name.c_str()) != m_SpritePool.end();
        case ResourceType::Animation:
            return m_AnimationPool.find(name.c_str()) != m_AnimationPool.end();
        case ResourceType::Music:
            return m_MusicPool.find(name.c_str()) != m_MusicPool.end();
        case ResourceType::SoundEffect:
            return m_SoundSpritePool.find(name.c_str()) != m_SoundSpritePool.end();
        case ResourceType::Particle:
            return m_ParticlePool.find(name.c_str()) != m_ParticlePool.end();
        case ResourceType::SpriteFont:
            return m_SpriteFontPool.find(name.c_str()) != m_SpriteFontPool.end();
        case ResourceType::TrueTypeFont:
            return m_TTFFontPool.find(name.c_str()) != m_TTFFontPool.end();
        case ResourceType::FX:
            return m_FXPool.find(name.c_str()) != m_FXPool.end();
        default:
            spdlog::warn("[luastg] CheckRes: 试图检索一个不存在的资源类型({})", (int)t);
            break;
    }
    return false;
}

int ResourcePool::ExportResourceList(lua_State* L, ResourceType t) const noexcept {
    int cnt = 1;
    switch (t) {
        case ResourceType::Texture:
            lua_createtable(L, (int) m_TexturePool.size(), 0);  // t
            for (auto& i : m_TexturePool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::Sprite:
            lua_createtable(L, (int) m_SpritePool.size(), 0);  // t
            for (auto& i : m_SpritePool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::Animation:
            lua_createtable(L, (int) m_AnimationPool.size(), 0);  // t
            for (auto& i : m_AnimationPool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::Music:
            lua_createtable(L, (int) m_MusicPool.size(), 0);  // t
            for (auto& i : m_MusicPool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::SoundEffect:
            lua_createtable(L, (int) m_SoundSpritePool.size(), 0);  // t
            for (auto& i : m_SoundSpritePool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::Particle:
            lua_createtable(L, (int) m_ParticlePool.size(), 0);  // t
            for (auto& i : m_ParticlePool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::SpriteFont:
            lua_createtable(L, (int) m_SpriteFontPool.size(), 0);  // t
            for (auto& i : m_SpriteFontPool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::TrueTypeFont:
            lua_createtable(L, (int) m_TTFFontPool.size(), 0);  // t
            for (auto& i : m_TTFFontPool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        case ResourceType::FX:
            lua_createtable(L, (int) m_FXPool.size(), 0);  // t
            for (auto& i : m_FXPool) {
                lua_pushstring(L, i.second->GetResName().c_str());  // t s
                lua_rawseti(L, -2, cnt++);  // t
            }
            break;
        default:
            spdlog::warn("[luastg] EnumRes: 试图枚举一个不存在的资源类型({})", (int)t);
            // lua_pushnil(L);
            lua_createtable(L, 0, 0);
            break;
    }
    return 1;
}

// 加载纹理

bool ResourcePool::LoadTexture(const char* name, const char* path, bool mipmaps) noexcept {
    if (!LAPP.GetRenderDev()) {
        spdlog::error("[luastg] LoadTexture: 无法加载纹理'{}'，f2dRenderDevice未准备好", name);
        return false;
    }
    
    if (m_TexturePool.find(name) != m_TexturePool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadTexture: 纹理'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!m_pMgr->LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadTexture: 无法从'{}'加载纹理'{}'，读取文件失败", path, name);
        return false;
    }
    
    fcyRefPointer<f2dTexture2D> tTexture;
    fResult fr = LAPP.GetRenderDev()->CreateTextureFromMemory((fcData) tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(),
                                                        0, 0, false, mipmaps, &tTexture);
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderDevice::CreateTextureFromMemory] 从'{}'创建纹理'{}'失败(fResult={})", path, name, fr);
        return false;
    }
    
    try {
        fcyRefPointer<ResTexture> tRes;
        tRes.DirectSet(new ResTexture(name, tTexture));
        m_TexturePool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadTexture: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadTexture: 已从'{}'加载纹理'{}' ({})", path, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 创建渲染目标

bool ResourcePool::CreateRenderTarget(const char* name, int width, int height) noexcept {
    if (!LAPP.GetRenderDev()) {
        spdlog::error("[luastg] CreateRenderTarget: 无法创建渲染目标'{}'，f2dRenderDevice未准备好", name);
        return false;
    }
    
    if (m_TexturePool.find(name) != m_TexturePool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] CreateRenderTarget: 渲染目标'{}'已存在，创建操作已取消", name);
        }
        return true;
    }
    
    fResult fr = 0;
    fcyRefPointer<f2dTexture2D> tTexture;
    if (width <= 0 || height <= 0) {
        fr = LAPP.GetRenderDev()->CreateRenderTarget(
            LAPP.GetRenderDev()->GetBufferWidth(), LAPP.GetRenderDev()->GetBufferHeight(),
            true, &tTexture);
    }
    else {
        fr = LAPP.GetRenderDev()->CreateRenderTarget(
            (fuInt) width, (fuInt) height,
            false, &tTexture);
    }
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderDevice::CreateRenderTarget] 创建渲染目标'{}'失败(fResult={})", name, fr);
        return false;
    }
    fcyRefPointer<f2dDepthStencilSurface> tDepthStencil;
    if (width > 0 && height > 0) {
        fr = LAPP.GetRenderDev()->CreateDepthStencilSurface(
            (fuInt)width, (fuInt)height,
            false, false, &tDepthStencil);
    }
    else { fr = FCYERR_OK; }
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderDevice::CreateDepthStencilSurface] 创建渲染目标附带的深度模板缓冲区'{}'失败(fResult={})", name, fr);
        return false;
    }

    try {
        fcyRefPointer<ResTexture> tRes;
        tRes.DirectSet(new ResTexture(name, tTexture, tDepthStencil));
        m_TexturePool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] CreateRenderTarget: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        if (width <= 0 || height <= 0) {
            spdlog::info("[luastg] CreateRenderTarget: 已创建渲染目标'{}' ({})", name, getResourcePoolTypeName());
        }
        else {
            spdlog::info("[luastg] CreateRenderTarget: 已创建渲染目标'{}'({}x{}) ({})", name, width, height, getResourcePoolTypeName());
        }
    }
    
    return true;
}

// 创建图片精灵

bool ResourcePool::CreateSprite(const char* name, const char* texname,
                                double x, double y, double w, double h,
                                double a, double b, bool rect) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] CreateSprite: 无法创建图片精灵'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_SpritePool.find(name) != m_SpritePool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] CreateSprite: 图片精灵'{}'已存在，创建操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
    if (!pTex) {
        spdlog::error("[luastg] CreateSprite: 无法创建图片精灵'{}'，无法找到纹理'{}'", name, texname);
        return false;
    }
    
    fcyRefPointer<f2dSprite> pSprite;
    fcyRect tRect((float) x, (float) y, (float) (x + w), (float) (y + h));
    fResult fr = LAPP.GetRenderer()->CreateSprite2D(pTex->GetTexture(), tRect, &pSprite);
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderer::CreateSprite2D] 从'{}'创建图片精灵'{}'失败(fResult={})", texname, name, fr);
        return false;
    }
    
    try {
        fcyRefPointer<ResSprite> tRes;
        tRes.DirectSet(new ResSprite(name, pSprite, a, b, rect));
        m_SpritePool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] CreateSprite: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] CreateSprite: 已从'{}'创建图片精灵'{}' ({})", texname, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 创建动画精灵

bool ResourcePool::CreateAnimation(const char* name, const char* texname,
                                   double x, double y, double w, double h, int n, int m, int intv,
                                   double a, double b, bool rect) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] CreateAnimation: 无法创建动画精灵'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_AnimationPool.find(name) != m_AnimationPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] CreateAnimation: 动画精灵'{}'已存在，创建操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
    if (!pTex) {
        spdlog::error("[luastg] CreateAnimation: 无法创建动画精灵'{}'，无法找到纹理'{}'", name, texname);
        return false;
    }
    
    try {
        fcyRefPointer<ResAnimation> tRes;
        tRes.DirectSet(
            new ResAnimation(name, pTex, (float) x, (float) y, (float) w, (float) h, n, m, intv, a, b, rect));
        m_AnimationPool.emplace(name, tRes);
    }
    catch (const fcyException&) {
        spdlog::error("[luastg] CreateAnimation: 无法创建动画精灵'{}'，内部错误", name);
        return false;
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] CreateAnimation: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] CreateAnimation: 已从'{}'创建动画精灵'{}' ({})", texname, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载音乐

bool ResourcePool::LoadMusic(const char* name, const char* path, double start, double end) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadMusic: 无法加载音乐'{}'，f2dSoundSys未准备好", name);
        return false;
    }
    
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!m_pMgr->LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadMusic: 无法从'{}'加载音乐'{}'，读取文件失败", path, name);
        return false;
    }
    
    try {
        //加载解码器，优先OGG解码器，加载失败则使用WAV解码器，都失败则error糊脸
        fcyRefPointer<f2dSoundDecoder> tDecoder;
        if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder))) {
            tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
            if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder))) {
                spdlog::error("[luastg] LoadMusic: 无法解码文件'{}'，要求文件格式为WAV或OGG", path);
                return false;
            }
        }
        
        //配置循环解码器
        fcyRefPointer<ResMusic::BGMWrapper> tWrapperedBuffer;
        tWrapperedBuffer.DirectSet(new ResMusic::BGMWrapper(tDecoder, start, end));
        
        //加载音频缓冲曲，动态缓冲区
        fcyRefPointer<f2dSoundBuffer> tBuffer;
        fResult fr = LAPP.GetSoundSys()->CreateDynamicBuffer(tWrapperedBuffer, LSOUNDGLOBALFOCUS, &tBuffer);
        if (FCYFAILED(fr)) {
            spdlog::error("[fancy2d] [f2dSoundSys::CreateDynamicBuffer] 无法从'{}'创建音频流'{}'(fResult={})", path, name, fr);
            return false;
        }
        
        //存入资源池
        fcyRefPointer<ResMusic> tRes;
        tRes.DirectSet(new ResMusic(name, tWrapperedBuffer, tBuffer));
        m_MusicPool.emplace(name, tRes);
    }
    catch (const fcyException& e) {
        spdlog::error("[fancy2d] [{}] 无法从'{}'创建音频流'{}'：{}", e.GetSrc(), path, name, e.GetDesc());
        return false;
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadMusic: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadMusic: 已从'{}'加载音乐'{}' ({})", path, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载音效

bool ResourcePool::LoadSoundEffect(const char* name, const char* path) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadSoundEffect: 无法加载音效'{}'，f2dSoundSys未准备好", name);
        return false;
    }
    
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!m_pMgr->LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadSoundEffect: 无法从'{}'加载音效'{}'，读取文件失败", path, name);
        return false;
    }
    
    try {
        fcyRefPointer<f2dSoundDecoder> tDecoder;
        if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder))) {
            tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
            if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder))) {
                spdlog::error("[luastg] LoadSoundEffect: 无法解码文件'{}'，要求文件格式为WAV或OGG", path);
                return false;
            }
        }
        
        fcyRefPointer<f2dSoundBuffer> tBuffer;
        fResult fr = LAPP.GetSoundSys()->CreateStaticBuffer(tDecoder, LSOUNDGLOBALFOCUS, &tBuffer);
        if (FCYFAILED(fr)) {
            spdlog::error("[fancy2d] [f2dSoundSys::CreateStaticBuffer] 无法从'{}'创建音频流'{}'(fResult={})", path, name, fr);
            return false;
        }
        
        fcyRefPointer<ResSound> tRes;
        tRes.DirectSet(new ResSound(name, tBuffer));
        m_SoundSpritePool.emplace(name, tRes);
    }
    catch (const fcyException& e) {
        spdlog::error("[fancy2d] [{}] 无法从'{}'创建音频流'{}'：{}", e.GetSrc(), path, name, e.GetDesc());
        return false;
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSoundEffect: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadSoundEffect: 已从'{}'加载音效'{}' ({})", path, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 创建粒子特效

bool ResourcePool::LoadParticle(const char* name, const ResParticle::ParticleInfo& info, const char* img_name,
                                double a,double b, bool rect, bool _nolog) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadParticle: 无法创建粒子特效'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_ParticlePool.find(name) != m_ParticlePool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadParticle: 粒子特效'{}'已存在，创建操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<ResSprite> pSprite = m_pMgr->FindSprite(img_name);
    if (!pSprite) {
        spdlog::error("[luastg] LoadParticle: 无法创建粒子特效'{}'，找不到图片精灵'{}'", name, img_name);
        return false;
    }
    
    fcyRefPointer<f2dSprite> pClone;
    if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pSprite->GetSprite()->GetTexture(),
                                                     pSprite->GetSprite()->GetTexRect(),
                                                     pSprite->GetSprite()->GetHotSpot(),
                                                     &pClone))) {
        spdlog::error("[luastg] LoadParticle: 无法创建粒子特效'{}'，复制图片精灵'{}'失败", name, img_name);
        return false;
    }
    pClone->SetColor(0, pSprite->GetSprite()->GetColor(0U));
    pClone->SetColor(1, pSprite->GetSprite()->GetColor(1U));
    pClone->SetColor(2, pSprite->GetSprite()->GetColor(2U));
    pClone->SetColor(3, pSprite->GetSprite()->GetColor(3U));
    pClone->SetZ(pSprite->GetSprite()->GetZ());
    
    try {
        ResParticle::ParticleInfo tInfo = info;
        tInfo.iBlendInfo = (tInfo.iBlendInfo >> 16) & 0x00000003;
        
        BlendMode tBlendInfo = BlendMode::AddAlpha;
        if (tInfo.iBlendInfo & 1)  // ADD
        {
            if (tInfo.iBlendInfo & 2)  // ALPHA
                tBlendInfo = BlendMode::AddAlpha;
            else
                tBlendInfo = BlendMode::AddAdd;
        }
        else  // MUL
        {
            if (tInfo.iBlendInfo & 2)  // ALPHA
                tBlendInfo = BlendMode::MulAlpha;
            else
                tBlendInfo = BlendMode::MulAdd;
        }
        
        fcyRefPointer<ResParticle> tRes;
        tRes.DirectSet(new ResParticle(name, tInfo, pClone, tBlendInfo, a, b, rect));
        m_ParticlePool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadParticle: 内存不足");
        return false;
    }
    
    if (!_nolog && ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadParticle: 已创建粒子特效'{}' ({})", name, getResourcePoolTypeName());
    }
    
    return true;
}

bool ResourcePool::LoadParticle(const char* name, const char* path, const char* img_name,
                                double a, double b,bool rect) noexcept {
    fcyRefPointer<fcyMemStream> outBuf;
    if (!LRES.LoadFile(path, outBuf)) {
        spdlog::error("[luastg] LoadParticle: 无法从'{}'创建粒子特效'{}'，读取文件失败", path, name);
        return false;
    }
    
    if (outBuf->GetLength() != sizeof(ResParticle::ParticleInfo)) {
        spdlog::error("[luastg] LoadParticle: 粒子特效定义文件'{}'格式不正确", path);
        return false;
    }
    ResParticle::ParticleInfo tInfo;
    memcpy(&tInfo, outBuf->GetInternalBuffer(), sizeof(ResParticle::ParticleInfo));
    
    if (!LoadParticle(name, tInfo, img_name, a, b, rect, /* _nolog */ true)) {
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadParticle: 已从'{}'创建粒子特效'{}' ({})", path, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载纹理字体（HGE）

bool ResourcePool::LoadSpriteFont(const char* name, const char* path, bool mipmaps) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadSpriteFont: 无法加载纹理字体'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadSpriteFont: 纹理字体'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!LRES.LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadSpriteFont: 无法从'{}'加载纹理字体'{}'，读取文件失败", path, name);
        return false;
    }
    
    // 转换编码
    std::wstring tFileData;
    try {
        if (tDataBuf->GetLength() > 0) {
            // stupid
            tFileData = fcyStringHelper::MultiByteToWideChar(
                std::string((const char*) tDataBuf->GetInternalBuffer(), (size_t) tDataBuf->GetLength())
            );
        }
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    // 读取HGE字体定义
    std::unordered_map<wchar_t, f2dGlyphInfo> tOutputCharset;
    std::string texpath;
    try {
        std::wstring tOutputTextureName;
        ResFont::HGEFont::ReadDefine(tFileData, tOutputCharset, tOutputTextureName);
        texpath = fcyPathParser::GetPath(path) + fcyStringHelper::WideCharToMultiByte(tOutputTextureName);
    }
    catch (const fcyException& e) {
        spdlog::error("[luastg] [{}] 无法从'{}'加载纹理字体'{}'：{}", e.GetSrc(), path, name, e.GetDesc());
        return false;
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    // 装载纹理
    try {
        if (!m_pMgr->LoadFile(texpath.c_str(), tDataBuf)) {
            spdlog::error("[luastg] LoadSpriteFont: 无法从'{}'加载纹理字体'{}'，读取文件失败", texpath, name);
            return false;
        }
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    fcyRefPointer<f2dTexture2D> tTexture;
    fResult fr = LAPP.GetRenderDev()->CreateTextureFromMemory(
        (fcData) tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture);
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderDevice::CreateTextureFromMemory] 从'{}'创建纹理'{}'失败(fResult={})", texpath, name, fr);
        return false;
    }
    
    // 创建定义
    try {
        fcyRefPointer<f2dFontProvider> tFontProvider;
        tFontProvider.DirectSet(new ResFont::HGEFont(std::move(tOutputCharset), tTexture));
        
        fcyRefPointer<ResFont> tRes;
        tRes.DirectSet(new ResFont(name, tFontProvider));
        m_SpriteFontPool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadSpriteFont: 已从'{}'和'{}'加载纹理字体'{}' ({})", path, texpath, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载纹理字体（fancy2d）

bool ResourcePool::LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadSpriteFont: 无法加载纹理字体'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadSpriteFont: 纹理字体'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    // 加载字体定义文件
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!LRES.LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadSpriteFont: 无法从'{}'加载纹理字体'{}'，读取文件失败", path, name);
        return false;
    }
    
    // 转换编码
    std::wstring tFileData;
    try {
        if (tDataBuf->GetLength() > 0) {
            // stupid
            tFileData = fcyStringHelper::MultiByteToWideChar(
                std::string((const char*) tDataBuf->GetInternalBuffer(), (size_t) tDataBuf->GetLength())
            );
        }
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    // 加载纹理文件
    try {
        const std::string fulltexpath = fcyPathParser::GetPath(path) + tex_path;
        if (!m_pMgr->LoadFile(fulltexpath.c_str(), tDataBuf)) {
            if (!m_pMgr->LoadFile(tex_path, tDataBuf)) {
                spdlog::error("[luastg] LoadSpriteFont: 无法从'{}'或'{}'加载纹理字体'{}'，读取文件失败", fulltexpath, tex_path, name);
                return false;
            }
        }
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    fResult fr = 0;
    
    fcyRefPointer<f2dTexture2D> tTexture;
    fr = LAPP.GetRenderDev()->CreateTextureFromMemory(
        (fcData) tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture);
    if (FCYFAILED(fr)) {
        spdlog::error("[fancy2d] [f2dRenderDevice::CreateTextureFromMemory] 从'{}'创建纹理'{}'失败(fResult={})", tex_path, name, fr);
        return false;
    }
    
    // 创建定义
    try {
        fcyRefPointer<f2dFontProvider> tFontProvider;
        fr = LAPP.GetRenderer()->CreateFontFromTex(tFileData.c_str(), tTexture, &tFontProvider);
        if (FCYFAILED(fr)) {
            spdlog::error("[fancy2d] [f2dRenderer::CreateFontFromTex] 创建纹理字体'{}'失败(fResult={})", name, fr);
            return false;
        }
        
        fcyRefPointer<ResFont> tRes;
        tRes.DirectSet(new ResFont(name, tFontProvider));
        m_SpriteFontPool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadSpriteFont: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadSpriteFont: 已从'{}'和'{}'加载纹理字体'{}' ({})", path, tex_path, name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载TrueType字体

bool ResourcePool::LoadTTFFont(const char* name, const char* path,
                               float width, float height, float bboxwidth,float bboxheight) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadTTFFont: 无法加载矢量字体'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_TTFFontPool.find(name) != m_TTFFontPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadTTFFont: 矢量字体'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<f2dFontProvider> tFontProvider;
    
    // 读取文件
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!m_pMgr->LoadFile(path, tDataBuf)) {
        spdlog::warn("[luastg] LoadTTFFont: 无法从'{}'加载矢量字体，文件不存在，尝试从系统字体库加载字体", path);
        try {
            const std::wstring wpath = fcyStringHelper::MultiByteToWideChar(path);
            if (FCYFAILED(LAPP.GetRenderer()->CreateSystemFont(
                wpath.c_str(), 0, fcyVec2(width, height), F2DFONTFLAG_NONE, &tFontProvider))) {
                spdlog::error("[luastg] LoadTTFFont: 尝试失败，无法从系统字体库加载字体'{}'", path);//向lua层返回错误，而不是直接崩游戏
                return false;
            }
        }
        catch (const std::bad_alloc&) {
            spdlog::error("[luastg] LoadTTFFont: 内存不足");
            return false;
        }
    }
    
    // 创建定义
    try {
        if (!tFontProvider) {
            fResult fr = LAPP.GetRenderer()->CreateFontFromMemory(tDataBuf, 0, fcyVec2(width, height),
                                                                  fcyVec2(bboxwidth, bboxheight), F2DFONTFLAG_NONE,
                                                                  &tFontProvider);
            if (FCYFAILED(fr)) {
                spdlog::error("[fancy2d] [f2dRenderer::CreateFontFromMemory] 从'{}'创建矢量字体'{}'失败(fResult={})", path, name, fr);
                return false;
            }
        }
        
        fcyRefPointer<ResFont> tRes;
        tRes.DirectSet(new ResFont(name, tFontProvider));
        tRes->SetBlendMode(BlendMode::MulAlpha);
        m_TTFFontPool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadTTFFont: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadTTFFont: 字形缓存数量：{}，字形缓存贴图大小：({}x{})",
            tFontProvider->GetCacheCount(),
            tFontProvider->GetCacheTexSize(),
            tFontProvider->GetCacheTexSize()
        );
        spdlog::info("[luastg] LoadTTFFont: 已从'{}'加载矢量字体'{}' ({})", path, name, getResourcePoolTypeName());
    }
    
    return true;
}

bool ResourcePool::LoadTTFFont(const char* name, fcyStream* stream, float width, float height,
                               float bboxwidth,float bboxheight) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadTTFFont: 无法加载矢量字体'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_TTFFontPool.find(name) != m_TTFFontPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadTTFFont: 矢量字体'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    // 创建定义
    fcyRefPointer<f2dFontProvider> tFontProvider;
    try {
        fResult fr = LAPP.GetRenderer()->CreateFontFromMemory((fcyMemStream*) stream, 0, fcyVec2(width, height),
                                                            fcyVec2(bboxwidth, bboxheight), F2DFONTFLAG_NONE,
                                                            &tFontProvider);
        if (FCYFAILED(fr)) {
            spdlog::error("[fancy2d] [f2dRenderer::CreateFontFromMemory] 创建矢量字体'{}'失败(fResult={})", name, fr);
            return false;
        }
        
        fcyRefPointer<ResFont> tRes;
        tRes.DirectSet(new ResFont(name, tFontProvider));
        tRes->SetBlendMode(BlendMode::MulAlpha);
        m_TTFFontPool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadTTFFont: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadTTFFont: 字形缓存数量：{}，字形缓存贴图大小：({}x{})",
            tFontProvider->GetCacheCount(),
            tFontProvider->GetCacheTexSize(),
            tFontProvider->GetCacheTexSize()
        );
        spdlog::info("[luastg] LoadTTFFont: 已加载矢量字体'{}' ({})", name, getResourcePoolTypeName());
    }
    
    return true;
}

bool ResourcePool::LoadTrueTypeFont(const char* name,
                                    f2dFontProviderParam param, f2dTrueTypeFontParam* fonts,fuInt count) noexcept {
    if (!LAPP.GetRenderer()) {
        spdlog::error("[luastg] LoadTrueTypeFont: 无法加载矢量字体组'{}'，f2dRenderer未准备好", name);
        return false;
    }
    
    if (m_TTFFontPool.find(name) != m_TTFFontPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadTrueTypeFont: 矢量字体组'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    // 创建定义
    fcyRefPointer<f2dFontProvider> tFontProvider;
    try {
        fResult fr = LAPP.GetRenderer()->CreateFontFromMemory(param, fonts, count, &tFontProvider);
        if (FCYFAILED(fr)) {
            spdlog::error("[fancy2d] [f2dRenderer::CreateFontFromMemory] 创建矢量字体组'{}'失败(fResult={})", name, fr);
            return false;
        }
        
        fcyRefPointer<ResFont> tRes;
        tRes.DirectSet(new ResFont(name, tFontProvider));
        tRes->SetBlendMode(BlendMode::MulAlpha);
        m_TTFFontPool.emplace(name, tRes);
    }
    catch (const std::bad_alloc&) {
        spdlog::error("[luastg] LoadTTFFont: 内存不足");
        return false;
    }
    
    if (ResourceMgr::GetResourceLoadingLog()) {
        spdlog::info("[luastg] LoadTrueTypeFont: 字形缓存数量：{}，字形缓存贴图大小：({}x{})",
            tFontProvider->GetCacheCount(),
            tFontProvider->GetCacheTexSize(),
            tFontProvider->GetCacheTexSize()
        );
        spdlog::info("[luastg] LoadTrueTypeFont: 已加载矢量字体组'{}' ({})", name, getResourcePoolTypeName());
    }
    
    return true;
}

// 加载后处理特效

bool ResourcePool::LoadFX(const char* name, const char* path, bool is_effect) noexcept {
    if (!LAPP.GetRenderDev()) {
        spdlog::error("[luastg] LoadFX: 无法加载后处理特效'{}'，f2dRenderDevice未准备好", name);
        return false;
    }
    
    if (m_FXPool.find(name) != m_FXPool.end()) {
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::warn("[luastg] LoadFX: 后处理特效'{}'已存在，加载操作已取消", name);
        }
        return true;
    }
    
    fcyRefPointer<fcyMemStream> tDataBuf;
    if (!m_pMgr->LoadFile(path, tDataBuf)) {
        spdlog::error("[luastg] LoadFX: 无法从'{}'加载后处理特效'{}'，读取文件失败", path, name);
        return false;
    }
    
    if (!is_effect) // 不是 d3d9 的 effect9 文件
    {
        LuaSTG::Core::ShaderID shader = LAPP.GetRenderer2D().createPostEffectShader(name, tDataBuf->GetInternalBuffer(), (SIZE_T)tDataBuf->GetLength());
        if (!shader.handle)
        {
            spdlog::error("[luastg] LoadFX: 创建着色器 '{}' ('{}') 失败：调用 LuaSTG::Core::Renderer::createPostEffectShader 出错", name, path);
        }
        try
        {
            fcyRefPointer<ResFX> tRes;
            tRes.DirectSet(new ResFX(name, shader));
            m_FXPool.emplace(name, tRes);
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] LoadFX: 内存不足");
            return false;
        }

        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadFX: 已从'{}'加载后处理特效'{}' ({})", path, name, getResourcePoolTypeName());
        }
    }
    
    return true;
}

// 加载模型（废弃）

bool ResourcePool::LoadModel(const char* name, const char* path) noexcept {
    //try {
    //    return LoadModel(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
    //}
    //catch (const std::bad_alloc&) {
    //    //("LoadModel: 转换编码时无法分配内存");
    //    return false;
    //}
    
    //ASSERT(LAPP.GetRenderDev());
    //
    //if (m_TexturePool.find(name) != m_TexturePool.end()) {
    //    //("LoadModel: 模型'%m'已存在，试图使用'%s'加载的操作已被取消", name, path.c_str());
    //    return true;
    //}
    //
    //fcyRefPointer<fcyMemStream> tDataBuf;
    //if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
    //    return false;
    //fcyRefPointer<fcyMemStream> tDataBuf2;
    //std::wstring path2 = path;
    //int i = path2.length();
    //path2[i - 3] = 'm';
    //path2[i - 2] = 't';
    //path2[i - 1] = 'l';
    //if (!m_pMgr->LoadFile(path2.c_str(), tDataBuf2))
    //    return false;
    //void* model = NULL;
    //void* LoadObj(const std::string& id, const std::string& path, const std::string& path2);
    //std::string buf((char*) tDataBuf->GetInternalBuffer(), tDataBuf->GetLength());
    //std::string buf2((char*) tDataBuf2->GetInternalBuffer(), tDataBuf2->GetLength());
    //model = LoadObj(name, buf, buf2);
    //
    //try {
    //    fcyRefPointer<ResModel> tRes;
    //    tRes.DirectSet(new ResModel(name, model));
    //    m_ModelPool.emplace(name, tRes);
    //}
    //catch (const std::bad_alloc&) {
    //    //("LoadModel: 内存不足");
    //    return false;
    //}
    //
    //if (ResourceMgr::GetResourceLoadingLog()) {
    //    //("LoadTexture: 纹理'%s'已装载 -> '%m' (%s)", path.c_str(), name, getResourcePoolTypeNameW());
    //}
    //
    //return true;
    
    return false;
}

// 查找并获取

fcyRefPointer<ResTexture> ResourcePool::GetTexture(const char* name) noexcept {
    auto i = m_TexturePool.find(name);
    if (i == m_TexturePool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResSprite> ResourcePool::GetSprite(const char* name) noexcept {
    auto i = m_SpritePool.find(name);
    if (i == m_SpritePool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResAnimation> ResourcePool::GetAnimation(const char* name) noexcept {
    auto i = m_AnimationPool.find(name);
    if (i == m_AnimationPool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResMusic> ResourcePool::GetMusic(const char* name) noexcept {
    auto i = m_MusicPool.find(name);
    if (i == m_MusicPool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResSound> ResourcePool::GetSound(const char* name) noexcept {
    auto i = m_SoundSpritePool.find(name);
    if (i == m_SoundSpritePool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResParticle> ResourcePool::GetParticle(const char* name) noexcept {
    auto i = m_ParticlePool.find(name);
    if (i == m_ParticlePool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResFont> ResourcePool::GetSpriteFont(const char* name) noexcept {
    auto i = m_SpriteFontPool.find(name);
    if (i == m_SpriteFontPool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResFont> ResourcePool::GetTTFFont(const char* name) noexcept {
    auto i = m_TTFFontPool.find(name);
    if (i == m_TTFFontPool.end())
        return nullptr;
    else
        return i->second;
}

fcyRefPointer<ResFX> ResourcePool::GetFX(const char* name) noexcept {
    auto i = m_FXPool.find(name);
    if (i == m_FXPool.end())
        return nullptr;
    else
        return i->second;
}
