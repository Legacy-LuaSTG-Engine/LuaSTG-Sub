#include "fcyException.h"

#include <Windows.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyException::fcyException()
	: m_Time(0)
{}

fcyException::fcyException(fcStr Src, fcStr Desc, ...)
{
	char szText[512];
	va_list marker;
	va_start(marker, Desc);
	vsprintf_s(szText, Desc, marker);
	va_end(marker);

	m_Time = GetTickCount();
	m_ExcpSrc = Src;
	m_ExcpDesc = szText;
}

fcyException::fcyException(const fcyException& Org)
{
	m_Time = Org.m_Time;
	m_ExcpSrc = Org.m_ExcpSrc;
	m_ExcpDesc = Org.m_ExcpDesc;
}

fcyException::~fcyException(void)
{}

uint32_t fcyException::GetTime()const
{
	return m_Time;
}

fcStr fcyException::GetSrc()const
{
	return m_ExcpSrc.c_str();
}

fcStr fcyException::GetDesc()const
{
	return m_ExcpDesc.c_str();
}

////////////////////////////////////////////////////////////////////////////////

fcyWin32Exception::fcyWin32Exception(fcStr Src, fcStr Info)
	: m_LastError(GetLastError()), fcyException(Src, Info)
{
	char tNumStr[16] = { 0 };
	_itoa_s((int)m_LastError, tNumStr, 16, 10);
	m_ExcpDesc = m_ExcpDesc + " ( LastErr = " + tNumStr + " )";
}

fcyWin32Exception::~fcyWin32Exception()
{}

uint32_t fcyWin32Exception::GetErrorCode()const
{
	return m_LastError;
}

////////////////////////////////////////////////////////////////////////////////

fcyWin32COMException::fcyWin32COMException(fcStr Src, fcStr Info, int32_t HRet)
	: m_HR(HRet), fcyException(Src, Info)
{
	char tNumStr[16] = { 0 };
	_itoa_s((int)m_HR, tNumStr, 16, 10);
	m_ExcpDesc = m_ExcpDesc + " ( HRESULT = " + tNumStr + " )";
}

fcyWin32COMException::~fcyWin32COMException()
{}

int32_t fcyWin32COMException::GetResult()const
{
	return m_HR;
}
