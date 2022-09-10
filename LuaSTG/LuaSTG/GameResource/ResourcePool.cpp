#include "GameResource/ResourceMgr.h"
#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"
#include "AppFrame.h"
#include "fcyException.h"

namespace LuaSTGPlus
{
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
        m_ModelPool.clear();
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
            case ResourceType::Model:
                removeResource(m_ModelPool, name);
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
            case ResourceType::Model:
                return m_ModelPool.find(name.c_str()) != m_ModelPool.end();
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
            case ResourceType::Model:
                lua_createtable(L, (int)m_ModelPool.size(), 0);  // t
                for (auto& i : m_ModelPool) {
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
        if (m_TexturePool.find(name) != m_TexturePool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadTexture: 纹理'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        Core::ScopeObject<Core::Graphics::ITexture2D> p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(path, mipmaps, ~p_texture))
        {
            spdlog::error("[luastg] 从'{}'创建纹理'{}'失败", path, name);
            return false;
        }

        try {
            fcyRefPointer<ResTexture> tRes;
            tRes.DirectSet(new ResTexture(name, p_texture.get()));
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

    bool ResourcePool::CreateTexture(const char* name, int width, int height) noexcept
    {
        if (m_TexturePool.find(name) != m_TexturePool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadTexture: 纹理'{}'已存在，创建操作已取消", name);
            }
            return true;
        }

        Core::ScopeObject<Core::Graphics::ITexture2D> p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTexture(Core::Vector2U((uint32_t)width, (uint32_t)height), ~p_texture))
        {
            spdlog::error("[luastg] 从创建纹理'{}'({}x{})失败", name, width, height);
            return false;
        }

        try {
            fcyRefPointer<ResTexture> tRes;
            tRes.DirectSet(new ResTexture(name, p_texture.get()));
            m_TexturePool.emplace(name, tRes);
        }
        catch (const std::bad_alloc&) {
            spdlog::error("[luastg] LoadTexture: 内存不足");
            return false;
        }

        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadTexture: 已创建纹理'{}'({}x{}) ({})", name, width, height, getResourcePoolTypeName());
        }

        return true;
    }

    // 创建渲染目标

    bool ResourcePool::CreateRenderTarget(const char* name, int width, int height) noexcept {
        if (m_TexturePool.find(name) != m_TexturePool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] CreateRenderTarget: 渲染目标'{}'已存在，创建操作已取消", name);
            }
            return true;
        }
    
        try
        {
            fcyRefPointer<ResTexture> tRes;
            if (width <= 0 || height <= 0)
            {
                tRes.DirectSet(new ResTexture(name, true));
            }
            else
            {
                tRes.DirectSet(new ResTexture(name, width, height, true));
            }
            m_TexturePool.emplace(name, tRes);
        }
        catch (const std::runtime_error&)
        {
            spdlog::error("[luastg] CreateRenderTarget: 内部错误");
            return false;
        }
        catch (const std::bad_alloc&)
        {
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
    
        Core::ScopeObject<Core::Graphics::ISprite> p_sprite;
        if (!Core::Graphics::ISprite::create(
            LAPP.GetAppModel()->getRenderer(),
            pTex->GetTexture(),
            ~p_sprite
        ))
        {
            spdlog::error("[luastg] 从'{}'创建图片精灵'{}'失败", texname, name);
            return false;
        }
        p_sprite->setTextureRect(Core::RectF((float)x, (float)y, (float)(x + w), (float)(y + h)));
        p_sprite->setTextureCenter(Core::Vector2F((float)(x + w * 0.5), (float)(y + h * 0.5)));
    
        try {
            fcyRefPointer<ResSprite> tRes;
            tRes.DirectSet(new ResSprite(name, p_sprite.get(), a, b, rect));
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

    bool ResourcePool::LoadMusic(const char* name, const char* path, double start, double end, bool once_decode) noexcept
    {
        if (m_MusicPool.find(name) != m_MusicPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadMusic: 音乐'{}'已存在，创建操作已取消", name);
            }
            //m_MusicPool.find(name)->second->Stop(); // 注：以前确实不判断同名资源是否存在，但是 emplace 失败了，所以没有打断旧 BGM
            return true;
        }
    
        using namespace Core;
        using namespace Core::Audio;

        // 创建解码器
        ScopeObject<IDecoder> p_decoder;
        if (!IDecoder::create(path, ~p_decoder))
        {
            spdlog::error("[luastg] LoadMusic: 无法解码文件'{}'，要求文件格式为 WAV 或 OGG", path);
            return false;
        }
        auto to_sample = [&](double t) -> uint32_t
        {
            return (uint32_t)(t * (double)p_decoder->getSampleRate());
        };

        // 检查循环节
        if (0 == to_sample(start) && to_sample(start) == to_sample(end))
        {
            end = (double)p_decoder->getFrameCount() / (double)p_decoder->getSampleRate();
            spdlog::info("[luastg] LoadMusic: 循环节范围设置为整首背景音乐 (start = {}, end = {})", start, end);
        }
        if (to_sample(start) >= to_sample(end))
        {
            spdlog::error("[luastg] LoadMusic: 循环节范围格式错误，结束位置不能等于或先于开始位置 (start = {}, end = {})", start, end);
            return false;
        }
    
        // 配置循环解码器（这里不用担心出现 exception，因为上面已经处理了）
        ScopeObject<ResMusic::LoopDecoder> p_loop_decoder;
        p_loop_decoder.attach(new ResMusic::LoopDecoder(p_decoder.get(), start, end));

        // 创建播放器
        ScopeObject<IAudioPlayer> p_player;
        if (!once_decode)
        {
            // 流式播放器
            if (!LAPP.GetAppModel()->getAudioDevice()->createStreamAudioPlayer(p_loop_decoder.get(), ~p_player))
            {
                spdlog::error("[luastg] LoadMusic: 无法创建音频播放器");
                return false;
            }
        }
        else
        {
            // 一次性解码的播放器
            if (!LAPP.GetAppModel()->getAudioDevice()->createLoopAudioPlayer(p_decoder.get(), ~p_player))
            {
                spdlog::error("[luastg] LoadMusic: 无法创建音频播放器");
                return false;
            }
            p_player->setLoop(true, start, end - start);
        }

        try
        {
            //存入资源池
            fcyRefPointer<ResMusic> tRes;
            tRes.DirectSet(new ResMusic(name, p_loop_decoder.get(), p_player.get()));
            m_MusicPool.emplace(name, tRes);
        }
        catch ( std::exception const& e)
        {
            spdlog::error("[luastg] LoadMusic: {}", e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadMusic: 已从'{}'加载音乐'{}'{} ({})", path, name, once_decode ? "并一次性解码" : "", getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载音效

    bool ResourcePool::LoadSoundEffect(const char* name, const char* path) noexcept
    {
        if (m_SoundSpritePool.find(name) != m_SoundSpritePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadSoundEffect: 音效'{}'已存在，创建操作已取消", name);
            }
            return true;
        }

        using namespace Core;
        using namespace Core::Audio;

        // 创建解码器
        ScopeObject<IDecoder> p_decoder;
        if (!IDecoder::create(path, ~p_decoder))
        {
            spdlog::error("[luastg] LoadSoundEffect: 无法解码文件'{}'，要求文件格式为 WAV 或 OGG", path);
            return false;
        }

        // 创建播放器
        ScopeObject<IAudioPlayer> p_player;
        if (!LAPP.GetAppModel()->getAudioDevice()->createAudioPlayer(p_decoder.get(), ~p_player))
        {
            spdlog::error("[luastg] LoadSoundEffect: 无法创建音频播放器");
            return false;
        }

        try
        {
            fcyRefPointer<ResSound> tRes;
            tRes.DirectSet(new ResSound(name, p_player.get()));
            m_SoundSpritePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadSoundEffect: {}", e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadSoundEffect: 已从'{}'加载音效'{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 创建粒子特效

    bool ResourcePool::LoadParticle(const char* name, const ResParticle::hgeParticleSystemInfo& info, const char* img_name,
                                    double a,double b, bool rect, bool _nolog) noexcept
    {
        if (m_ParticlePool.find(name) != m_ParticlePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadParticle: 粒子特效'{}'已存在，创建操作已取消", name);
            }
            return true;
        }
    
        fcyRefPointer<ResSprite> pSprite = m_pMgr->FindSprite(img_name);
        if (!pSprite) {
            spdlog::error("[luastg] LoadParticle: 无法创建粒子特效'{}'，找不到图片精灵'{}'", name, img_name);
            return false;
        }
    
        Core::ScopeObject<Core::Graphics::ISprite> p_sprite;
        if (!pSprite->GetSprite()->clone(~p_sprite))
        {
            spdlog::error("[luastg] LoadParticle: 无法创建粒子特效'{}'，复制图片精灵'{}'失败", name, img_name);
            return false;
        }

        try {
            fcyRefPointer<ResParticle> tRes;
            tRes.DirectSet(new ResParticle(name, info, p_sprite.get(), a, b, rect));
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
                                    double a, double b,bool rect) noexcept
    {
        std::vector<uint8_t> src;
        if (!GFileManager().loadEx(path, src))
        {
            spdlog::error("[luastg] LoadParticle：无法从 '{}' 加载粒子特效 '{}'，读取文件失败", path, name);
            return false;
        }
    
        if (src.size() != sizeof(ResParticle::hgeParticleSystemInfo))
        {
            spdlog::error("[luastg] LoadParticle: 粒子特效定义文件'{}'格式不正确", path);
            return false;
        }
        ResParticle::hgeParticleSystemInfo tInfo;
        std::memcpy(&tInfo, src.data(), sizeof(ResParticle::hgeParticleSystemInfo));
    
        if (!LoadParticle(name, tInfo, img_name, a, b, rect, /* _nolog */ true))
        {
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadParticle: 已从'{}'创建粒子特效'{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载纹理字体（HGE）

    bool ResourcePool::LoadSpriteFont(const char* name, const char* path, bool mipmaps) noexcept {
        if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadSpriteFont: 纹理字体'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        // 创建定义
        try
        {
            fcyRefPointer<ResFont> tRes;
            tRes.DirectSet(new ResFont(name, path, mipmaps));
            m_SpriteFontPool.emplace(name, tRes);
        }
        catch (const std::runtime_error& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 HGE 纹理字体失败 ({})", e.what());
            return false;
        }
        catch (const fcyException& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 HGE 纹理字体失败 [{}] ({})", e.GetSrc(), e.GetDesc());
            return false;
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] LoadSpriteFont: 内存不足");
            return false;
        }
        catch (...)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 HGE 纹理字体失败");
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadSpriteFont: 已从'{}'加载 HGE 纹理字体'{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载纹理字体（fancy2d）

    bool ResourcePool::LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps) noexcept {
        if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadSpriteFont: 纹理字体'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        // 创建定义
        try
        {
            fcyRefPointer<ResFont> tRes;
            tRes.DirectSet(new ResFont(name, path, tex_path, mipmaps));
            m_SpriteFontPool.emplace(name, tRes);
        }
        catch (const std::runtime_error& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 fancy2d 纹理字体失败 ({})", e.what());
            return false;
        }
        catch (const fcyException& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 fancy2d 纹理字体失败 [{}] ({})", e.GetSrc(), e.GetDesc());
            return false;
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] LoadSpriteFont: 内存不足");
            return false;
        }
        catch (...)
        {
            spdlog::error("[luastg] LoadSpriteFont: 加载 fancy2d 纹理字体失败");
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadSpriteFont: 已从'{}'和'{}'加载 fancy2d 纹理字体'{}' ({})", path, tex_path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载TrueType字体

    bool ResourcePool::LoadTTFFont(const char* name, const char* path, float width, float height) noexcept {
        if (m_TTFFontPool.find(name) != m_TTFFontPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadTTFFont: 矢量字体'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        Core::ScopeObject<Core::Graphics::IGlyphManager> p_glyphmgr;
        Core::Graphics::TrueTypeFontInfo create_info = {
            .source = path,
            .font_face = 0,
            .font_size = Core::Vector2F(width, height),
            .is_force_to_file = false,
            .is_buffer = false,
        };
        if (!Core::Graphics::IGlyphManager::create(LAPP.GetAppModel()->getDevice(), &create_info, 1, ~p_glyphmgr))
        {
            spdlog::error("[luastg] LoadTTFFont: 加载矢量字体'{}'失败", name);
            return false;
        }

        // 创建定义
        try
        {
            fcyRefPointer<ResFont> tRes;
            tRes.DirectSet(new ResFont(name, p_glyphmgr.get()));
            m_TTFFontPool.emplace(name, tRes);
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] LoadTTFFont: 内存不足");
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadTTFFont: 已从'{}'加载矢量字体'{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    bool ResourcePool::LoadTrueTypeFont(const char* name, Core::Graphics::TrueTypeFontInfo* fonts, size_t count) noexcept {
        if (m_TTFFontPool.find(name) != m_TTFFontPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadTrueTypeFont: 矢量字体组'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        Core::ScopeObject<Core::Graphics::IGlyphManager> p_glyphmgr;
        if (!Core::Graphics::IGlyphManager::create(LAPP.GetAppModel()->getDevice(), fonts, count, ~p_glyphmgr))
        {
            spdlog::error("[luastg] LoadTrueTypeFont: 加载矢量字体组'{}'失败", name);
            return false;
        }

        // 创建定义
        try
        {
            fcyRefPointer<ResFont> tRes;
            tRes.DirectSet(new ResFont(name, p_glyphmgr.get()));
            m_TTFFontPool.emplace(name, tRes);
        }
        catch (const std::bad_alloc&)
        {
            spdlog::error("[luastg] LoadTrueTypeFont: 内存不足");
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadTrueTypeFont: 已加载矢量字体组'{}' ({})", name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载后处理特效

    bool ResourcePool::LoadFX(const char* name, const char* path) noexcept {
        if (m_FXPool.find(name) != m_FXPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadFX: 后处理特效'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        try
        {
            fcyRefPointer<ResFX> tRes;
            tRes.DirectSet(new ResFX(name, path));
            if (!tRes->GetPostEffectShader())
            {
                spdlog::error("[luastg] LoadFX: 从'{}'加载后处理特效'{}'失败", path, name);
                return false;
            }
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
    
        return true;
    }

    // 加载模型

    bool ResourcePool::LoadModel(const char* name, const char* path) noexcept
    {
        if (m_ModelPool.find(name) != m_ModelPool.end()) {
            if (ResourceMgr::GetResourceLoadingLog()) {
                spdlog::warn("[luastg] LoadModel: 模型'{}'已存在，加载操作已取消", name);
            }
            return true;
        }
    
        try {
            fcyRefPointer<ResModel> tRes;
            tRes.DirectSet(new ResModel(name, path));
            m_ModelPool.emplace(name, tRes);
        }
        catch (const std::bad_alloc&) {
            spdlog::error("[luastg] LoadModel: 内存不足");
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadModel: 已从'{}'加载模型'{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
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

    fcyRefPointer<ResModel> ResourcePool::GetModel(const char* name) noexcept
    {
        auto i = m_ModelPool.find(name);
        if (i == m_ModelPool.end())
            return nullptr;
        else
            return i->second;
    }
}
