////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFPSControllerImpl.h
/// @brief fancy2D FPS控制器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <f2dEngine.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief FPS控制器实现
////////////////////////////////////////////////////////////////////////////////
class f2dFPSControllerImpl : public f2dFPSController
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
	inline double get_fps_(size_t idx)
	{
		return fps_[(fps_index_ + std::size(fps_) - 1 - idx) % std::size(fps_)];
	}
public:
	fDouble Update();
public:
	fuInt GetLimitedFPS();
	void SetLimitedFPS(fuInt MaxFPS);
	fDouble GetFPS();
	fuInt GetTotalFrame();
	fDouble GetTotalTime();
	fDouble GetAvgFPS();
	fDouble GetMinFPS();
	fDouble GetMaxFPS();
public:
	f2dFPSControllerImpl(fuInt MaxFPS);
	~f2dFPSControllerImpl();
};
