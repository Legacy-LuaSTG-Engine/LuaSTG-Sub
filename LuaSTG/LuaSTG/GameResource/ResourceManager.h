﻿#pragma once
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "GameResource/ResourceMusic.hpp"
#include "GameResource/ResourceSoundEffect.hpp"
#include "GameResource/ResourceParticle.hpp"
#include "GameResource/ResourceFont.hpp"
#include "GameResource/ResourcePostEffectShader.hpp"
#include "GameResource/ResourceModel.hpp"
#include "lua.hpp"
#include "xxhash.h"

namespace LuaSTGPlus
{
    class ResourceMgr;
    
    // 资源池类型
    enum class ResourcePoolType
    {
        None = 0,
        Global,
        Stage
    };
    
    // 资源池
    class ResourcePool
    {
        friend class ResourceMgr;
    public:
        struct dictionary_key_t
        {
        #if (SIZE_MAX == UINT32_MAX)
            XXH32_hash_t const hash{};
            XXH32_hash_t const check{};
        #elif (SIZE_MAX == UINT64_MAX)
            XXH64_hash_t const hash{};
        #else
            static_assert(false, "unsupported platform");
        #endif

            inline dictionary_key_t(std::string_view key) noexcept
            #if (SIZE_MAX == UINT32_MAX)
                : hash(XXH32(key.data(), key.size(), 0))
                , check(XXH32(key.data(), key.size(), 0x65766F6C))
            #elif (SIZE_MAX == UINT64_MAX)
                : hash(XXH3_64bits(key.data(), key.size()))
            #else
                : __invalid_member()
            #endif
            {
            }

            inline bool operator==(dictionary_key_t const& right) const noexcept
            {
            #if (SIZE_MAX == UINT32_MAX)
                return hash == right.hash && check == right.check;
            #elif (SIZE_MAX == UINT64_MAX)
                return hash == right.hash;
            #else
                static_assert(false, "unsupported platform");
            #endif
            }
        };
        struct dictionary_key_hash_t
        {
            inline size_t operator()(dictionary_key_t const& key) const noexcept
            {
                static_assert(sizeof(size_t) == sizeof(decltype(dictionary_key_t::hash)));
                return key.hash;
            }
        };
        template<typename T>
        using dictionary_t = std::pmr::unordered_map<dictionary_key_t, T, dictionary_key_hash_t>;
    private:
        ResourceMgr* m_pMgr;
        ResourcePoolType m_iType;
        std::pmr::unsynchronized_pool_resource m_memory_resource;
        dictionary_t<Core::ScopeObject<IResourceTexture>> m_TexturePool;
        dictionary_t<Core::ScopeObject<IResourceSprite>> m_SpritePool;
        dictionary_t<Core::ScopeObject<IResourceAnimation>> m_AnimationPool;
        dictionary_t<Core::ScopeObject<IResourceMusic>> m_MusicPool;
        dictionary_t<Core::ScopeObject<IResourceSoundEffect>> m_SoundSpritePool;
        dictionary_t<Core::ScopeObject<IResourceParticle>> m_ParticlePool;
        dictionary_t<Core::ScopeObject<IResourceFont>> m_SpriteFontPool;
        dictionary_t<Core::ScopeObject<IResourceFont>> m_TTFFontPool;
        dictionary_t<Core::ScopeObject<IResourcePostEffectShader>> m_FXPool;
        dictionary_t<Core::ScopeObject<IResourceModel>> m_ModelPool;
    private:
        const char* getResourcePoolTypeName();
    public:
        void Clear() noexcept;
        void RemoveResource(ResourceType t, const char* name) noexcept;
        bool CheckResourceExists(ResourceType t, std::string_view name) const noexcept;
        int ExportResourceList(lua_State* L, ResourceType t) const  noexcept;
        
        // 纹理
        bool LoadTexture(const char* name, const char* path, bool mipmaps = true) noexcept;
        bool CreateTexture(const char* name, int width, int height) noexcept;
        // 渲染目标
        bool CreateRenderTarget(const char* name, int width = 0, int height = 0, bool depth_buffer = false) noexcept;
        // 图片精灵
        bool CreateSprite(const char* name, const char* texname,
                          double x, double y, double w, double h,
                          double a, double b, bool rect = false) noexcept;
        // 动画精灵
        bool CreateAnimation(const char* name, const char* texname,
                             double x, double y, double w, double h, int n, int m, int intv,
                             double a, double b, bool rect = false) noexcept;
        bool CreateAnimation(const char* name,
            std::vector<Core::ScopeObject<IResourceSprite>> const& sprite_list,
            int intv,
            double a, double b, bool rect = false) noexcept;
        // 音乐
        bool LoadMusic(const char* name, const char* path, double start, double end, bool once_decode) noexcept;
        // 音效
        bool LoadSoundEffect(const char* name, const char* path) noexcept;
        // 粒子特效(HGE)
        bool LoadParticle(const char* name, const hgeParticleSystemInfo& info, const char* img_name,
                          double a, double b, bool rect = false, bool _nolog = false) noexcept;
        bool LoadParticle(const char* name, const char* path, const char* img_name,
                          double a, double b, bool rect = false) noexcept;
        // 装载纹理字体(HGE)
        bool LoadSpriteFont(const char* name, const char* path, bool mipmaps = true) noexcept;
        // 装载纹理字体(fancy2d)
        bool LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps = true) noexcept;
        // 加载矢量字体
        bool LoadTTFFont(const char* name, const char* path, float width, float height) noexcept;
        bool LoadTrueTypeFont(const char* name, Core::Graphics::TrueTypeFontInfo* fonts, size_t count) noexcept;
        // 特效
        bool LoadFX(const char* name, const char* path) noexcept;
        // 模型
        bool LoadModel(const char* name, const char* path) noexcept;
        
        Core::ScopeObject<IResourceTexture> GetTexture(std::string_view name) noexcept;
        Core::ScopeObject<IResourceSprite> GetSprite(std::string_view name) noexcept;
        Core::ScopeObject<IResourceAnimation> GetAnimation(std::string_view name) noexcept;
        Core::ScopeObject<IResourceMusic> GetMusic(std::string_view name) noexcept;
        Core::ScopeObject<IResourceSoundEffect> GetSound(std::string_view name) noexcept;
        Core::ScopeObject<IResourceParticle> GetParticle(std::string_view name) noexcept;
        Core::ScopeObject<IResourceFont> GetSpriteFont(std::string_view name) noexcept;
        Core::ScopeObject<IResourceFont> GetTTFFont(std::string_view name) noexcept;
        Core::ScopeObject<IResourcePostEffectShader> GetFX(std::string_view name) noexcept;
        Core::ScopeObject<IResourceModel> GetModel(std::string_view name) noexcept;
    public:
        ResourcePool(ResourceMgr* mgr, ResourcePoolType t);
        ResourcePool& operator=(const ResourcePool&) = delete;
        ResourcePool(const ResourcePool&) = delete;
    };
    
    // 资源管理器
    class ResourceMgr
    {
    private:
        ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
        ResourcePool m_GlobalResourcePool;
        ResourcePool m_StageResourcePool;
    public:
        ResourcePoolType GetActivedPoolType() noexcept;
        void SetActivedPoolType(ResourcePoolType t) noexcept;
        ResourcePool* GetActivedPool() noexcept;
        ResourcePool* GetResourcePool(ResourcePoolType t) noexcept;
        void ClearAllResource() noexcept;

        Core::ScopeObject<IResourceTexture> FindTexture(const char* name) noexcept;
        Core::ScopeObject<IResourceSprite> FindSprite(const char* name) noexcept;
        Core::ScopeObject<IResourceAnimation> FindAnimation(const char* name) noexcept;
        Core::ScopeObject<IResourceMusic> FindMusic(const char* name) noexcept;
        Core::ScopeObject<IResourceSoundEffect> FindSound(const char* name) noexcept;
        Core::ScopeObject<IResourceParticle> FindParticle(const char* name) noexcept;
        Core::ScopeObject<IResourceFont> FindSpriteFont(const char* name) noexcept;
        Core::ScopeObject<IResourceFont> FindTTFFont(const char* name) noexcept;
        Core::ScopeObject<IResourcePostEffectShader> FindFX(const char* name) noexcept;
        Core::ScopeObject<IResourceModel> FindModel(const char* name) noexcept;
        
        bool GetTextureSize(const char* name, Core::Vector2U& out) noexcept;
        void CacheTTFFontString(const char* name, const char* text, size_t len) noexcept;
        void UpdateSound();
    private:
        static bool g_ResourceLoadingLog;
        float m_GlobalImageScaleFactor = 1.0f;
    public:
        static void SetResourceLoadingLog(bool b);
        static bool GetResourceLoadingLog();
        float GetGlobalImageScaleFactor() const noexcept { return m_GlobalImageScaleFactor; }
        void SetGlobalImageScaleFactor(float s) noexcept { m_GlobalImageScaleFactor = s; }
        void ShowResourceManagerDebugWindow(bool* p_open = nullptr);
    public:
        ResourceMgr();
    };
}
