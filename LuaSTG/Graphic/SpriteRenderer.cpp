#include "Graphic/SpriteRenderer.h"
#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <DirectXMath.h>

namespace slow::Graphic
{
    using Microsoft::WRL::ComPtr;
    
    struct SpriteRenderer::Implement
    {
    };
    
    #define self (*implememt)
    
    bool SpriteRenderer::bind(Device& device)
    {
        return true;
    };
    void SpriteRenderer::unbind()
    {
    };
    
    SpriteRenderer::SpriteRenderer()
    {
        implememt = new Implement;
    };
    SpriteRenderer::~SpriteRenderer()
    {
        unbind();
        delete implememt;
    };
    SpriteRenderer& SpriteRenderer::get()
    {
        static SpriteRenderer instance;
        return instance;
    };
};
