#pragma once
#include "Core/Type.hpp"
#include "Core/Graphics/Window.hpp"
#include "Core/Graphics/Format.hpp"
#include "Core/Graphics/Device.hpp"

namespace LuaSTG::Core::Graphics
{
	struct DisplayMode
	{
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		Rational refresh_rate;
		Format format{ Format::B8G8R8A8_UNORM };
	};

	struct ISwapChainEventListener
	{
		virtual void onSwapChainCreate() = 0;
		virtual void onSwapChainDestroy() = 0;
	};

	struct ISwapChain : public IObject
	{
		virtual void addEventListener(ISwapChainEventListener* e) = 0;
		virtual void removeEventListener(ISwapChainEventListener* e) = 0;

		virtual bool refreshDisplayMode() = 0;
		virtual uint32_t getDisplayModeCount() = 0;
		virtual DisplayMode getDisplayMode(uint32_t index) = 0;
		virtual bool findBestMatchDisplayMode(DisplayMode& mode) = 0; // 匹配最佳刷新率

		virtual bool setWindowMode(uint32_t width, uint32_t height, bool flip_model) = 0;
		virtual bool setSize(uint32_t width, uint32_t height) = 0; // 仅限窗口模式下
		virtual bool setExclusiveFullscreenMode(DisplayMode const& mode) = 0;
		virtual bool isWindowMode() = 0;
		virtual uint32_t getWidth() = 0;
		virtual uint32_t getHeight() = 0;

		virtual void clearRenderAttachment() = 0;
		virtual void applyRenderAttachment() = 0;
		virtual void waitFrameLatency() = 0;
		virtual void syncWindowActive() = 0;
		virtual void setVSync(bool enable) = 0;
		virtual bool present() = 0;

		static bool create(IWindow* p_window, IDevice* p_device, ISwapChain** pp_swapchain);
	};
}
