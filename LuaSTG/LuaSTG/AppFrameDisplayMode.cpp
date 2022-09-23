#include "AppFrame.h"
#include "resource.h"

namespace LuaSTGPlus
{
	static std::string const MODE_NAME_WINDOW("窗口");
	static std::string const MODE_NAME_FULLSCREEN("全屏无边框窗口");
	static std::string const MODE_NAME_EX_FULLSCREEN("独占全屏");

	inline bool isRationalEmpty(Core::Rational const& rational)
	{
		return rational.numerator == 0 || rational.denominator == 0;
	}

	inline bool isRectEmpty(Core::RectI const& rect)
	{
		return (rect.b.x - rect.a.x) > 0
			&& (rect.b.y - rect.a.y) > 0
			;
	}

	inline bool isRectSameSize(Core::RectI const& rect1, Core::RectI const& rect2)
	{
		return (rect1.b.x - rect1.a.x) == (rect2.b.x - rect2.a.x)
			&& (rect1.b.y - rect1.a.y) == (rect2.b.y - rect2.a.y)
			;
	}

	inline uint32_t matchMonitorIndex(Core::Graphics::IWindow* window, Core::RectI const& monitor_rect, bool& find)
	{
		uint32_t const count = window->getMonitorCount();
		// Stage 1: Match Same Rect
		for (uint32_t i = 0; i < count; i += 1)
		{
			Core::RectI const rect = window->getMonitorRect(i);
			if (rect == monitor_rect)
			{
				find = true;
				return i;
			}
		}
		// Stage 2: Match Same Size
		for (uint32_t i = 0; i < count; i += 1)
		{
			Core::RectI const rect = window->getMonitorRect(i);
			if (isRectSameSize(rect, monitor_rect))
			{
				find = true;
				return i;
			}
		}
		// Stage 3: fallback
		find = false;
		return 0;
	}

	inline Core::Vector2I getMonitorSize(Core::Graphics::IWindow* window, uint32_t index)
	{
		Core::RectI const rect = window->getMonitorRect(index);
		return Core::Vector2I(rect.b.x - rect.a.x, rect.b.y - rect.a.y);
	}

	inline std::string_view getFullscreenTypeString(ApplicationSetting const& setting)
	{
		switch (setting.display_setting_type)
		{
		case DisplaySettingType::Window:
			return MODE_NAME_WINDOW;
		case DisplaySettingType::Fullscreen:
			return MODE_NAME_FULLSCREEN;
		case DisplaySettingType::ExclusiveFullscreen:
			return MODE_NAME_EX_FULLSCREEN;
		default:
			assert(false); // 不应该发生
			return MODE_NAME_WINDOW;
		}
	}

	inline void logResult(bool ok, ApplicationSetting const& from_mode, std::string_view to_mode)
	{
		if (ok)
			spdlog::info("[luastg] 显示模式切换成功：{} -> {}", getFullscreenTypeString(from_mode), to_mode);
		else
			spdlog::error("[luastg] 显示模式切换失败：{} -> {}", getFullscreenTypeString(from_mode), to_mode);
	}

	bool AppFrame::SetDisplayModeWindow(Core::Vector2I window_size, bool vsync, Core::RectI monitor_rect, bool borderless, bool swapchain_flip, bool swapchain_low_latency)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		swapchain->setVSync(vsync);
		bool const result = swapchain->setWindowMode(window_size.x, window_size.y, swapchain_flip, swapchain_low_latency);

		window->setLayer(Core::Graphics::WindowLayer::Normal); // 强制取消窗口置顶
		window->setFrameStyle(borderless ? Core::Graphics::WindowFrameStyle::None : Core::Graphics::WindowFrameStyle::Fixed);
		window->setSize(window_size);
		if (isRectEmpty(monitor_rect))
		{
			window->setCentered();
		}
		else
		{
			bool find_result = false;
			uint32_t const index = matchMonitorIndex(window, monitor_rect, find_result);
			std::ignore = find_result; // 对于窗口模式，即使找不到对应的显示器用于居中也无所谓
			window->setMonitorCentered(index);
		}
		if (!borderless)
		{
			window->setNativeIcon((void*)(ptrdiff_t)IDI_APPICON);
		}
		
		logResult(result, m_Setting, MODE_NAME_WINDOW);

		m_Setting.display_setting_type = DisplaySettingType::Window;
		m_Setting.window.window_size = window_size;
		m_Setting.window.monitor_rect = monitor_rect;
		m_Setting.window.vsync = vsync;
		m_Setting.window.borderless = borderless;
		m_Setting.window.swapchain_flip = swapchain_flip;
		m_Setting.window.swapchain_low_latency = swapchain_low_latency;

		m_Setting.toggle_between_fullscreen_type = DisplaySettingType::Window;

		return result;
	}

	bool AppFrame::SetDisplayModeFullscreen(Core::RectI monitor_rect, bool vsync, bool swapchain_flip, bool swapchain_low_latency)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		if (isRectEmpty(monitor_rect))
		{
			// 对于全屏无边框窗口模式，显示器矩形为空，将会失败
			logResult(false, m_Setting, MODE_NAME_FULLSCREEN);
			return false;
		}

		bool find_result = false;
		uint32_t const index = matchMonitorIndex(window, monitor_rect, find_result);
		if (!find_result)
		{
			// 对于全屏无边框窗口模式，如果找不到对应的显示器，将会失败
			logResult(find_result, m_Setting, MODE_NAME_FULLSCREEN);
			return false;
		}
		Core::Vector2I const window_size = getMonitorSize(window, index);

		swapchain->setVSync(vsync);
		bool const result = swapchain->setWindowMode(window_size.x, window_size.y, swapchain_flip, swapchain_low_latency);

		window->setLayer(Core::Graphics::WindowLayer::Normal); // 强制取消窗口置顶
		window->setFrameStyle(Core::Graphics::WindowFrameStyle::None);
		//window->setSize(size); // 这个其实没必要调用……
		window->setMonitorFullScreen(index);

		logResult(result, m_Setting, MODE_NAME_FULLSCREEN);

		m_Setting.display_setting_type = DisplaySettingType::Fullscreen;
		m_Setting.fullscreen.window_size = window_size;
		m_Setting.fullscreen.monitor_rect = monitor_rect;
		m_Setting.fullscreen.vsync = vsync;
		m_Setting.fullscreen.swapchain_flip = swapchain_flip;
		m_Setting.fullscreen.swapchain_low_latency = swapchain_low_latency;

		m_Setting.toggle_between_fullscreen_type = DisplaySettingType::Fullscreen;

		return result;
	}

	bool AppFrame::SetDisplayModeExclusiveFullscreen(Core::Vector2I window_size, bool vsync, Core::Rational refresh_rate)
	{
		auto* window = GetAppModel()->getWindow();

		//window->setLayer(Core::Graphics::WindowLayer::Normal); // 不需要调用
		window->setFrameStyle(Core::Graphics::WindowFrameStyle::None);
		window->setSize(window_size);
		//window->setCentered(); // 不需要调用

		auto* swapchain = GetAppModel()->getSwapChain();

		Core::Graphics::DisplayMode display_mode = {
			.width = (uint32_t)window_size.x,
			.height = (uint32_t)window_size.y,
			.refresh_rate = refresh_rate,
			.format = Core::Graphics::Format::B8G8R8A8_UNORM, // TODO: 未使用
		};
		if (isRationalEmpty(display_mode.refresh_rate))
		{
			display_mode.refresh_rate.numerator = m_Setting.target_fps;
			display_mode.refresh_rate.denominator = 1;
			if (!swapchain->findBestMatchDisplayMode(display_mode))
			{
				// 无法找到可用的独占全屏显示模式
				logResult(false, m_Setting, MODE_NAME_EX_FULLSCREEN);
				return false;
			}
		}

		swapchain->setVSync(vsync);
		bool const result = swapchain->setExclusiveFullscreenMode(display_mode);

		logResult(result, m_Setting, MODE_NAME_EX_FULLSCREEN);

		m_Setting.display_setting_type = DisplaySettingType::ExclusiveFullscreen;
		m_Setting.exclusive_fullscreen.window_size = window_size;
		m_Setting.exclusive_fullscreen.refresh_rate = refresh_rate;
		m_Setting.exclusive_fullscreen.vsync = vsync;

		//m_Setting.toggle_between_fullscreen_type = DisplaySettingType::?; // 不需要改动它

		return result;
	}

	Core::Vector2I AppFrame::GetCurrentWindowSize()
	{
		switch (m_Setting.display_setting_type)
		{
		case DisplaySettingType::Window:
			return m_Setting.window.window_size;
		case DisplaySettingType::Fullscreen:
			return m_Setting.fullscreen.window_size;
		case DisplaySettingType::ExclusiveFullscreen:
			return m_Setting.exclusive_fullscreen.window_size;
		default:
			assert(false); return Core::Vector2I(); // 不应该发生
		}
	}

	bool AppFrame::UpdateDisplayMode()
	{
		switch (m_Setting.display_setting_type)
		{
		case DisplaySettingType::Window:
			return SetDisplayModeWindow(
				m_Setting.window.window_size,
				m_Setting.window.vsync,
				m_Setting.window.monitor_rect,
				m_Setting.window.borderless,
				m_Setting.window.swapchain_flip,
				m_Setting.window.swapchain_low_latency);
		case DisplaySettingType::Fullscreen:
			return SetDisplayModeFullscreen(
				m_Setting.fullscreen.monitor_rect,
				m_Setting.fullscreen.vsync,
				m_Setting.fullscreen.swapchain_flip,
				m_Setting.fullscreen.swapchain_low_latency);
		case DisplaySettingType::ExclusiveFullscreen:
			return SetDisplayModeExclusiveFullscreen(
				m_Setting.exclusive_fullscreen.window_size,
				m_Setting.exclusive_fullscreen.vsync,
				m_Setting.exclusive_fullscreen.vsync);
		default:
			assert(false); return false; // 不应该发生
		}
	}

	bool AppFrame::ToggleBetweenFullscreen()
	{
		if (m_Setting.toggle_between_fullscreen_type == DisplaySettingType::Window)
		{
			if (m_Setting.display_setting_type == DisplaySettingType::Window)
			{
				// 从窗口模式切换到独占全屏
				m_Setting.display_setting_type = DisplaySettingType::ExclusiveFullscreen;
				m_Setting.exclusive_fullscreen.window_size = m_Setting.window.window_size;
				m_Setting.exclusive_fullscreen.vsync       = m_Setting.window.vsync;
				return UpdateDisplayMode();
			}
			else if (m_Setting.display_setting_type == DisplaySettingType::ExclusiveFullscreen)
			{
				// 从独占全屏返回窗口模式
				m_Setting.display_setting_type = DisplaySettingType::Window;
				m_Setting.window.window_size = m_Setting.exclusive_fullscreen.window_size;
				m_Setting.window.vsync       = m_Setting.exclusive_fullscreen.vsync;
				return UpdateDisplayMode();
			}
			else
			{
				assert(false); return false; // 这不应该发生
			}
		}
		else if (m_Setting.toggle_between_fullscreen_type == DisplaySettingType::Fullscreen)
		{
			if (m_Setting.display_setting_type == DisplaySettingType::Fullscreen)
			{
				// 从全屏无边框窗口进入独占全屏
				m_Setting.display_setting_type = DisplaySettingType::ExclusiveFullscreen;
				m_Setting.exclusive_fullscreen.window_size = m_Setting.fullscreen.window_size;
				m_Setting.exclusive_fullscreen.vsync       = m_Setting.fullscreen.vsync;
				return UpdateDisplayMode();
			}
			else if (m_Setting.display_setting_type == DisplaySettingType::ExclusiveFullscreen)
			{
				// 从独占全屏返回全屏无边框窗口
				if (m_Setting.fullscreen.window_size == m_Setting.exclusive_fullscreen.window_size)
				{
					// 正常返回到全屏无边框窗口
					m_Setting.display_setting_type = DisplaySettingType::Fullscreen;
					m_Setting.fullscreen.vsync = m_Setting.exclusive_fullscreen.vsync;
					return UpdateDisplayMode();
				}
				else
				{
					// 大小不再匹配，衰减到窗口模式
					m_Setting.display_setting_type = DisplaySettingType::Window;
					m_Setting.window.window_size = m_Setting.exclusive_fullscreen.window_size;
					m_Setting.window.vsync       = m_Setting.exclusive_fullscreen.vsync;
					return UpdateDisplayMode();
				}
			}
			else
			{
				assert(false); return false; // 这不应该发生
			}
		}
		else
		{
			assert(false); return false; // 这不应该发生
		}
	}

	bool AppFrame::InitializationApplySettingStage1()
	{
		// 配置窗口
		{
			using namespace Core::Graphics;
			auto* p_window = m_pAppModel->getWindow();
			p_window->setTitleText(m_Setting.window_title);
			p_window->setCursor(m_Setting.show_cursor ? WindowCursor::Arrow : WindowCursor::None);
			p_window->setNativeIcon((void*)(ptrdiff_t)IDI_APPICON);
			switch (m_Setting.display_setting_type)
			{
			case DisplaySettingType::Window:
				p_window->setFrameStyle(m_Setting.window.borderless ? WindowFrameStyle::None : WindowFrameStyle::Fixed);
				p_window->setSize(m_Setting.window.window_size);
				break;
			case DisplaySettingType::Fullscreen:
				p_window->setFrameStyle(WindowFrameStyle::None);
				p_window->setSize(m_Setting.fullscreen.window_size);
				break;
			case DisplaySettingType::ExclusiveFullscreen:
				p_window->setFrameStyle(WindowFrameStyle::None);
				p_window->setSize(m_Setting.exclusive_fullscreen.window_size);
				break;
			default:
				assert(false); return false;
			}
		}
		// 配置音量
		{
			using namespace Core::Audio;
			auto* p_audio = GetAppModel()->getAudioDevice();
			p_audio->setMixChannelVolume(MixChannel::SoundEffect, m_Setting.volume_sound_effect);
			p_audio->setMixChannelVolume(MixChannel::Music, m_Setting.volume_music);
		}
		return true;
	}

	bool AppFrame::InitializationApplySettingStage2()
	{
		// 显示窗口
		{
			using namespace Core::Graphics;
			auto* p_window = m_pAppModel->getWindow();
			p_window->setLayer(WindowLayer::Normal); // 显示窗口
		}
		// 启动交换链
		{
			using namespace Core::Graphics;
			auto* p_swapchain = m_pAppModel->getSwapChain();
			// 无论如何，首先初始化交换链，这样下面 findBestMatchDisplayMode 的时候才有有效的交换链对象
			Core::Vector2I window_size;
			switch (m_Setting.display_setting_type)
			{
			case DisplaySettingType::Window:
				window_size = m_Setting.window.window_size;
				p_swapchain->setVSync(m_Setting.window.vsync);
				break;
			case DisplaySettingType::Fullscreen:
				window_size = m_Setting.fullscreen.window_size;
				p_swapchain->setVSync(m_Setting.fullscreen.vsync);
				break;
			case DisplaySettingType::ExclusiveFullscreen:
				window_size = m_Setting.exclusive_fullscreen.window_size;
				p_swapchain->setVSync(m_Setting.exclusive_fullscreen.vsync);
				break;
			default:
				assert(false); return false;
			}
			if (!p_swapchain->setWindowMode(window_size.x, window_size.y, false, false))
				return false;
			// 正式应用显示模式
			UpdateDisplayMode();
			p_swapchain->refreshDisplayMode();
			// 先刷新一下画面，避免白屏
			p_swapchain->clearRenderAttachment();
			p_swapchain->present();
		}
		return true;
	}

	void AppFrame::SetWindowed(bool v)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			m_Setting.display_setting_type = v ? DisplaySettingType::Window : DisplaySettingType::ExclusiveFullscreen;
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
			m_Setting.window.vsync = v;
			m_Setting.fullscreen.vsync = v;
			m_Setting.exclusive_fullscreen.vsync = v;
		}
		else if (m_iStatus == AppStatus::Running)
		{
			spdlog::warn("[luastg] SetVsync: 试图在运行时更改垂直同步模式");
		}
	}

	void AppFrame::SetResolution(uint32_t width, uint32_t height, uint32_t A, uint32_t B)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			m_Setting.window.window_size = Core::Vector2I(width, height);
			m_Setting.fullscreen.window_size = Core::Vector2I(width, height);
			m_Setting.exclusive_fullscreen.window_size = Core::Vector2I(width, height);
			m_Setting.exclusive_fullscreen.refresh_rate = Core::Rational(A, B);
		}
		else if (m_iStatus == AppStatus::Running)
			spdlog::warn("[luastg] SetResolution: 试图在运行时更改分辨率");
	}
}
