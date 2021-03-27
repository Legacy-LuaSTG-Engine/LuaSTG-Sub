#pragma once
#include <cstdint>
#include "Graphic/Type.h"

namespace slow::Graphic
{
    class Device
    {
    private:
        struct Implement;
        Implement* implememt = nullptr;
    public:
        handle_t getDeviceHandle();
    public:
        bool autoResizeSwapChain();
        bool resizeSwapChain(uint32_t width, uint32_t height);
        void setSwapChain();
        void clearRenderTarget(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
        void clearDepthBuffer(float depth = 1.0f, uint8_t stencil = 0u);
        bool updateSwapChain(bool vsync = true);
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
