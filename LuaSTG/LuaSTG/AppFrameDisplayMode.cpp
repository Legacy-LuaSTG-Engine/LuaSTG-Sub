#include "AppFrame.h"
#include "core/Configuration.hpp"
#include "resource.h"

namespace luastg
{
	bool AppFrame::SetDisplayModeWindow(core::Vector2U window_size, bool vsync)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		swapchain->setVSync(vsync);
		bool const result = swapchain->setCanvasSize(window_size);

		window->setWindowMode(window_size);

		return result;
	}

	// TODO: 废弃
	bool AppFrame::SetDisplayModeExclusiveFullscreen(core::Vector2U window_size, bool vsync, core::Rational)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		swapchain->setVSync(vsync);
		bool const result = swapchain->setCanvasSize(window_size);

		window->setWindowMode(window_size);
		window->setFullScreenMode();

		return result;
	}

	bool AppFrame::InitializationApplySettingStage1()
	{
		// 配置框架
		{
			auto const& timing = core::ConfigurationLoader::getInstance().getTiming();
			m_target_fps = std::max(1u, timing.getFrameRate());
		}
		// 配置窗口
		{
			auto const& gs = core::ConfigurationLoader::getInstance().getGraphicsSystem();
			auto const& win = core::ConfigurationLoader::getInstance().getWindow();
			using namespace core::Graphics;
			auto* p_window = m_pAppModel->getWindow();
			p_window->setTitleText(win.hasTitle() ? win.getTitle() : std::string(LUASTG_INFO));
			p_window->setCursor(win.isCursorVisible() ? WindowCursor::Arrow : WindowCursor::None);
			p_window->setNativeIcon((void*)(ptrdiff_t)IDI_APPICON);
			//p_window->setSize(core::Vector2U(gs.getWidth(), gs.getHeight()));
			//p_window->setCentered(false);
			//p_window->setFrameStyle(core::Graphics::WindowFrameStyle::Normal);
			p_window->setWindowCornerPreference(win.isAllowWindowCorner());
			p_window->setLayer(core::Graphics::WindowLayer::Invisible);
			p_window->setWindowMode(core::Vector2U(gs.getWidth(), gs.getHeight()));
		}
		return true;
	}

	bool AppFrame::InitializationApplySettingStage2()
	{
		auto const& gs = core::ConfigurationLoader::getInstance().getGraphicsSystem();
		auto* p_window = m_pAppModel->getWindow();
		auto* p_swapchain = GetAppModel()->getSwapChain();
		// 先初始化交换链
		bool const result = p_swapchain->setWindowMode(core::Vector2U(gs.getWidth(), gs.getHeight()));
		if (!result) return false;
		p_swapchain->setVSync(gs.isVsync());
		// 先刷新一下画面，避免白屏
		p_swapchain->clearRenderAttachment();
		p_swapchain->present();
		// 显示窗口
		p_window->setLayer(core::Graphics::WindowLayer::Top);
		if (gs.isFullscreen()) {
			p_window->setFullScreenMode();
		}
		return true;
	}

	void AppFrame::SetWindowed(bool v)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			auto& gs = core::ConfigurationLoader::getInstance().getGraphicsSystemRef();
			gs.setFullscreen(!v);
		}
		else if (m_iStatus == AppStatus::Running)
		{
			spdlog::warn("[luastg] SetWindowed: 试图在运行时更改窗口化模式");
		}
	}

	void AppFrame::SetVsync(bool v)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			auto& gs = core::ConfigurationLoader::getInstance().getGraphicsSystemRef();
			gs.setVsync(v);
		}
		else if (m_iStatus == AppStatus::Running)
		{
			spdlog::warn("[luastg] SetVsync: 试图在运行时更改垂直同步模式");
		}
	}

	void AppFrame::SetResolution(uint32_t width, uint32_t height)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			auto& gs = core::ConfigurationLoader::getInstance().getGraphicsSystemRef();
			gs.setWidth(width);
			gs.setHeight(height);
		}
		else if (m_iStatus == AppStatus::Running)
			spdlog::warn("[luastg] SetResolution: 试图在运行时更改分辨率");
	}
}
