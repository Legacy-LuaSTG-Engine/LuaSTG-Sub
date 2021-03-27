#pragma once
#include <cstdint>
#include "Graphic/Type.h"
#include "Graphic/DeviceState.h"

namespace slow::Graphic
{
    class DeviceContext
    {
    private:
        friend class Device;
        struct Implement;
        Implement* implememt = nullptr;
    public:
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
        bool createBlendState(const DBlendState& def, IBlendState** pp);
    public:
        bool bind(handle_t window);
        void unbind();
        bool validate();
    public:
        Device();
        ~Device();
        static Device& get();
    };
};
