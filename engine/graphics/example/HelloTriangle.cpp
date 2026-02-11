#include "core/Application.hpp"
#include "core/FrameRateController.hpp"
#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/Window.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/Graphics/SwapChain.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "win32/base.hpp"

namespace {
	using std::string_view_literals::operator ""sv;

	class HelloTriangleApplication : public core::IApplication {
	public:
		bool onCreate() override {
			core::Logger::info("[app] create"sv);

			if (!core::IWindow::create(m_window.put())) {
				return false;
			}
			if (!core::IGraphicsDevice::create(""sv, m_graphics_device.put())) {
				return false;
			}
			if (!core::Graphics::ISwapChain::create(m_window.get(), m_graphics_device.get(), m_swap_chain.put())) {
				return false;
			}

			m_window->setWindowMode(core::Vector2U(640, 480));
			m_swap_chain->setWindowMode(core::Vector2U(640, 480));

			return true;
		}
		void onBeforeUpdate() override {
			m_frame_rate_controller->update();
			m_swap_chain->waitFrameLatency();
		}
		bool onUpdate() override {
			m_swap_chain->applyRenderAttachment();
			m_swap_chain->present();
			return true;
		}
		void onDestroy() override {
			core::Logger::info("[app] destroy"sv);
		}

	private:
		core::IFrameRateController* m_frame_rate_controller{ core::IFrameRateController::getInstance() };
		core::SmartReference<core::IWindow> m_window;
		core::SmartReference<core::IGraphicsDevice> m_graphics_device;
		core::SmartReference<core::Graphics::ISwapChain> m_swap_chain;
	};
}

int main() {
	const auto logger = spdlog::stdout_color_mt("main", spdlog::color_mode::always);
	spdlog::set_default_logger(logger);
	win32::set_logger_writer([](const std::string_view message) -> void {
		spdlog::error(message);
	});

	HelloTriangleApplication app;
	core::ApplicationManager::run(&app);

	return 0;
}
