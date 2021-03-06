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
	fLong m_cFreq;      ///< @brief CPU频率
	fLong m_cLast;      ///< @brief 上一次时间
	fLong m_cFixStart;  ///< @brief 暂停时的时间修复参数
	fLong m_cFixAll;    ///< @brief 暂停时的时间修复参数
public:
	void Pause();        ///< @brief 暂停
	void Resume();       ///< @brief 继续
	void Reset();        ///< @brief 归零
	fDouble GetElapsed(); ///< @brief 获得流逝时间
	                      ///< @note  以秒为单位
public:
	fcyStopWatch();
	~fcyStopWatch();
};
/// @}
