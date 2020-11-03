#include "fcyMisc/fcyStopWatch.h"

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

fcyStopWatch::fcyStopWatch(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_cFreq); // 初始化
	Reset();
}

fcyStopWatch::~fcyStopWatch(void)
{
}

void fcyStopWatch::Pause()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_cFixStart);
}

void fcyStopWatch::Resume()
{
	fuLong tNow;
	QueryPerformanceCounter((LARGE_INTEGER*)&tNow);
	m_cFixAll += tNow - m_cFixStart;
}

void fcyStopWatch::Reset()
{
	m_cFixAll = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&m_cLast);
}

fDouble fcyStopWatch::GetElapsed()
{
	fuLong tNow;
	QueryPerformanceCounter((LARGE_INTEGER*)&tNow);
	return ((double)(tNow - m_cLast - m_cFixAll))/((double)m_cFreq);
}
