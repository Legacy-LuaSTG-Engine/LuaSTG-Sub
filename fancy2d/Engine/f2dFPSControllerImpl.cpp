#include "Engine/f2dFPSControllerImpl.h"

fDouble f2dFPSControllerImpl::Update()
{
	// 先获取当前计数器
	LARGE_INTEGER curr_{};
	QueryPerformanceCounter(&curr_);

	// 当设备运行时间足够长后，即使是 int64 也会溢出
	if (curr_.QuadPart < last_)
	{
		total_frame_ += 1;
		total_time_ += 1.0 / get_fps_(0);
		last_ = curr_.QuadPart;
		return get_fps_(0);
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

	// 更新各项数值
	double const fps = (double)freq_ / (double)(curr_.QuadPart - last_);
	double const s = 1.0 / fps;
	total_frame_ += 1;
	total_time_ += s;
	fps_[fps_index_] = fps;
	fps_index_ = (fps_index_ + 1) % std::size(fps_);
	last_ = curr_.QuadPart;

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
			double const fps_history = get_fps_(i);
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

fuInt f2dFPSControllerImpl::GetLimitedFPS()
{
	return (fuInt)target_fps_;
}
void f2dFPSControllerImpl::SetLimitedFPS(fuInt MaxFPS)
{
	target_fps_ = (double)(MaxFPS > 0 ? MaxFPS : 1);
	LARGE_INTEGER lli{};
	QueryPerformanceFrequency(&lli);
	freq_ = lli.QuadPart;
	wait_ = (LONGLONG)((double)freq_ / target_fps_);
	_2ms_ = (2ll * freq_) / 1000ll;
}
fDouble f2dFPSControllerImpl::GetFPS()
{
	return get_fps_(0);
}
fuInt f2dFPSControllerImpl::GetTotalFrame()
{
	return (fuInt)total_frame_; // TODO: overflow
}
fDouble f2dFPSControllerImpl::GetTotalTime()
{
	return total_time_;
}
fDouble f2dFPSControllerImpl::GetAvgFPS()
{
	return fps_avg_;
}
fDouble f2dFPSControllerImpl::GetMinFPS()
{
	return fps_min_;
}
fDouble f2dFPSControllerImpl::GetMaxFPS()
{
	return fps_max_;
}

f2dFPSControllerImpl::f2dFPSControllerImpl(fuInt MaxFPS)
{
	timeBeginPeriod(1);
	SetLimitedFPS(MaxFPS);
	LARGE_INTEGER lli{};
	QueryPerformanceCounter(&lli);
	last_ = lli.QuadPart;
}
f2dFPSControllerImpl::~f2dFPSControllerImpl()
{
	timeEndPeriod(1);
}
