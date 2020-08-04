#include "fcyDebug.h"

#include <cstdio>
#include <Windows.h>

//////////////////////////////////////////////////////////////////////////////////////////

void fcyDebug::Trace(fcStr Format, ...)
{
	char szText[1024];
	va_list marker;
	va_start(marker, Format);
	vsprintf_s(szText, Format, marker);
	va_end(marker);

	OutputDebugStringA(szText);
}

void fcyDebug::Trace(fcStrW Format, ...)
{
	wchar_t szText[1024];
	va_list marker;
	va_start(marker, Format);
	vswprintf_s(szText, Format, marker);
	va_end(marker);

	OutputDebugStringW(szText);
}
