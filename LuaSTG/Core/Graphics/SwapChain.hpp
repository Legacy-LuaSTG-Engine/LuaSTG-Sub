#pragma once
#include "Core/Type.hpp"
#include "Core/Graphics/Window.hpp"
#include "Core/Graphics/Format.hpp"
#include "Core/Graphics/Device.hpp"

namespace Core::Graphics
{
	struct DisplayMode
	{
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		Rational refresh_rate;
		Format format{ Format::B8G8R8A8_UNORM };
	};

	enum class SwapChainScalingMode
	{
		Stretch,
		AspectRatio,
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

		virtual bool setWindowMode(Vector2U size) = 0;
		virtual bool setCanvasSize(Vector2U size) = 0;
		virtual Vector2U getCanvasSize() = 0;

		virtual void setScalingMode(SwapChainScalingMode mode) = 0;
		virtual SwapChainScalingMode getScalingMode() = 0;

		virtual void clearRenderAttachment() = 0;
		virtual void applyRenderAttachment() = 0;
		virtual void waitFrameLatency() = 0;
		virtual void setVSync(bool enable) = 0;
		virtual bool present() = 0;

		virtual bool saveSnapshotToFile(StringView path) = 0;

		static bool create(IWindow* p_window, IDevice* p_device, ISwapChain** pp_swapchain);
	};
}
