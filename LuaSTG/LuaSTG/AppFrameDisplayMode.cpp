#include "AppFrame.h"
#include "resource.h"

namespace LuaSTGPlus
{
	static std::string const MODE_NAME_WINDOW("窗口");
	static std::string const MODE_NAME_FULLSCREEN("全屏");

	inline std::string_view getFullscreenTypeString(ApplicationSetting const& setting)
	{
		if (setting.fullscreen)
			return MODE_NAME_FULLSCREEN;
		else
			return MODE_NAME_WINDOW;
	}

	inline void logResult(bool ok, ApplicationSetting const& from_mode, std::string_view to_mode)
	{
		if (ok)
			spdlog::info("[luastg] 显示模式切换成功：{} -> {}", getFullscreenTypeString(from_mode), to_mode);
		else
			spdlog::error("[luastg] 显示模式切换失败：{} -> {}", getFullscreenTypeString(from_mode), to_mode);
	}

	bool AppFrame::SetDisplayModeWindow(Core::Vector2U window_size, bool vsync)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		swapchain->setVSync(vsync);
		bool const result = swapchain->setCanvasSize(window_size);

		window->setWindowMode(window_size);
		window->setNativeIcon((void*)(ptrdiff_t)IDI_APPICON);

		logResult(result, m_Setting, MODE_NAME_WINDOW);

		m_Setting.canvas_size = window_size;
		m_Setting.fullscreen = false;
		m_Setting.vsync = vsync;
		
		return result;
	}

	// TODO: 废弃
	bool AppFrame::SetDisplayModeExclusiveFullscreen(Core::Vector2U window_size, bool vsync, Core::Rational)
	{
		auto* window = GetAppModel()->getWindow();
		auto* swapchain = GetAppModel()->getSwapChain();

		swapchain->setVSync(vsync);
		bool const result = swapchain->setCanvasSize(window_size);

		window->setWindowMode(window_size);
		window->setFullScreenMode();

		logResult(result, m_Setting, MODE_NAME_FULLSCREEN);

		m_Setting.canvas_size = window_size;
		m_Setting.fullscreen = true;
		m_Setting.vsync = vsync;

		return result;
	}

	bool AppFrame::UpdateDisplayMode()
	{
		if (m_Setting.fullscreen)
			return SetDisplayModeExclusiveFullscreen(
				m_Setting.canvas_size,
				m_Setting.vsync,
				Core::Rational());
		else
			return SetDisplayModeWindow(
				m_Setting.canvas_size,
				m_Setting.vsync);
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
			p_window->setSize(m_Setting.canvas_size);
			p_window->setWindowCornerPreference(m_Setting.allow_windows_11_window_corner);
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
		auto* p_swapchain = GetAppModel()->getSwapChain();
		// 先初始化交换链
		bool const result = p_swapchain->setWindowMode(m_Setting.canvas_size);
		if (!result) return false;
		// 正式应用显示模式
		UpdateDisplayMode();
		// 先刷新一下画面，避免白屏
		p_swapchain->clearRenderAttachment();
		p_swapchain->present();
		return true;
	}

	void AppFrame::SetWindowed(bool v)
	{
		if (m_iStatus == AppStatus::Initializing)
		{
			m_Setting.fullscreen = !v;
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
			m_Setting.vsync = v;
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
			m_Setting.canvas_size = Core::Vector2U(width, height);
		}
		else if (m_iStatus == AppStatus::Running)
			spdlog::warn("[luastg] SetResolution: 试图在运行时更改分辨率");
	}
}
