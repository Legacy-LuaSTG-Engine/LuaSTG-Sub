#pragma once
#include "Core/Object.hpp"
#include "Core/ApplicationModel.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/SwapChain_D3D11.hpp"
#include "Core/Graphics/Renderer_D3D11.hpp"
#include "Core/Audio/Device_XAUDIO2.hpp"

namespace Core
{
	class FrameRateController : public IFrameRateController
	{
	private:
		int64_t freq_{};
		int64_t last_{};
		int64_t wait_{};
		int64_t _2ms_{};
		uint64_t total_frame_{};
		double total_time_{};
		double target_fps_{ 60.0 };
		double fps_[256]{};
		double fps_avg_{};
		double fps_min_{};
		double fps_max_{};
		size_t fps_index_{};
	private:
		double indexFPS(size_t idx);
	public:
		double udateData(int64_t curr);
		bool arrive();
		double update();
	public:
		uint32_t getTargetFPS();
		void setTargetFPS(uint32_t target_FPS);
		double getFPS();
		uint64_t getTotalFrame();
		double getTotalTime();
		double getAvgFPS();
		double getMinFPS();
		double getMaxFPS();
	public:
		FrameRateController(uint32_t target_FPS = 60);
		~FrameRateController();
	};

	class TimeStampFrameRateController : public IFrameRateController
	{
	private:
		double target_fps_{ 0.0 };
		double target_spf_{ 0.0 };
		int64_t clock_freq_{ 0 };
		int64_t clock_pcpf_{ 0 };
		double delta_time_{ 0.0 };
		int64_t begin_pc_{ 0 };
		int64_t last_pc_{ 0 };
		int64_t frame_count_{ 0 };
	public:
		bool arrive();
		double update();
	public:
		uint32_t getTargetFPS();
		void setTargetFPS(uint32_t target_FPS);
		double getFPS();
		uint64_t getTotalFrame();
		double getTotalTime();
		double getAvgFPS();
		double getMinFPS();
		double getMaxFPS();
	public:
		TimeStampFrameRateController(uint32_t target_FPS = 60);
		~TimeStampFrameRateController();
	};

	class ApplicationModel_Win32 : public Object<IApplicationModel>
	{
	private:
		// 多个线程共享

		ScopeObject<Graphics::Window_Win32> m_window;
		Microsoft::WRL::Wrappers::Event win32_event_exit;

		// 仅限工作线程

		ScopeObject<Graphics::Device_D3D11> m_device;
		ScopeObject<Graphics::SwapChain_D3D11> m_swapchain;
		ScopeObject<Graphics::Renderer_D3D11> m_renderer;
		ScopeObject<Audio::Device_XAUDIO2> m_audiosys;
		FrameRateController m_ratelimit;
		IApplicationEventListener* m_listener{ nullptr };
		size_t m_framestate_index{ 0 };
		FrameStatistics m_framestate[2]{};

		static DWORD WINAPI win32_thread_worker_entry(LPVOID lpThreadParameter);
		void worker();

	public:
		// 多个线程共享

		Graphics::IWindow* getWindow() { return *m_window; }
		void requestExit();

		// 仅限工作线程

		IFrameRateController* getFrameRateController() { return &m_ratelimit; };
		Graphics::IDevice* getDevice() { return *m_device; }
		Graphics::ISwapChain* getSwapChain() { return *m_swapchain; }
		Graphics::IRenderer* getRenderer() { return *m_renderer; }
		Audio::IAudioDevice* getAudioDevice() { return m_audiosys.get(); }
		FrameStatistics getFrameStatistics();

		// 仅限主线程

		bool run();

	public:
		ApplicationModel_Win32(ApplicationModelCreationParameters param, IApplicationEventListener* p_listener);
		~ApplicationModel_Win32();
	};
}
