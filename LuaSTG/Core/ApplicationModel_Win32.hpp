#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/ApplicationModel.hpp"
#include "Core/Graphics/Window_Win32.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "Core/Graphics/SwapChain_D3D11.hpp"
#include "Core/Graphics/Renderer_D3D11.hpp"

namespace core
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

	// Windows 10 1803+ 开始可用的高精度帧率控制方法
	class SteadyFrameRateController : public IFrameRateController
	{
	private:
		uint32_t const m_max_error{ 10 }; // 相差超过 10 帧就不应该再追帧了
		double const m_update_avg_fps_delta{ 0.25 }; // 每 0.25s 更新一次平均帧率
		wil::unique_event_nothrow m_event;
		LARGE_INTEGER m_freq{};
		LARGE_INTEGER m_last{};
		LARGE_INTEGER m_target_time{}; // 下一个期望的时间戳
		LARGE_INTEGER m_target_delta{}; // 期望的时间戳间隔
		uint32_t m_target_frame_rate{}; // 期望的帧率
		double m_last_fps{};
		double m_last_max_fps{};
		double m_last_min_fps{};
		uint64_t m_total_frame{};
		double m_total_time{};
		std::array<double, 60> m_time_history{};
		size_t m_time_history_index{};
		double m_last_avg_fps{};
		double m_update_avg_fps_timer{};
	private:
		double updateTime()
		{
			LARGE_INTEGER v_curr{};
			QueryPerformanceCounter(&v_curr);
			double delta_s = double(v_curr.QuadPart - m_last.QuadPart) / double(m_freq.QuadPart);

			m_last_fps = 1.0 / delta_s;
			m_total_frame += 1;
			m_total_time += delta_s;
			m_time_history[m_time_history_index] = delta_s;
			m_time_history_index = (m_time_history_index + 1) % m_time_history.size();

			m_last_min_fps = std::numeric_limits<double>::max();
			m_last_max_fps = std::numeric_limits<double>::lowest();
			double total_s{};
			for (size_t i = 0; i < m_time_history.size(); i += 1) {
				size_t idx = (m_time_history_index + m_time_history.size() - 1 - i) % m_time_history.size();
				double fps = 1.0 / m_time_history[idx];
				m_last_min_fps = std::min(m_last_min_fps, fps);
				m_last_max_fps = std::max(m_last_max_fps, fps);
				total_s += m_time_history[idx];
			}

			// 每 0.25s 更新一次平均帧率
			m_update_avg_fps_timer -= delta_s;
			if (m_update_avg_fps_timer <= 0.0) {
				m_update_avg_fps_timer = m_update_avg_fps_delta;
				m_last_avg_fps = double(m_time_history.size()) / total_s;
			}
			
			m_last = v_curr;
			return delta_s;
		}
		bool recreateResource()
		{
			m_event.reset(CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS));
			if (!m_event) {
				spdlog::error("[core] [SteadyFrameRateController] CreateWaitableTimerExW failed (LastError={})", GetLastError());
			}
			return !!m_event;
		}
	public:
		bool arrive() { return true; }
		double update()
		{
			FILETIME file_time{};
			GetSystemTimeAsFileTime(&file_time);
			LARGE_INTEGER current_time{};
			current_time.LowPart = file_time.dwLowDateTime;
			current_time.HighPart = static_cast<LONG>(file_time.dwHighDateTime);

			// 如果相差太大，就不应该追帧了
			if (std::abs(current_time.QuadPart - (m_target_time.QuadPart + m_target_delta.QuadPart)) >= (m_target_delta.QuadPart * 10))
			{
				m_target_time.QuadPart = current_time.QuadPart;
				return updateTime(); // 立即放弃等待，尽快追上
			}

			// 等待到下个时间戳
			m_target_time.QuadPart += m_target_delta.QuadPart;
			if (!SetWaitableTimerEx(m_event.get(), &m_target_time, 0, NULL, NULL, NULL, 0))
			{
				spdlog::error("[core] [SteadyFrameRateController] SetWaitableTimerEx failed (LastError={})", GetLastError());
				recreateResource(); // 尝试重新创建资源
				return updateTime();
			}
			if (WAIT_FAILED == WaitForSingleObject(m_event.get(), INFINITE))
			{
				spdlog::error("[core] [SteadyFrameRateController] WaitForSingleObject failed (LastError={})", GetLastError());
				recreateResource(); // 尝试重新创建资源
				return updateTime();
			}

			return updateTime();
		}
	public:
		uint32_t getTargetFPS() { return m_target_frame_rate; }
		void setTargetFPS(uint32_t target_frame_rate)
		{
			if (target_frame_rate == m_target_frame_rate) {
				return;
			}
			m_target_frame_rate = std::max<uint32_t>(1, target_frame_rate); // 保护措施
			
			double const delta_s = 1.0 / double(target_frame_rate);
			m_target_delta.QuadPart = LONGLONG(delta_s * 10000000.0);

			FILETIME file_time{};
			GetSystemTimeAsFileTime(&file_time);
			m_target_time.LowPart = file_time.dwLowDateTime;
			m_target_time.HighPart = static_cast<LONG>(file_time.dwHighDateTime);
		}
		double getFPS() { return m_last_fps; }
		uint64_t getTotalFrame() { return m_total_frame; }
		double getTotalTime() { return m_total_time; }
		double getAvgFPS() { return m_last_avg_fps; }
		double getMinFPS() { return m_last_min_fps; }
		double getMaxFPS() { return m_last_max_fps; }
	public:
		bool available() { return !!m_event; }
	public:
		SteadyFrameRateController()
		{
			QueryPerformanceFrequency(&m_freq);
			QueryPerformanceCounter(&m_last);
			recreateResource();
			setTargetFPS(60);
		}
		~SteadyFrameRateController() = default;
	};

	class FrameQuery : public Graphics::IDeviceEventListener
	{
	private:

		SmartReference<Graphics::IDevice> m_device;
		win32::com_ptr<ID3D11Device> d3d11_device;
		win32::com_ptr<ID3D11DeviceContext> d3d11_device_context;
		win32::com_ptr<ID3D11Query> d3d11_query_freq;
		win32::com_ptr<ID3D11Query> d3d11_query_time_begin;
		win32::com_ptr<ID3D11Query> d3d11_query_time_end;
		win32::com_ptr<ID3D11Query> d3d11_query_statistics;
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT freq{};
		UINT64 time_begin{};
		UINT64 time_end{};
		D3D11_QUERY_DATA_PIPELINE_STATISTICS statistics{};
		bool is_flying{};
		bool is_scope{};

		void onDeviceCreate() override {
			createResources(static_cast<ID3D11Device*>(m_device->getNativeHandle()));
		}
		void onDeviceDestroy() override {
			destroyResources();
		}

	public:

		bool createResources(ID3D11Device* device)
		{
			assert(device);
			d3d11_device = device;
			d3d11_device->GetImmediateContext(d3d11_device_context.put());
			D3D11_QUERY_DESC d3d11_query_info{};
			d3d11_query_info.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
			if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_freq.put())) {
				return false;
			}
			d3d11_query_info.Query = D3D11_QUERY_TIMESTAMP;
			if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_time_begin.put())) {
				return false;
			}
			if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_time_end.put())) {
				return false;
			}
			d3d11_query_info.Query = D3D11_QUERY_PIPELINE_STATISTICS;
			if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_statistics.put())) {
				return false;
			}
			is_flying = false;
			is_scope = false;
			return true;
		}
		void destroyResources()
		{
			d3d11_query_freq = nullptr;
			d3d11_query_time_begin = nullptr;
			d3d11_query_time_end = nullptr;
			d3d11_query_statistics = nullptr;
			is_flying = false;
			is_scope = false;
		}
		void fetchData()
		{
			assert(d3d11_device_context);
			assert(d3d11_query_freq);
			assert(d3d11_query_time_begin);
			assert(d3d11_query_time_end);
			assert(d3d11_query_statistics);
			if (!is_flying) {
				return;
			}
			if (d3d11_query_freq) {
				while (S_OK != d3d11_device_context->GetData(d3d11_query_freq.get(), &freq, sizeof(freq), 0)) {
					;
				}
			}
			if (d3d11_query_time_begin) {
				while (S_OK != d3d11_device_context->GetData(d3d11_query_time_begin.get(), &time_begin, sizeof(time_begin), 0)) {
					;
				}
			}
			if (d3d11_query_time_end) {
				while (S_OK != d3d11_device_context->GetData(d3d11_query_time_end.get(), &time_end, sizeof(time_end), 0)) {
					;
				}
			}
			if (d3d11_query_statistics) {
				while (S_OK != d3d11_device_context->GetData(d3d11_query_statistics.get(), &statistics, sizeof(statistics), 0)) {
					;
				}
			}
			is_flying = false;
		}
		double getTime()
		{
			if (freq.Frequency == 0) {
				return 0.0;
			}
			return double(time_end - time_begin) / double(freq.Frequency);
		}
		void begin()
		{
			if (is_flying) {
				fetchData();
			}
			assert(!is_flying);
			assert(!is_scope);
			is_scope = true;
			assert(d3d11_device_context);
			assert(d3d11_query_freq);
			assert(d3d11_query_time_begin);
			assert(d3d11_query_statistics);
			if (d3d11_query_freq) {
				d3d11_device_context->Begin(d3d11_query_freq.get());
			}
			if (d3d11_query_time_begin) {
				d3d11_device_context->End(d3d11_query_time_begin.get()); // timestamp End only
			}
			if (d3d11_query_statistics) {
				d3d11_device_context->Begin(d3d11_query_statistics.get());
			}
		}
		void end()
		{
			assert(d3d11_device_context);
			assert(d3d11_query_freq);
			assert(d3d11_query_time_end);
			assert(d3d11_query_statistics);
			assert(is_scope);
			if (d3d11_query_freq) {
				d3d11_device_context->End(d3d11_query_freq.get());
			}
			if (d3d11_query_time_end) {
				d3d11_device_context->End(d3d11_query_time_end.get()); // timestamp End only
			}
			if (d3d11_query_statistics) {
				d3d11_device_context->End(d3d11_query_statistics.get());
			}
			is_scope = false;
			is_flying = true;
		}

		FrameQuery(Graphics::IDevice* p_device) : m_device(p_device) {
			createResources(static_cast<ID3D11Device*>(m_device->getNativeHandle()));
			m_device->addEventListener(this);
		}
		~FrameQuery() {
			m_device->removeEventListener(this);
		}
	};

	class ApplicationModel_Win32 : public implement::ReferenceCounted<IApplicationModel>
	{
	private:
		// 多个线程共享

		SmartReference<Graphics::Window_Win32> m_window;
		Microsoft::WRL::Wrappers::Event win32_event_exit;
		bool m_exit_flag{};

		// 仅限工作线程

		SmartReference<Graphics::Direct3D11::Device> m_device;
		SmartReference<Graphics::SwapChain_D3D11> m_swapchain;
		SmartReference<Graphics::Renderer_D3D11> m_renderer;
		IFrameRateController* m_p_frame_rate_controller{};
		FrameRateController m_frame_rate_controller;
		SteadyFrameRateController m_steady_frame_rate_controller;
		IApplicationEventListener* m_listener{ nullptr };
		size_t m_framestate_index{ 0 };
		FrameStatistics m_framestate[2]{};
		std::vector<FrameQuery> m_frame_query_list;
		size_t m_frame_query_index{};

		static DWORD WINAPI win32_thread_worker_entry(LPVOID lpThreadParameter);
		void worker();

		bool runSingleThread();
		bool runDoubleThread();

	public:
		// 内部公开

		void runFrame();

	public:
		// 多个线程共享

		Graphics::IWindow* getWindow() { return *m_window; }
		void requestExit();

		// 仅限工作线程

		IFrameRateController* getFrameRateController() { return m_p_frame_rate_controller; };
		Graphics::IDevice* getDevice() { return *m_device; }
		Graphics::ISwapChain* getSwapChain() { return *m_swapchain; }
		Graphics::IRenderer* getRenderer() { return *m_renderer; }
		FrameStatistics getFrameStatistics();
		FrameRenderStatistics getFrameRenderStatistics();

		// 仅限主线程

		bool run();

	public:
		ApplicationModel_Win32(IApplicationEventListener* p_listener);
		~ApplicationModel_Win32();
	};
}
