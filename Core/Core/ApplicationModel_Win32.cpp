#include "Core/ApplicationModel_Win32.hpp"
#include "Core/i18n.hpp"
#include "platform/WindowsVersion.hpp"
#include "platform/DetectCPU.hpp"

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
			spdlog::error("[fancy2d] 无法获取系统内存使用情况");
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
		TracyD3D11Collect(tracy::xTracyD3D11Ctx());
		FrameMark;
		while (true)
		{
			size_t const i = (m_framestate_index + 1) % 2;
			FrameStatistics& d = m_framestate[i];
			ScopeTimer gt(d.total_time);

			// 等待下一帧
			{
				ZoneScopedN("OnWait");
				ScopeTimer t(d.wait_time);
				// 如果需要退出
				if (WAIT_OBJECT_0 == WaitForSingleObjectEx(win32_event_exit.Get(), 0, TRUE))
				{
					break;
				}
				m_swapchain->waitFrameLatency();
				m_ratelimit.update();
			}

			// 更新
			{
				ZoneScopedN("OnUpdate");
				ScopeTimer t(d.update_time);
				m_listener->onUpdate();
				m_swapchain->syncWindowActive();
			}
			
			// 渲染
			{
				ZoneScopedN("OnRender");
				TracyD3D11Zone(tracy::xTracyD3D11Ctx(), "OnRender");
				ScopeTimer t(d.render_time);
				m_swapchain->applyRenderAttachment();
				m_swapchain->clearRenderAttachment();
				m_listener->onRender();
			}
			
			// 呈现
			{
				ZoneScopedN("OnPresent");
				ScopeTimer t(d.present_time);
				m_swapchain->present();
				TracyD3D11Collect(tracy::xTracyD3D11Ctx());
			}
			
			m_framestate_index = i;
			FrameMark;
		}
	}
	
	FrameStatistics ApplicationModel_Win32::getFrameStatistics()
	{
		return m_framestate[m_framestate_index];
	}

	void ApplicationModel_Win32::requestExit()
	{
		SetEvent(win32_event_exit.Get());
	}
	bool ApplicationModel_Win32::run()
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
					i18n_log_error_fmt("[core].system_call_failed_f", "MsgWaitForMultipleObjectsEx => WAIT_TIMEOUT");
					break;
				case WAIT_FAILED:
					gHRLastError;
					i18n_log_error_fmt("[core].system_call_failed_f", "MsgWaitForMultipleObjectsEx");
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
					i18n_log_error_fmt("[core].system_call_failed_f", "GetMessageW");
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

	ApplicationModel_Win32::ApplicationModel_Win32(ApplicationModelCreationParameters param, IApplicationEventListener* p_listener)
		: m_listener(p_listener)
	{
		assert(m_listener);
		spdlog::info("[core] System {}", platform::WindowsVersion::GetName());
		spdlog::info("[core] Kernel {}", platform::WindowsVersion::GetKernelVersionString());
		spdlog::info("[core] CPU {} {}", InstructionSet::Vendor(), InstructionSet::Brand());
		get_system_memory_status();
		if (!Graphics::Window_Win32::create(~m_window))
			throw std::runtime_error("Graphics::Window_Win32::create");
		if (!Graphics::Device_D3D11::create(param.gpu, ~m_device))
			throw std::runtime_error("Graphics::Device_D3D11::create");
		if (!Graphics::SwapChain_D3D11::create(*m_window, *m_device, ~m_swapchain))
			throw std::runtime_error("Graphics::SwapChain_D3D11::create");
		if (!Graphics::Renderer_D3D11::create(*m_device, ~m_renderer))
			throw std::runtime_error("Graphics::Renderer_D3D11::create");
		if (!Audio::Device_XAUDIO2::create(~m_audiosys))
			throw std::runtime_error("Audio::Device_XAUDIO2::create");
	}
	ApplicationModel_Win32::~ApplicationModel_Win32()
	{
		std::ignore = 0;
	}

	bool IApplicationModel::create(ApplicationModelCreationParameters param, IApplicationEventListener* p_app, IApplicationModel** pp_model)
	{
		try
		{
			*pp_model = new ApplicationModel_Win32(param, p_app);
			return true;
		}
		catch (...)
		{
			*pp_model = nullptr;
			return false;
		}
	}
}
