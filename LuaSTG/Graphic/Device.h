#pragma once
#include <cstdint>
#include "Graphic/Type.h"
#include "Graphic/DeviceState.h"
#include "Graphic/DeviceResource.h"

namespace slow::Graphic
{
    class DeviceContext
    {
    private:
        friend class Device;
        struct Implement;
        Implement* implememt = nullptr;
    public:
        void setRasterizerState(IRasterizerState* p);
        void setPixelShaderSampler(uint32_t slot, ISamplerState* p);
        void setDepthStencilState(IDepthStencilState* p);
        void setBlendState(IBlendState* p);
        void setBlendFactor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
    private:
        DeviceContext();
        ~DeviceContext();
    };
    
    class Device
    {
    private:
        struct Implement;
        Implement* implememt = nullptr;
    public:
        handle_t getHandle();
        DeviceContext& getContext();
    public:
        bool autoResizeSwapChain();
        bool resizeSwapChain(uint32_t width, uint32_t height);
        void setSwapChain();
        void clearRenderTarget(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
        void clearDepthBuffer(float depth = 1.0f, uint8_t stencil = 0u);
        bool updateSwapChain(bool vsync = true);
    public:
        bool createRasterizerState(const DRasterizerState& def, IRasterizerState** pp);
        bool createSamplerState(const DSamplerState& def, ISamplerState** pp);
        bool createDepthStencilState(const DDepthStencilState& def, IDepthStencilState** pp);
        bool createBlendState(const DBlendState& def, IBlendState** pp);
    public:
        bool createTexture2D(const char* path, ITexture2D** pp);
    public:
        bool bind(handle_t window);
        void unbind();
        bool validate();
    public:
        Device();
        ~Device();
        static Device& get();
    };
}
