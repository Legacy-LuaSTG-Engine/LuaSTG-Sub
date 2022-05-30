#pragma once
#include "Core/Type.hpp"
#include "Core/Graphics/Window.hpp"
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTG::Core
{
    struct IFrameRateController
    {
        virtual uint32_t getTargetFPS() = 0;
        virtual void setTargetFPS(uint32_t target_FPS) = 0;
        virtual double getFPS() = 0;
        virtual uint64_t getTotalFrame() = 0;
        virtual double getTotalTime() = 0;
        virtual double getAvgFPS() = 0;
        virtual double getMinFPS() = 0;
        virtual double getMaxFPS() = 0;
    };

    struct IApplicationEventListener
    {
        // [工作线程]
        virtual void onUpdate() {}
        // [工作线程]
        virtual void onRender() {}
    };

    struct FrameStatistics
    {
        double total_time{};
        double wait_time{};
        double update_time{};
        double render_time{};
        double present_time{};
    };

    struct IApplicationModel : public IObject
    {
        // [工作线程]
        virtual IFrameRateController* getFrameRateController() = 0;
        // [主线程|工作线程]
        virtual Graphics::IWindow* getWindow() = 0;
        // [工作线程]
        virtual Graphics::IDevice* getDevice() = 0;
        // [工作线程]
        virtual Graphics::ISwapChain* getSwapChain() = 0;
        // [工作线程]
        virtual Graphics::IRenderer* getRenderer() = 0;
        // [工作线程]
        virtual FrameStatistics getFrameStatistics() = 0;

        // [主线程|工作线程]
        virtual void requestExit() = 0;
        // [主线程]
        virtual bool run() = 0;

        static bool create(IApplicationEventListener* p_app, IApplicationModel** pp_model);
    };
}
