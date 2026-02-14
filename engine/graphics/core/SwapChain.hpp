#pragma once
#include "core/Vector2.hpp"
#include "core/Rational.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Window.hpp"
#include "core/GraphicsDevice.hpp"

namespace core {
	struct DisplayMode {
		uint32_t width{};
		uint32_t height{};
		Rational refresh_rate;
	};

	enum class SwapChainScalingMode {
		stretch,
		aspect_ratio,
	};

	struct ISwapChainEventListener {
		virtual void onSwapChainCreate() = 0;
		virtual void onSwapChainDestroy() = 0;
	};

	CORE_INTERFACE ISwapChain : IReferenceCounted {
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
		virtual bool getVSync() = 0;
		virtual bool present() = 0;

		virtual bool saveSnapshotToFile(StringView path) = 0;

		static bool create(IWindow* p_window, IGraphicsDevice* p_device, ISwapChain** pp_swapchain);
	};

	CORE_INTERFACE_ID(ISwapChain, "9036abca-4134-5258-9021-a79b7bfe5a58");
}
