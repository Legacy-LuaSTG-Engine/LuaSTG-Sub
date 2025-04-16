#include "GameResource/ResourceManager.h"
#include "GameResource/Implement/ResourceTextureImpl.hpp"
#include "GameResource/Implement/ResourceSpriteImpl.hpp"
#include "GameResource/Implement/ResourceAnimationImpl.hpp"
#include "GameResource/Implement/ResourceMusicImpl.hpp"
#include "GameResource/Implement/ResourceSoundEffectImpl.hpp"
#include "GameResource/Implement/ResourceParticleImpl.hpp"
#include "GameResource/Implement/ResourceFontImpl.hpp"
#include "GameResource/Implement/ResourcePostEffectShaderImpl.hpp"
#include "GameResource/Implement/ResourceModelImpl.hpp"
#include "Core/FileManager.hpp"
#include "AppFrame.h"
#include "lua/plus.hpp"

namespace luastg
{
    // 总体管理

    void ResourcePool::Clear() noexcept
    {
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
        spdlog::info("[luastg] 已清空资源池 '{}'", getResourcePoolTypeName());
    }

    template<typename T>
    inline void removeResource(T& pool, const char* name)
    {
        auto i = pool.find(std::string_view(name));
        if (i == pool.end())
        {
            spdlog::warn("[luastg] RemoveResource: 试图卸载一个不存在的资源 '{}'", name);
            return;
        }
        pool.erase(i);
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] RemoveResource: 资源 '{}' 已卸载", name);
        }
    }

    const char* ResourcePool::getResourcePoolTypeName()
    {
        switch (m_iType) {
            case ResourcePoolType::Global:
                return "global";
            case ResourcePoolType::Stage:
                return "stage";
            default:
                return "none";
        }
    }

    void ResourcePool::RemoveResource(ResourceType t, const char* name) noexcept
    {
        switch (t)
        {
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
            spdlog::warn("[luastg] RemoveResource: 试图移除一个不存在的资源类型 ({})", (int)t);
            return;
        }
    }

    bool ResourcePool::CheckResourceExists(ResourceType t, std::string_view name) const noexcept
    {
        switch (t)
        {
        case ResourceType::Texture:
            return m_TexturePool.find(name) != m_TexturePool.end();
        case ResourceType::Sprite:
            return m_SpritePool.find(name) != m_SpritePool.end();
        case ResourceType::Animation:
            return m_AnimationPool.find(name) != m_AnimationPool.end();
        case ResourceType::Music:
            return m_MusicPool.find(name) != m_MusicPool.end();
        case ResourceType::SoundEffect:
            return m_SoundSpritePool.find(name) != m_SoundSpritePool.end();
        case ResourceType::Particle:
            return m_ParticlePool.find(name) != m_ParticlePool.end();
        case ResourceType::SpriteFont:
            return m_SpriteFontPool.find(name) != m_SpriteFontPool.end();
        case ResourceType::TrueTypeFont:
            return m_TTFFontPool.find(name) != m_TTFFontPool.end();
        case ResourceType::FX:
            return m_FXPool.find(name) != m_FXPool.end();
        case ResourceType::Model:
            return m_ModelPool.find(name) != m_ModelPool.end();
        default:
            spdlog::warn("[luastg] CheckRes: 试图检索一个不存在的资源类型({})", (int)t);
            break;
        }
        return false;
    }

    template<typename T>
    inline void listResourceName(lua_State* L, T& resource_set)
    {
        lua::stack_t S(L);
        int index = 0;
        S.create_array(resource_set.size());
        for (auto& i : resource_set)
        {
            auto ptr = i.second;
            index += 1;
            S.set_array_value<std::string_view>(index, ptr->GetResName());
        }
    }

    int ResourcePool::ExportResourceList(lua_State* L, ResourceType t) const noexcept
    {
        lua::stack_t S(L);
        switch (t)
        {
        case ResourceType::Texture:
            listResourceName(L, m_TexturePool);
            break;
        case ResourceType::Sprite:
            listResourceName(L, m_SpritePool);
            break;
        case ResourceType::Animation:
            listResourceName(L, m_AnimationPool);
            break;
        case ResourceType::Music:
            listResourceName(L, m_MusicPool);
            break;
        case ResourceType::SoundEffect:
            listResourceName(L, m_SoundSpritePool);
            break;
        case ResourceType::Particle:
            listResourceName(L, m_ParticlePool);
            break;
        case ResourceType::SpriteFont:
            listResourceName(L, m_SpriteFontPool);
            break;
        case ResourceType::TrueTypeFont:
            listResourceName(L, m_TTFFontPool);
            break;
        case ResourceType::FX:
            listResourceName(L, m_FXPool);
            break;
        case ResourceType::Model:
            listResourceName(L, m_ModelPool);
            break;
        default:
            spdlog::warn("[luastg] EnumRes: 试图枚举一个不存在的资源类型({})", (int)t);
            S.create_array(0);
            break;
        }
        return 1;
    }

    // 加载纹理

    bool ResourcePool::LoadTexture(const char* name, const char* path, bool mipmaps) noexcept
    {
        if (m_TexturePool.find(std::string_view(name)) != m_TexturePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadTexture: 纹理 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<core::Graphics::ITexture2D> p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(path, mipmaps, ~p_texture))
        {
            spdlog::error("[luastg] 从 '{}' 创建纹理 '{}' 失败", path, name);
            return false;
        }

        try
        {
            core::ScopeObject<IResourceTexture> tRes;
            tRes.attach(new ResourceTextureImpl(name, p_texture.get()));
            m_TexturePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadTexture: 创建纹理 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadTexture: 已从 '{}' 加载纹理 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    bool ResourcePool::CreateTexture(const char* name, int width, int height) noexcept
    {
        if (m_TexturePool.find(std::string_view(name)) != m_TexturePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadTexture: 纹理 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }

        core::ScopeObject<core::Graphics::ITexture2D> p_texture;
        if (!LAPP.GetAppModel()->getDevice()->createTexture(core::Vector2U((uint32_t)width, (uint32_t)height), ~p_texture))
        {
            spdlog::error("[luastg] 创建纹理 '{}' ({}x{}) 失败", name, width, height);
            return false;
        }

        try
        {
            core::ScopeObject<IResourceTexture> tRes;
            tRes.attach(new ResourceTextureImpl(name, p_texture.get()));
            m_TexturePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadTexture: {}", e.what());
            return false;
        }

        if (ResourceMgr::GetResourceLoadingLog()) {
            spdlog::info("[luastg] LoadTexture: 已创建纹理 '{}' ({}x{}) ({})", name, width, height, getResourcePoolTypeName());
        }

        return true;
    }

    // 创建渲染目标

    bool ResourcePool::CreateRenderTarget(const char* name, int width, int height, bool depth_buffer) noexcept
    {
        if (m_TexturePool.find(std::string_view(name)) != m_TexturePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] CreateRenderTarget: 渲染目标 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }
    
        std::string_view ds_info("和深度缓冲区");

        try
        {
            core::ScopeObject<IResourceTexture> tRes;
            if (width <= 0 || height <= 0)
            {
                tRes.attach(new ResourceTextureImpl(name, depth_buffer));
            }
            else
            {
                tRes.attach(new ResourceTextureImpl(name, width, height, depth_buffer));
            }
            m_TexturePool.emplace(name, tRes);
        }
        catch (std::runtime_error const& e)
        {
            spdlog::error("[luastg] CreateRenderTarget: 创建渲染目标 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            if (width <= 0 || height <= 0)
            {
                spdlog::info("[luastg] CreateRenderTarget: 已创建渲染目标{} '{}' ({})", ds_info, name, getResourcePoolTypeName());
            }
            else
            {
                spdlog::info("[luastg] CreateRenderTarget: 已创建渲染目标{} '{}' ({}x{}) ({})", ds_info, name, width, height, getResourcePoolTypeName());
            }
        }
    
        return true;
    }

    // 创建图片精灵

    bool ResourcePool::CreateSprite(const char* name, const char* texname,
                                    double x, double y, double w, double h,
                                    double a, double b, bool rect) noexcept
    {
        if (m_SpritePool.find(std::string_view(name)) != m_SpritePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] CreateSprite: 图片精灵 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<IResourceTexture> pTex = m_pMgr->FindTexture(texname);
        if (!pTex)
        {
            spdlog::error("[luastg] CreateSprite: 无法创建图片精灵 '{}'，无法找到纹理 '{}'", name, texname);
            return false;
        }
    
        core::ScopeObject<core::Graphics::ISprite> p_sprite;
        if (!core::Graphics::ISprite::create(
            LAPP.GetAppModel()->getRenderer(),
            pTex->GetTexture(),
            ~p_sprite
        ))
        {
            spdlog::error("[luastg] 从纹理 '{}' 创建图片精灵 '{}' 失败", texname, name);
            return false;
        }
        p_sprite->setTextureRect(core::RectF((float)x, (float)y, (float)(x + w), (float)(y + h)));
        p_sprite->setTextureCenter(core::Vector2F((float)(x + w * 0.5), (float)(y + h * 0.5)));
    
        try
        {
            core::ScopeObject<IResourceSprite> tRes;
            tRes.attach(new ResourceSpriteImpl(name, p_sprite.get(), a, b, rect));
            m_SpritePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] CreateSprite: 创建图片精灵 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] CreateSprite: 已从纹理 '{}' 创建图片精灵 '{}' ({})", texname, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 创建动画精灵

    bool ResourcePool::CreateAnimation(const char* name, const char* texname,
                                       double x, double y, double w, double h, int n, int m, int intv,
                                       double a, double b, bool rect) noexcept
    {
        if (m_AnimationPool.find(std::string_view(name)) != m_AnimationPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] CreateAnimation: 动画精灵 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<IResourceTexture> pTex = m_pMgr->FindTexture(texname);
        if (!pTex)
        {
            spdlog::error("[luastg] CreateAnimation: 无法创建动画精灵 '{}'，无法找到纹理 '{}'", name, texname);
            return false;
        }
    
        try {
            core::ScopeObject<IResourceAnimation> tRes;
            tRes.attach(
                new ResourceAnimationImpl(name, pTex,
                    (float) x, (float) y,
                    (float) w, (float) h,
                    n, m, intv,
                    a, b, rect)
            );
            m_AnimationPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] CreateAnimation: 创建动画精灵 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] CreateAnimation: 已从 '{}' 创建动画精灵 '{}' ({})", texname, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    bool ResourcePool::CreateAnimation(const char* name,
        std::vector<core::ScopeObject<IResourceSprite>> const& sprite_list,
        int intv,
        double a, double b, bool rect) noexcept
    {
        if (m_AnimationPool.find(std::string_view(name)) != m_AnimationPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] CreateAnimation: 动画精灵 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }

        try {
            core::ScopeObject<IResourceAnimation> tRes;
            tRes.attach(
                new ResourceAnimationImpl(name, sprite_list, intv, a, b, rect)
            );
            m_AnimationPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] CreateAnimation: 创建动画精灵 '{}' 失败 ({})", name, e.what());
            return false;
        }

        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] CreateAnimation: 已创建动画精灵 '{}' ({})", name, getResourcePoolTypeName());
        }

        return true;
    }

    // 加载音乐

    bool ResourcePool::LoadMusic(const char* name, const char* path, double start, double end, bool once_decode) noexcept
    {
        if (m_MusicPool.find(std::string_view(name)) != m_MusicPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadMusic: 音乐 '{}' 已存在，创建操作已取消", name);
            }
            //m_MusicPool.find(name)->second->Stop(); // 注：以前确实不判断同名资源是否存在，但是 emplace 失败了，所以没有打断旧 BGM
            return true;
        }
    
        using namespace core;
        using namespace core::Audio;

        // 创建解码器
        ScopeObject<IDecoder> p_decoder;
        if (!IDecoder::create(path, ~p_decoder))
        {
            spdlog::error("[luastg] LoadMusic: 无法解码文件 '{}'，要求文件格式为 WAV 或 OGG", path);
            return false;
        }
        auto to_sample = [&p_decoder](double t) -> uint32_t
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
        ScopeObject<ResourceMusicImpl::LoopDecoder> p_loop_decoder;
        p_loop_decoder.attach(new ResourceMusicImpl::LoopDecoder(p_decoder.get(), start, end));

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
            core::ScopeObject<IResourceMusic> tRes;
            tRes.attach(new ResourceMusicImpl(name, p_loop_decoder.get(), p_player.get()));
            m_MusicPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadMusic: 加载音乐 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadMusic: 已从 '{}' 加载音乐 '{}'{} ({})", path, name, once_decode ? " 并一次性解码" : "", getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载音效

    bool ResourcePool::LoadSoundEffect(const char* name, const char* path) noexcept
    {
        if (m_SoundSpritePool.find(std::string_view(name)) != m_SoundSpritePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadSoundEffect: 音效 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }

        using namespace core;
        using namespace core::Audio;

        // 创建解码器
        ScopeObject<IDecoder> p_decoder;
        if (!IDecoder::create(path, ~p_decoder))
        {
            spdlog::error("[luastg] LoadSoundEffect: 无法解码文件 '{}'，要求文件格式为 WAV 或 OGG", path);
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
            core::ScopeObject<IResourceSoundEffect> tRes;
            tRes.attach(new ResourceSoundEffectImpl(name, p_player.get()));
            m_SoundSpritePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadSoundEffect: 加载音效 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadSoundEffect: 已从 '{}' 加载音效 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 创建粒子特效

    bool ResourcePool::LoadParticle(const char* name, const hgeParticleSystemInfo& info, const char* img_name,
                                    double a,double b, bool rect, bool _nolog) noexcept
    {
        if (m_ParticlePool.find(std::string_view(name)) != m_ParticlePool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadParticle: 粒子特效 '{}' 已存在，创建操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<IResourceSprite> pSprite = m_pMgr->FindSprite(img_name);
        if (!pSprite)
        {
            spdlog::error("[luastg] LoadParticle: 无法创建粒子特效 '{}'，找不到图片精灵 '{}'", name, img_name);
            return false;
        }
    
        core::ScopeObject<core::Graphics::ISprite> p_sprite;
        if (!pSprite->GetSprite()->clone(~p_sprite))
        {
            spdlog::error("[luastg] LoadParticle: 无法创建粒子特效 '{}'，复制图片精灵 '{}' 失败", name, img_name);
            return false;
        }

        try
        {
            core::ScopeObject<IResourceParticle> tRes;
            tRes.attach(new ResourceParticleImpl(name, info, p_sprite.get(), a, b, rect));
            m_ParticlePool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadParticle: 创建粒子特效 '{}' 失败 ({})", name, e.what());
            return false;
        }
    
        if (!_nolog && ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadParticle: 已创建粒子特效 '{}' ({})", name, getResourcePoolTypeName());
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
    
        if (src.size() != sizeof(hgeParticleSystemInfo))
        {
            spdlog::error("[luastg] LoadParticle: 粒子特效定义文件 '{}' 格式不正确", path);
            return false;
        }
        hgeParticleSystemInfo tInfo;
        std::memcpy(&tInfo, src.data(), sizeof(hgeParticleSystemInfo));
    
        if (!LoadParticle(name, tInfo, img_name, a, b, rect, /* _nolog */ true))
        {
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadParticle: 已从 '{}' 创建粒子特效 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载纹理字体（HGE）

    bool ResourcePool::LoadSpriteFont(const char* name, const char* path, bool mipmaps) noexcept
    {
        if (m_SpriteFontPool.find(std::string_view(name)) != m_SpriteFontPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadSpriteFont: 纹理字体 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        // 创建定义
        try
        {
            core::ScopeObject<IResourceFont> tRes;
            tRes.attach(new ResourceFontImpl(name, path, mipmaps));
            m_SpriteFontPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 无法加载 HGE 纹理字体 '{}' ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadSpriteFont: 已从 '{}' 加载 HGE 纹理字体 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载纹理字体（fancy2d）

    bool ResourcePool::LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps) noexcept
    {
        if (m_SpriteFontPool.find(std::string_view(name)) != m_SpriteFontPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadSpriteFont: 纹理字体 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        // 创建定义
        try
        {
            core::ScopeObject<IResourceFont> tRes;
            tRes.attach(new ResourceFontImpl(name, path, tex_path, mipmaps));
            m_SpriteFontPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadSpriteFont: 无法加载 fancy2d 纹理字体 '{}' ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadSpriteFont: 已从 '{}' 和 '{}' 加载 fancy2d 纹理字体 '{}' ({})", path, tex_path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载TrueType字体

    bool ResourcePool::LoadTTFFont(const char* name, const char* path, float width, float height) noexcept
    {
        if (m_TTFFontPool.find(std::string_view(name)) != m_TTFFontPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadTTFFont: 矢量字体 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<core::Graphics::IGlyphManager> p_glyphmgr;
        core::Graphics::TrueTypeFontInfo create_info = {
            .source = path,
            .font_face = 0,
            .font_size = core::Vector2F(width, height),
            .is_force_to_file = false,
            .is_buffer = false,
        };
        if (!core::Graphics::IGlyphManager::create(LAPP.GetAppModel()->getDevice(), &create_info, 1, ~p_glyphmgr))
        {
            spdlog::error("[luastg] LoadTTFFont: 加载矢量字体 '{}' 失败", name);
            return false;
        }

        // 创建定义
        try
        {
            core::ScopeObject<IResourceFont> tRes;
            tRes.attach(new ResourceFontImpl(name, p_glyphmgr.get()));
            m_TTFFontPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadTTFFont: 无法加载矢量字体 '{}' ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadTTFFont: 已从 '{}' 加载矢量字体 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    bool ResourcePool::LoadTrueTypeFont(const char* name, core::Graphics::TrueTypeFontInfo* fonts, size_t count) noexcept
    {
        if (m_TTFFontPool.find(std::string_view(name)) != m_TTFFontPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadTrueTypeFont: 矢量字体组 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        core::ScopeObject<core::Graphics::IGlyphManager> p_glyphmgr;
        if (!core::Graphics::IGlyphManager::create(LAPP.GetAppModel()->getDevice(), fonts, count, ~p_glyphmgr))
        {
            spdlog::error("[luastg] LoadTrueTypeFont: 加载矢量字体组 '{}' 失败", name);
            return false;
        }

        // 创建定义
        try
        {
            core::ScopeObject<IResourceFont> tRes;
            tRes.attach(new ResourceFontImpl(name, p_glyphmgr.get()));
            m_TTFFontPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadTrueTypeFont: 无法加载矢量字体组 '{}' ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadTrueTypeFont: 已加载矢量字体组 '{}' ({})", name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载后处理特效

    bool ResourcePool::LoadFX(const char* name, const char* path) noexcept
    {
        if (m_FXPool.find(std::string_view(name)) != m_FXPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadFX: 后处理特效 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        try
        {
            core::ScopeObject<IResourcePostEffectShader> tRes;
            tRes.attach(new ResourcePostEffectShaderImpl(name, path));
            if (!tRes->GetPostEffectShader())
            {
                spdlog::error("[luastg] LoadFX: 从 '{}' 加载后处理特效 '{}' 失败", path, name);
                return false;
            }
            m_FXPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadFX: 无法加载后处理特效 '{}' ({})", name, e.what());
            return false;
        }

        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadFX: 已从 '{}' 加载后处理特效 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 加载模型

    bool ResourcePool::LoadModel(const char* name, const char* path) noexcept
    {
        if (m_ModelPool.find(std::string_view(name)) != m_ModelPool.end())
        {
            if (ResourceMgr::GetResourceLoadingLog())
            {
                spdlog::warn("[luastg] LoadModel: 模型 '{}' 已存在，加载操作已取消", name);
            }
            return true;
        }
    
        try
        {
            core::ScopeObject<IResourceModel> tRes;
            tRes.attach(new ResourceModelImpl(name, path));
            m_ModelPool.emplace(name, tRes);
        }
        catch (std::exception const& e)
        {
            spdlog::error("[luastg] LoadModel: 无法加载模型 '{}' ({})", name, e.what());
            return false;
        }
    
        if (ResourceMgr::GetResourceLoadingLog())
        {
            spdlog::info("[luastg] LoadModel: 已从 '{}' 加载模型 '{}' ({})", path, name, getResourcePoolTypeName());
        }
    
        return true;
    }

    // 查找并获取

    template<typename T>
    inline T::value_type::second_type findResource(T& resource_set, std::string_view name)
    {
        auto i = resource_set.find(name);
        if (i == resource_set.end())
            return nullptr;
        else
            return i->second;
    }

	core::ScopeObject<IResourceTexture> ResourcePool::GetTexture(std::string_view name) noexcept
    {
        return findResource(m_TexturePool, name);
	}

	core::ScopeObject<IResourceSprite> ResourcePool::GetSprite(std::string_view name) noexcept
    {
        return findResource(m_SpritePool, name);
	}

	core::ScopeObject<IResourceAnimation> ResourcePool::GetAnimation(std::string_view name) noexcept
    {
        return findResource(m_AnimationPool, name);
	}

	core::ScopeObject<IResourceMusic> ResourcePool::GetMusic(std::string_view name) noexcept
    {
        return findResource(m_MusicPool, name);
	}

	core::ScopeObject<IResourceSoundEffect> ResourcePool::GetSound(std::string_view name) noexcept
    {
        return findResource(m_SoundSpritePool, name);
	}

	core::ScopeObject<IResourceParticle> ResourcePool::GetParticle(std::string_view name) noexcept
    {
        return findResource(m_ParticlePool, name);
	}

	core::ScopeObject<IResourceFont> ResourcePool::GetSpriteFont(std::string_view name) noexcept
    {
        return findResource(m_SpriteFontPool, name);
	}

	core::ScopeObject<IResourceFont> ResourcePool::GetTTFFont(std::string_view name) noexcept
    {
        return findResource(m_TTFFontPool, name);
	}

	core::ScopeObject<IResourcePostEffectShader> ResourcePool::GetFX(std::string_view name) noexcept
    {
        return findResource(m_FXPool, name);
	}

	core::ScopeObject<IResourceModel> ResourcePool::GetModel(std::string_view name) noexcept
	{
        return findResource(m_ModelPool, name);
	}

    ResourcePool::ResourcePool(ResourceMgr* mgr, ResourcePoolType t)
        : m_pMgr(mgr)
        , m_iType(t)
        , m_TexturePool(&m_memory_resource)
        , m_SpritePool(&m_memory_resource)
        , m_AnimationPool(&m_memory_resource)
        , m_MusicPool(&m_memory_resource)
        , m_SoundSpritePool(&m_memory_resource)
        , m_ParticlePool(&m_memory_resource)
        , m_SpriteFontPool(&m_memory_resource)
        , m_TTFFontPool(&m_memory_resource)
        , m_FXPool(&m_memory_resource)
        , m_ModelPool(&m_memory_resource)
    {

    }
}
