#include "AppFrame.h"
#include "core/Configuration.hpp"
#include "resource.h"

namespace luastg
{
	bool AppFrame::SetDisplayModeWindow(core::Vector2U window_size, bool vsync)
	{
		m_swap_chain->setVSync(vsync);
		bool const result = m_swap_chain->setCanvasSize(window_size);

		m_window->setWindowMode(window_size);

		return result;
	}

	// TODO: 废弃
	bool AppFrame::SetDisplayModeExclusiveFullscreen(core::Vector2U window_size, bool vsync, core::Rational)
	{
		m_swap_chain->setVSync(vsync);
		bool const result = m_swap_chain->setCanvasSize(window_size);

		m_window->setWindowMode(window_size);
		m_window->setFullScreenMode();

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
			m_window->setCursor(win.isCursorVisible() ? core::Graphics::WindowCursor::Arrow : core::Graphics::WindowCursor::None);
			m_window->setWindowMode(core::Vector2U(gs.getWidth(), gs.getHeight()));
		}
		return true;
	}

	bool AppFrame::InitializationApplySettingStage2()
	{
		auto const& gs = core::ConfigurationLoader::getInstance().getGraphicsSystem();
		// 初始化交换链
		auto const canvas_size = core::Vector2U(gs.getWidth(), gs.getHeight());
		m_swap_chain->setVSync(gs.isVsync());
		if (!m_swap_chain->setWindowMode(canvas_size)) {
			return false;
		}
		if (gs.isFullscreen()) {
			m_window->setFullScreenMode();
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
