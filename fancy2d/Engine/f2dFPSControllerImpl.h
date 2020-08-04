////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFPSControllerImpl.h
/// @brief fancy2D FPS控制器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyRefObj.h>
#include <fcyIO/fcyStream.h>
#include <fcyMisc/fcyStopWatch.h>

#include "f2dEngine.h"

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief FPS控制器实现
////////////////////////////////////////////////////////////////////////////////
class f2dFPSControllerImpl : 
	public f2dFPSController
{
protected:
	// 计数器
	fuInt m_TotalFrame;      ///< @brief 总帧数
	fDouble m_TotalTime;     ///< @brief 总运行时间
	fDouble m_MaxFPS;        ///< @brief 最大FPS
	fDouble m_CurFPS;        ///< @brief 当前FPS

	// 单帧计数器
	fDouble m_FrameTimeSecond; ///< @brief 计数1秒
	fuInt m_FrameCountSecond;  ///< @brief 1秒内帧数

	fuInt m_FPSMax;       ///< @brief 最大FPS
	fDouble m_FrameDelay; ///< @brief 帧时间间隔
public: // 公开
	/// @brief 更新FPS控制器
	/// @note  获得两个时间点之间的流逝时间并更新FPS计数器
	fDouble Update(fcyStopWatch& Watch)
	{
		fDouble tElapsedTime = Watch.GetElapsed();

		// FPS限制
		if(m_FPSMax && tElapsedTime < m_FrameDelay)
		{
			fuInt tTimeToSleep = (fuInt)((m_FrameDelay - tElapsedTime) * 1000.);

			// Sleep限速
			if(tTimeToSleep > 2)
				Sleep(tTimeToSleep - 2);

			// 自旋精确限速
			while((tElapsedTime = Watch.GetElapsed()) < m_FrameDelay) {}
		}

		tElapsedTime = Watch.GetElapsed();
		Watch.Reset();

		m_TotalFrame++;
		m_TotalTime += tElapsedTime;

		m_FrameTimeSecond += tElapsedTime;
		m_FrameCountSecond++;

		// FPS计数
		if(m_FrameTimeSecond >= 1.)
		{
			m_CurFPS = m_FrameCountSecond / m_FrameTimeSecond;
			m_FrameCountSecond = 0;
			m_FrameTimeSecond = 0.;

			if(m_CurFPS > m_MaxFPS)
				m_MaxFPS = m_CurFPS;
		}

		return tElapsedTime;
	}
public: // 接口实现
	fuInt GetLimitedFPS()
	{
		return m_FPSMax;
	}
	void SetLimitedFPS(fuInt iMaxFPS)
	{
		m_FPSMax = iMaxFPS;
		m_FrameDelay = m_FPSMax ? 1./m_FPSMax : 0;
	}
	fDouble GetFPS()
	{
		return m_CurFPS;
	}
	fuInt GetTotalFrame()
	{
		return m_TotalFrame;
	}
	fDouble GetTotalTime()
	{
		return m_TotalTime;
	}
	fDouble GetAvgFPS()
	{
		if(m_TotalTime == 0)
			return 0;
		return m_TotalFrame / m_TotalTime;
	}
	fDouble GetMaxFPS()
	{
		return m_MaxFPS;
	}
public:
	f2dFPSControllerImpl(fuInt MaxFPS)
		: m_TotalFrame(0), m_TotalTime(0), m_MaxFPS(0), m_CurFPS(0),
		m_FrameTimeSecond(0), m_FrameCountSecond(0),
		m_FPSMax(MaxFPS), m_FrameDelay(MaxFPS?1./MaxFPS : 0) 
	{
		timeBeginPeriod(1);
	}
	~f2dFPSControllerImpl()
	{
		timeEndPeriod(1);
	}
};
