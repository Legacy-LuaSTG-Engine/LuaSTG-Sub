#include "fcyMisc/fcyStopWatch.h"
#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

fcyStopWatch::fcyStopWatch(void)
{
	LARGE_INTEGER freq = {};
	QueryPerformanceFrequency(&freq); // 初始化
	m_cFreq = freq.QuadPart;
	Reset();
}

fcyStopWatch::~fcyStopWatch(void)
{
}

void fcyStopWatch::Pause()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cFixStart = t.QuadPart;
}

void fcyStopWatch::Resume()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cFixAll += t.QuadPart - m_cFixStart;
}

void fcyStopWatch::Reset()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	m_cLast = t.QuadPart;
	m_cFixAll = 0;
}

double fcyStopWatch::GetElapsed()
{
	LARGE_INTEGER t = {};
	QueryPerformanceCounter(&t);
	return ((double)(t.QuadPart - m_cLast - m_cFixAll)) / ((double)m_cFreq);
}
