#pragma once
#include "Core/Type.hpp"
#include "Core/Graphics/Window.hpp"
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/SwapChain.hpp"
#include "Core/Graphics/Renderer.hpp"
#include "Core/Audio/Device.hpp"
#include "core/ReferenceCounted.hpp"

namespace core
{
    struct IFrameRateController
    {
        virtual double update() = 0;
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
        virtual bool onUpdate() { return true; }
        // [工作线程]
        virtual bool onRender() { return true; }
    };

    struct FrameStatistics
    {
        double total_time{};
        double wait_time{};
        double update_time{};
        double render_time{};
        double present_time{};
    };

    struct FrameRenderStatistics
    {
        double render_time{};
    };

    struct IApplicationModel : public IReferenceCounted
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
        virtual Audio::IAudioDevice* getAudioDevice() = 0;
        // [工作线程]
        virtual FrameStatistics getFrameStatistics() = 0;
        // [工作线程]
        virtual FrameRenderStatistics getFrameRenderStatistics() = 0;

        // [主线程|工作线程]
        virtual void requestExit() = 0;
        // [主线程]
        virtual bool run() = 0;

        static bool create(IApplicationEventListener* p_app, IApplicationModel** pp_model);
    };

    // UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IApplicationModel
	template<> constexpr InterfaceId getInterfaceId<IApplicationModel>() { return UUID::parse("42313368-4b16-511f-895f-ee43f0e10713"); }
}
