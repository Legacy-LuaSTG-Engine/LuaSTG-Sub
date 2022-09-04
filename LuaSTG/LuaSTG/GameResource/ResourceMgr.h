#pragma once
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "GameResource/ResourceParticle.hpp"
#include "GameResource/ResourceFont.hpp"
#include "GameResource/ResourceFX.hpp"
#include "GameResource/ResourceAudio.hpp"
#include "GameResource/ResourceModel.hpp"
#include "Utility/Dictionary.hpp"
#include "lua.hpp"

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
    private:
        ResourceMgr* m_pMgr;
        ResourcePoolType m_iType;
        Dictionary<fcyRefPointer<ResTexture>> m_TexturePool;
        Dictionary<fcyRefPointer<ResSprite>> m_SpritePool;
        Dictionary<fcyRefPointer<ResAnimation>> m_AnimationPool;
        Dictionary<fcyRefPointer<ResMusic>> m_MusicPool;
        Dictionary<fcyRefPointer<ResSound>> m_SoundSpritePool;
        Dictionary<fcyRefPointer<ResParticle>> m_ParticlePool;
        Dictionary<fcyRefPointer<ResFont>> m_SpriteFontPool;
        Dictionary<fcyRefPointer<ResFont>> m_TTFFontPool;
        Dictionary<fcyRefPointer<ResFX>> m_FXPool;
        Dictionary<fcyRefPointer<ResModel>> m_ModelPool;
    private:
        const char* getResourcePoolTypeName();
    public:
        void Clear() noexcept;
        void RemoveResource(ResourceType t, const char* name) noexcept;
        bool CheckResourceExists(ResourceType t, const std::string& name) const noexcept;
        int ExportResourceList(lua_State* L, ResourceType t) const  noexcept;
        
        // 纹理
        bool LoadTexture(const char* name, const char* path, bool mipmaps = true) noexcept;
        bool CreateTexture(const char* name, int width, int height) noexcept;
        // 渲染目标
        bool CreateRenderTarget(const char* name, int width = 0, int height = 0) noexcept;
        // 图片精灵
        bool CreateSprite(const char* name, const char* texname,
                          double x, double y, double w, double h,
                          double a, double b, bool rect = false) noexcept;
        // 动画精灵
        bool CreateAnimation(const char* name, const char* texname,
                             double x, double y, double w, double h, int n, int m, int intv,
                             double a, double b, bool rect = false) noexcept;
        // 音乐
        bool LoadMusic(const char* name, const char* path, double start, double end, bool once_decode) noexcept;
        // 音效
        bool LoadSoundEffect(const char* name, const char* path) noexcept;
        // 粒子特效(HGE)
        bool LoadParticle(const char* name, const ResParticle::hgeParticleSystemInfo& info, const char* img_name,
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
        
        fcyRefPointer<ResTexture> GetTexture(const char* name) noexcept;
        fcyRefPointer<ResSprite> GetSprite(const char* name) noexcept;
        fcyRefPointer<ResAnimation> GetAnimation(const char* name) noexcept;
        fcyRefPointer<ResMusic> GetMusic(const char* name) noexcept;
        fcyRefPointer<ResSound> GetSound(const char* name) noexcept;
        fcyRefPointer<ResParticle> GetParticle(const char* name) noexcept;
        fcyRefPointer<ResFont> GetSpriteFont(const char* name) noexcept;
        fcyRefPointer<ResFont> GetTTFFont(const char* name) noexcept;
        fcyRefPointer<ResFX> GetFX(const char* name) noexcept;
        fcyRefPointer<ResModel> GetModel(const char* name) noexcept;
    public:
        ResourcePool(ResourceMgr* mgr, ResourcePoolType t) : m_pMgr(mgr), m_iType(t) {}
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

        fcyRefPointer<ResTexture> FindTexture(const char* name) noexcept;
        fcyRefPointer<ResSprite> FindSprite(const char* name) noexcept;
        fcyRefPointer<ResAnimation> FindAnimation(const char* name) noexcept;
        fcyRefPointer<ResMusic> FindMusic(const char* name) noexcept;
        fcyRefPointer<ResSound> FindSound(const char* name) noexcept;
        fcyRefPointer<ResParticle> FindParticle(const char* name) noexcept;
        fcyRefPointer<ResFont> FindSpriteFont(const char* name) noexcept;
        fcyRefPointer<ResFont> FindTTFFont(const char* name) noexcept;
        fcyRefPointer<ResFX> FindFX(const char* name) noexcept;
        fcyRefPointer<ResModel> FindModel(const char* name) noexcept;
        
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
