#pragma once
#include "Core/Object.hpp"
#include "Core/ApplicationModel.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/SwapChain_D3D11.hpp"

namespace LuaSTG::Core
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

	class ApplicationModel_Win32 : public Object<IApplicationModel>
	{
	private:
		// 多个线程共享

		ScopeObject<Graphics::Window_Win32> m_window;
		Microsoft::WRL::Wrappers::Event win32_event_exit;

		// 仅限工作线程

		ScopeObject<Graphics::Device_D3D11> m_device;
		ScopeObject<Graphics::SwapChain_D3D11> m_swapchain;
		FrameRateController m_ratelimit;
		IApplicationEventListener* m_listener{ nullptr };

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

		// 仅限主线程

		bool run();

	public:
		ApplicationModel_Win32(IApplicationEventListener* p_listener);
		~ApplicationModel_Win32();
	};
}
