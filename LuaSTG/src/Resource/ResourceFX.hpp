#pragma once
#include "ResourceBase.hpp"
#include "cpp/Dictionary.hpp"
#include "f2dRenderDevice.h"

namespace LuaSTGPlus {
    // shader包装
    class ResFX :
        public Resource
    {
    private:
        bool m_bIsEffect;
        
        fcyRefPointer<f2dEffect> m_pShader;
        
        // 特殊对象绑定
        std::vector<f2dEffectParamValue*> m_pBindingPostEffectTexture;  // POSTEFFECTTEXTURE
        std::vector<f2dEffectParamValue*> m_pBindingViewport;  // VIEWPORT
        std::vector<f2dEffectParamValue*> m_pBindingScreenSize;  // SCREENSIZE
        
        // 变量绑定
        Dictionary<std::vector<f2dEffectParamValue*>> m_pBindingVar;
        
        void* m_pPixelShader;
    public:
        bool IsEffect() noexcept { return m_bIsEffect; };
        
        f2dEffect* GetEffect() noexcept { return m_pShader; }
        void* GetPixelShader() noexcept { return m_pPixelShader; }
        
        void SetPostEffectTexture(f2dTexture2D* val) noexcept;
        void SetViewport(fcyRect rect) noexcept;
        void SetScreenSize(fcyVec2 size) noexcept;
        
        void SetValue(const char* key, float val) noexcept;
        void SetValue(const char* key, fcyColor val) noexcept;  // 以float4进行绑定
        void SetValue(const char* key, f2dTexture2D* val) noexcept;
    public:
        ResFX(const char* name, fcyRefPointer<f2dEffect> shader);
        ResFX(const char* name, void* shader);
        virtual ~ResFX();
    };
}
