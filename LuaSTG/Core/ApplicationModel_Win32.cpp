#include "Core/ApplicationModel_Win32.hpp"
#include "Core/i18n.hpp"
#include "core/Configuration.hpp"
#include "Platform/WindowsVersion.hpp"
#include "Platform/DetectCPU.hpp"

namespace Core
{
	double FrameRateController::indexFPS(size_t idx)
	{
		return fps_[(fps_index_ + std::size(fps_) - 1 - idx) % std::size(fps_)];
	}

	double FrameRateController::udateData(int64_t curr)
	{
		// 更新各项数值
		double const fps = (double)freq_ / (double)(curr - last_);
		double const s = 1.0 / fps;
		total_frame_ += 1;
		total_time_ += s;
		fps_[fps_index_] = fps;
		fps_index_ = (fps_index_ + 1) % std::size(fps_);
		last_ = curr;

		// 更新统计数据
		fps_min_ = DBL_MAX;
		fps_max_ = -DBL_MAX;
		size_t const total_history = total_frame_ < std::size(fps_) ? (size_t)total_frame_ : std::size(fps_);
		if (total_history > 0)
		{
			double total_fps = 0.0;
			double total_time = 0.0;
			size_t history_count = 0;
			for (size_t i = 0; i < total_history; i += 1)
			{
				double const fps_history = indexFPS(i);
				total_fps += fps_history;
				fps_min_ = std::min(fps_min_, fps_history);
				fps_max_ = std::max(fps_max_, fps_history);
				total_time += 1.0 / fps_history;
				history_count += 1;
				if (total_time >= 0.25)
				{
					break;
				}
			}
			fps_avg_ = total_fps / (double)history_count;
		}
		else
		{
			fps_avg_ = 0.0;
			fps_min_ = 0.0;
			fps_max_ = 0.0;
		}

		return s;
	}
	bool FrameRateController::arrive()
	{
		// 先获取当前计数器
		LARGE_INTEGER curr_{};
		QueryPerformanceCounter(&curr_);
		// 判断
		if ((curr_.QuadPart - last_) < wait_)
		{
			return false;
		}
		else
		{
			udateData(curr_.QuadPart);
			return true;
		}
	}
	double FrameRateController::update()
	{
		// 先获取当前计数器
		LARGE_INTEGER curr_{};
		QueryPerformanceCounter(&curr_);

		// 当设备运行时间足够长后，即使是 int64 也会溢出
		if (curr_.QuadPart < last_)
		{
			total_frame_ += 1;
			total_time_ += 1.0 / indexFPS(0);
			last_ = curr_.QuadPart;
			return indexFPS(0);
		}

		// 在启用了高精度计时器的情况下，可以用 Sleep 等待，不需要占用太多 CPU
		LONGLONG const sleep_ms = (((wait_ - (curr_.QuadPart - last_)) - _2ms_) * 1000ll) / freq_;
		if (sleep_ms > 0)
		{
			Sleep((DWORD)sleep_ms);
		}

		// 轮询等待
		do
		{
			QueryPerformanceCounter(&curr_);
		} while ((curr_.QuadPart - last_) < wait_);

		return udateData(curr_.QuadPart);
	}

	uint32_t FrameRateController::getTargetFPS()
	{
		return (uint32_t)target_fps_;
	}
	void FrameRateController::setTargetFPS(uint32_t target_FPS)
	{
		target_fps_ = (double)(target_FPS > 0 ? target_FPS : 1);
		LARGE_INTEGER lli{};
		QueryPerformanceFrequency(&lli);
		freq_ = lli.QuadPart;
		wait_ = (LONGLONG)((double)freq_ / target_fps_);
		_2ms_ = (2ll * freq_) / 1000ll;
	}
	double FrameRateController::getFPS()
	{
		return indexFPS(0);
	}
	uint64_t FrameRateController::getTotalFrame()
	{
		return total_frame_;
	}
	double FrameRateController::getTotalTime()
	{
		return total_time_;
	}
	double FrameRateController::getAvgFPS()
	{
		return fps_avg_;
	}
	double FrameRateController::getMinFPS()
	{
		return fps_min_;
	}
	double FrameRateController::getMaxFPS()
	{
		return fps_max_;
	}

	FrameRateController::FrameRateController(uint32_t target_FPS)
	{
		timeBeginPeriod(1);
		setTargetFPS(target_FPS);
		LARGE_INTEGER lli{};
		QueryPerformanceCounter(&lli);
		last_ = lli.QuadPart;
	}
	FrameRateController::~FrameRateController()
	{
		timeEndPeriod(1);
	}
}

namespace Core
{
	// 基于时间戳的帧率控制器
	// 当帧率有波动时，追赶或者等待更长时间

	inline int64_t winQPC()
	{
		LARGE_INTEGER ll = {};
		QueryPerformanceCounter(&ll);
		return ll.QuadPart;
	}
	inline int64_t winQPF()
	{
		LARGE_INTEGER ll = {};
		QueryPerformanceFrequency(&ll);
		return ll.QuadPart;
	}

	bool TimeStampFrameRateController::arrive()
	{
		// 计算下一个要到达的时间戳
		int64_t const target_pc_ = begin_pc_ + (frame_count_ + 1) * clock_pcpf_;
		int64_t cur_pc_ = winQPC();
		return cur_pc_ >= target_pc_;
	}
	double TimeStampFrameRateController::update()
	{
		// 计算下一个要到达的时间戳
		int64_t const target_pc_ = begin_pc_ + (frame_count_ + 1) * clock_pcpf_;
		int64_t cur_pc_ = winQPC();
		if (cur_pc_ > target_pc_)
		{
			// 已经超过
			if ((cur_pc_ - target_pc_) >= (clock_pcpf_ * 10))
			{
				// 落后超过 10 帧，放弃追赶，并设置新的基准点
				begin_pc_ = cur_pc_;
				frame_count_ = 0;
			}
			else
			{
				// 立即推进 1 帧
				frame_count_ += 1;
			}
		}
		else
		{
			// 当需要等待的时间 Tms 大于 2ms 时，用Sleep 等待 (T - 2)ms
			int64_t const err_pc_ = clock_freq_ * 2 / 1000;
			int64_t const ddt_pc_ = target_pc_ - cur_pc_;
			if (ddt_pc_ > err_pc_)
			{
				DWORD const ms_ = (DWORD)((ddt_pc_ - err_pc_) / (err_pc_ / 2));
				Sleep(ms_);
			}
			// 精确的轮询等待
			for (;;)
			{
				cur_pc_ = winQPC();
				if (cur_pc_ >= target_pc_)
				{
					break;
				}
			}
			// 推进 1 帧
			frame_count_ += 1;
		}
		// 刷新数值
		int64_t const delta_pc_ = cur_pc_ - last_pc_;
		delta_time_ = (double)delta_pc_ / (double)clock_freq_;
		last_pc_ = cur_pc_;
		return delta_time_;
	}

	uint32_t TimeStampFrameRateController::getTargetFPS()
	{
		return (uint32_t)target_fps_;
	}
	void TimeStampFrameRateController::setTargetFPS(uint32_t target_FPS)
	{
		if (std::abs(target_fps_ - (double)target_FPS) < 0.01)
		{
			return;
		}

		target_fps_ = (double)target_FPS;
		target_spf_ = 1.0 / target_fps_;

		clock_freq_ = winQPF();
		clock_pcpf_ = (int64_t)(target_spf_ * (double)clock_freq_);

		begin_pc_ = winQPC();
		last_pc_ = begin_pc_;
		frame_count_ = 0;
	}
	double TimeStampFrameRateController::getFPS() { return 1.0 / delta_time_; }
	uint64_t TimeStampFrameRateController::getTotalFrame() { return 0; }
	double TimeStampFrameRateController::getTotalTime() { return 0.0; }
	double TimeStampFrameRateController::getAvgFPS() { return getFPS(); }
	double TimeStampFrameRateController::getMinFPS() { return getFPS(); }
	double TimeStampFrameRateController::getMaxFPS() { return getFPS(); }

	TimeStampFrameRateController::TimeStampFrameRateController(uint32_t target_FPS)
	{
		timeBeginPeriod(1);
		setTargetFPS(target_FPS);
	}
	TimeStampFrameRateController::~TimeStampFrameRateController()
	{
		timeEndPeriod(1);
	}
}

namespace Core
{
	static std::string bytes_count_to_string(DWORDLONG size)
	{
		int count = 0;
		char buffer[64] = {};
		if (size < 1024llu) // B
		{
			count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
		}
		else if (size < (1024llu * 1024llu)) // KB
		{
			count = std::snprintf(buffer, 64, "%.2f KiB", (double)size / 1024.0);
		}
		else if (size < (1024llu * 1024llu * 1024llu)) // MB
		{
			count = std::snprintf(buffer, 64, "%.2f MiB", (double)size / 1048576.0);
		}
		else // GB
		{
			count = std::snprintf(buffer, 64, "%.2f GiB", (double)size / 1073741824.0);
		}
		return std::string(buffer, count);
	}
	static void get_system_memory_status()
	{
		MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
		if (GlobalMemoryStatusEx(&info))
		{
			spdlog::info("[core] 系统内存使用情况：\n"
				"    使用百分比：{}%\n"
				"    总物理内存：{}\n"
				"    剩余物理内存：{}\n"
				"    当前进程可提交内存限制：{}\n"
				"    当前进程剩余的可提交内存：{}\n"
				"    当前进程用户模式内存空间限制*1：{}\n"
				"    当前进程剩余的用户模式内存空间：{}\n"
				"        *1 此项反映此程序实际上能用的最大内存，在 32 位应用程序上此项一般为 2 GB，修改 Windows 操作系统注册表后可能为 1 到 3 GB"
				, info.dwMemoryLoad
				, bytes_count_to_string(info.ullTotalPhys)
				, bytes_count_to_string(info.ullAvailPhys)
				, bytes_count_to_string(info.ullTotalPageFile)
				, bytes_count_to_string(info.ullAvailPageFile)
				, bytes_count_to_string(info.ullTotalVirtual)
				, bytes_count_to_string(info.ullAvailVirtual)
			);
		}
		else
		{
			spdlog::error("[core] 无法获取系统内存使用情况");
		}
	}

	struct ScopeTimer
	{
		LARGE_INTEGER freq{};
		LARGE_INTEGER last{};
		double& t;
		ScopeTimer(double& v_ref) : t(v_ref)
		{
			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&last);
		}
		~ScopeTimer()
		{
			LARGE_INTEGER curr{};
			QueryPerformanceCounter(&curr);
			t = (double)(curr.QuadPart - last.QuadPart) / (double)freq.QuadPart;
		}
	};

	DWORD WINAPI ApplicationModel_Win32::win32_thread_worker_entry(LPVOID lpThreadParameter)
	{
		static_cast<ApplicationModel_Win32*>(lpThreadParameter)->worker();
		return 0;
	}
	void ApplicationModel_Win32::worker()
	{
		// 设置线程优先级为最高，并尽量让它运行在同一个 CPU 核心上，降低切换开销
		SetThreadAffinityMask(GetCurrentThread(), 1);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		// 更新、渲染循环
		TracyD3D11Collect(m_device->GetTracyContext());
		FrameMark;
		{
			tracy_zone_scoped_with_name("OnInitWait");
			m_swapchain->waitFrameLatency();
			m_p_frame_rate_controller->update();
		}
		while (true)
		{
			size_t const i = (m_framestate_index + 1) % 2;
			FrameStatistics& d = m_framestate[i];
			ScopeTimer gt(d.total_time);

			bool update_result = false;

			// 更新
			{
				tracy_zone_scoped_with_name("OnUpdate");
				ScopeTimer t(d.update_time);
				// 如果需要退出
				if (WAIT_OBJECT_0 == WaitForSingleObjectEx(win32_event_exit.Get(), 0, TRUE))
				{
					break;
				}
				update_result = m_listener->onUpdate();
			}
			
			bool render_result = false;

			// 渲染
			if (update_result)
			{
				tracy_zone_scoped_with_name("OnRender");
				tracy_d3d11_context_zone(m_device->GetTracyContext(), "OnRender");
				ScopeTimer t(d.render_time);
				m_swapchain->applyRenderAttachment();
				m_swapchain->clearRenderAttachment();
				render_result = m_listener->onRender();
			}
			
			// 呈现
			if (render_result)
			{
				tracy_zone_scoped_with_name("OnPresent");
				ScopeTimer t(d.present_time);
				m_swapchain->present();
				TracyD3D11Collect(m_device->GetTracyContext());
			}
			
			// 等待下一帧
			{
				tracy_zone_scoped_with_name("OnWait");
				ScopeTimer t(d.wait_time);
				m_swapchain->waitFrameLatency();
				m_p_frame_rate_controller->update();
			}

			m_framestate_index = i;
			FrameMark;
		}
	}
	
	bool ApplicationModel_Win32::runSingleThread()
	{
		// 设置线程优先级为高，并尽量让它运行在同一个 CPU 核心上，降低切换开销
		SetThreadAffinityMask(GetCurrentThread(), 1);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		// 初次收集诊断信息
		TracyD3D11Collect(m_device->GetTracyContext());
		FrameMark;
		{
			tracy_zone_scoped_with_name("OnInitWait");
			m_swapchain->waitFrameLatency();
			m_p_frame_rate_controller->update();
		}
		
		// 游戏循环
		MSG msg{};
		while (!m_exit_flag)
		{
			// 提取消息
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					m_exit_flag = true; // 应该结束循环
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}

			// 更新并渲染
			if (m_exit_flag)
			{
				break;
			}
			runFrame();
		}

		return true;
	}
	bool ApplicationModel_Win32::runDoubleThread()
	{
		// 设置线程优先级为稍高，并尽量让它运行在同一个 CPU 核心上，降低切换开销
		//SetThreadAffinityMask(GetCurrentThread(), 1);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		// 创建退出用的事件
		win32_event_exit.Attach(CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
		if (!win32_event_exit.IsValid())
		{
			return false;
		}
		// 创建更新、渲染线程
		Microsoft::WRL::Wrappers::ThreadHandle win32_thread_worker;
		win32_thread_worker.Attach(CreateThread(NULL, 0, &win32_thread_worker_entry, this, 0, NULL));
		if (!win32_thread_worker.IsValid())
		{
			return false;
		}
		// 消息循环
		MSG msg = {};
		BOOL running = TRUE;
		HANDLE win32_events[1] = { win32_event_exit.Get() };
		while (running)
		{
			if (win32_events[0])
			{
				// 监控退出事件和消息队列
				// MsgWaitForMultipleObjectsEx 在不带 MWMO_INPUTAVAILABLE 调用时，只会注意到“新”消息
				// 如果某些地方 PeekMessageW 不带 PM_REMOVE 调用，那么这个消息仍然存在于消息队列中，但是已经成为“老”消息
				// 为了防止这种情况卡住 MsgWaitForMultipleObjectsEx 造成无限等待：
				//   1、调用 MsgWaitForMultipleObjectsEx 时带上 MWMO_INPUTAVAILABLE
				//   2、使用 while 循环带 PM_REMOVE 调用 PeekMessageW 直到返回 FALSE
				switch (MsgWaitForMultipleObjectsEx(1, win32_events, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE))
				{
				case (WAIT_OBJECT_0):
					// 退出信号不是由窗口触发的，而是由工作线程触发的
					PostQuitMessage(EXIT_SUCCESS);
					win32_events[0] = NULL; // 不再需要监控退出事件
					break;
				case (WAIT_OBJECT_0 + 1):
					while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
					{
						if (msg.message == WM_QUIT)
						{
							running = FALSE; // 应该结束循环
						}
						else
						{
							TranslateMessage(&msg);
							DispatchMessageW(&msg);
						}
					}
					break;
				case WAIT_TIMEOUT:
					i18n_core_system_call_report_error("MsgWaitForMultipleObjectsEx => WAIT_TIMEOUT");
					break;
				case WAIT_FAILED:
					gHRLastError;
					i18n_core_system_call_report_error("MsgWaitForMultipleObjectsEx");
					running = FALSE; // 应该结束循环
					break;
				}
			}
			else
			{
				// 监控消息队列
				BOOL const result = GetMessageW(&msg, NULL, 0, 0);
				if (result >= TRUE)
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				else if (result == FALSE)
				{
					running = FALSE; // 应该结束循环
				}
				else
				{
					gHRLastError;
					i18n_core_system_call_report_error("GetMessageW");
					running = FALSE; // 应该结束循环
				}
			}
		}
		// 让工作线程退出
		requestExit();
		//WaitForSingleObjectEx(win32_thread_worker.Get(), INFINITE, TRUE);
		auto wait_worker = [&]()
		{
			for (;;)
			{
				switch (WaitForSingleObjectEx(win32_thread_worker.Get(), USER_TIMER_MINIMUM, TRUE))
				{
				case WAIT_OBJECT_0:
					return; // ok
				case WAIT_TIMEOUT:
					break; // msg
				case WAIT_FAILED:
					gHRLastError;
					return; // failed
				}
				// 你妈的，是不是还有消息没处理
				if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						// NOOP
					}
					else
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
				}
			}
		};
		wait_worker();
		return true;
	}
	void ApplicationModel_Win32::runFrame()
	{
		size_t const i = (m_framestate_index + 1) % 2;
		FrameStatistics& d = m_framestate[i];
		ScopeTimer gt(d.total_time);
		size_t const next_frame_query_index = (m_frame_query_index + 1) % m_frame_query_list.size();
		FrameQuery& frame_query = m_frame_query_list[next_frame_query_index];
		
		bool update_result = false;

		// 更新
		{
			tracy_zone_scoped_with_name("OnUpdate");
			ScopeTimer t(d.update_time);
			update_result = m_listener->onUpdate();
		}

		bool render_result = false;

		

		// 渲染
		if (update_result)
		{
			tracy_zone_scoped_with_name("OnRender");
			tracy_d3d11_context_zone(m_device->GetTracyContext(), "OnRender");
			ScopeTimer t(d.render_time);
			frame_query.begin(); // TODO: enable/disable by configuration
			m_swapchain->applyRenderAttachment();
			m_swapchain->clearRenderAttachment();
			render_result = m_listener->onRender();
			frame_query.end();  // TODO: enable/disable by configuration
		}

		// 呈现
		if (render_result)
		{
			tracy_zone_scoped_with_name("OnPresent");
			tracy_d3d11_context_zone(m_device->GetTracyContext(), "OnPresent");
			ScopeTimer t(d.present_time);
			m_swapchain->present();
			TracyD3D11Collect(m_device->GetTracyContext());
		}

		// 等待下一帧
		{
			tracy_zone_scoped_with_name("OnWait");
			ScopeTimer t(d.wait_time);
			m_swapchain->waitFrameLatency();
			m_p_frame_rate_controller->update();
		}

		m_framestate_index = i;
		m_frame_query_index = next_frame_query_index;
		FrameMark;
	}

	FrameStatistics ApplicationModel_Win32::getFrameStatistics()
	{
		return m_framestate[m_framestate_index];
	}
	FrameRenderStatistics ApplicationModel_Win32::getFrameRenderStatistics()
	{
		FrameQuery& frame_query = m_frame_query_list[m_frame_query_index];
		FrameRenderStatistics statistics{};
		statistics.render_time = frame_query.getTime();
		return statistics;
	}

	void ApplicationModel_Win32::requestExit()
	{
		SetEvent(win32_event_exit.Get());
		m_exit_flag = true;
	}
	bool ApplicationModel_Win32::run()
	{
		return runSingleThread();
	}

	ApplicationModel_Win32::ApplicationModel_Win32(IApplicationEventListener* p_listener)
		: m_listener(p_listener)
	{
		assert(m_listener);
		spdlog::info("[core] System {}", Platform::WindowsVersion::GetName());
		spdlog::info("[core] Kernel {}", Platform::WindowsVersion::GetKernelVersionString());
		spdlog::info("[core] CPU {} {}", InstructionSet::Vendor(), InstructionSet::Brand());
		if (m_steady_frame_rate_controller.available()) {
			spdlog::info("[core] High Resolution Waitable Timer available, enable SteadyFrameRateController");
			m_p_frame_rate_controller = &m_steady_frame_rate_controller;
		}
		else {
			m_p_frame_rate_controller = &m_frame_rate_controller;
		}
		get_system_memory_status();
		if (!Graphics::Window_Win32::create(~m_window))
			throw std::runtime_error("Graphics::Window_Win32::create");
		m_window->implSetApplicationModel(this);
		auto const& gpu = core::ConfigurationLoader::getInstance().getGraphicsSystem().getPreferredDeviceName();
		if (!Graphics::Device_D3D11::create(gpu, ~m_device))
			throw std::runtime_error("Graphics::Device_D3D11::create");
		if (!Graphics::SwapChain_D3D11::create(*m_window, *m_device, ~m_swapchain))
			throw std::runtime_error("Graphics::SwapChain_D3D11::create");
		if (!Graphics::Renderer_D3D11::create(*m_device, ~m_renderer))
			throw std::runtime_error("Graphics::Renderer_D3D11::create");
		if (!Audio::Device_XAUDIO2::create(~m_audiosys))
			throw std::runtime_error("Audio::Device_XAUDIO2::create");
		m_frame_query_list.reserve(2);
		for (int i = 0; i < 2; i += 1) {
			m_frame_query_list.emplace_back(m_device.get());
		}
	}
	ApplicationModel_Win32::~ApplicationModel_Win32()
	{
		std::ignore = 0;
	}

	bool IApplicationModel::create(IApplicationEventListener* p_app, IApplicationModel** pp_model)
	{
		try
		{
			*pp_model = new ApplicationModel_Win32(p_app);
			return true;
		}
		catch (...)
		{
			*pp_model = nullptr;
			return false;
		}
	}
}
