////////////////////////////////////////////////////////////////////////////////
/// @file  fcyStopWatch.h
/// @brief fancy计时器
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"

/// @addtogroup fancy杂项
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief 高精度停表类
////////////////////////////////////////////////////////////////////////////////
class fcyStopWatch
{
private:
	int64_t m_cFreq;      ///< @brief CPU频率
	int64_t m_cLast;      ///< @brief 上一次时间
	int64_t m_cFixStart;  ///< @brief 暂停时的时间修复参数
	int64_t m_cFixAll;    ///< @brief 暂停时的时间修复参数
public:
	void Pause();        ///< @brief 暂停
	void Resume();       ///< @brief 继续
	void Reset();        ///< @brief 归零
	double GetElapsed(); ///< @brief 获得流逝时间
	                      ///< @note  以秒为单位
public:
	fcyStopWatch();
	~fcyStopWatch();
};
/// @}
