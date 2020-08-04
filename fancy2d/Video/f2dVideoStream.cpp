#ifndef _M_ARM

#include "Video/f2dVideoStream.h"

////////////////////////////////////////////////////////////////////////////////

f2dVideoStream::f2dVideoStream(fcyStream* pStream)
	: m_pStream(pStream)
{
	if(m_pStream == NULL)
		throw fcyException("f2dVideoStream::f2dVideoStream", "Invaild Pointer.");
	
	m_pStream->AddRef();

	m_pStream->Lock();
	m_Pointer = m_pStream->GetPosition();
	m_pStream->Unlock();
}

f2dVideoStream::~f2dVideoStream()
{
	FCYSAFEKILL(m_pStream);
}

fcyStream* f2dVideoStream::GetStream()
{
	return m_pStream;
}

HRESULT f2dVideoStream::SetPointer(LONGLONG llPos)
{
	m_pStream->Lock();
	fResult tFR = m_pStream->SetPosition(FCYSEEKORIGIN_BEG, llPos);
	m_pStream->Unlock();

	if(FCYFAILED(tFR))
		return E_FAIL;
	else
	{
		m_Pointer = llPos;
		return S_OK;
	}	
}

HRESULT f2dVideoStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
	if(!pbBuffer)
		return E_POINTER;
	if(dwBytesToRead == 0)
		return S_OK;
	if(pdwBytesRead)
		*pdwBytesRead = 0;

	m_pStream->Lock();
	fResult tFR = m_pStream->SetPosition(FCYSEEKORIGIN_BEG, m_Pointer);
	if(FCYFAILED(tFR))
	{
		m_pStream->Unlock();
		return E_FAIL;
	}

	fLen tReaded = 0;
	tFR = m_pStream->ReadBytes(pbBuffer, dwBytesToRead, &tReaded);

	m_pStream->Unlock();

	if(FCYFAILED(tFR))
		return E_FAIL;
	else
	{
		m_Pointer += tReaded;
		if(pdwBytesRead)
			*pdwBytesRead = (DWORD)tReaded;
		return S_OK;
	}
}

LONGLONG f2dVideoStream::Size(LONGLONG* pSizeAvailable)
{
	fLen tLength = 0;

	m_pStream->Lock();
	tLength = m_pStream->GetLength();
	m_pStream->Unlock();

	if(pSizeAvailable)
		*pSizeAvailable = tLength;
	return tLength;
}

DWORD f2dVideoStream::Alignment()
{
	return 1;
}

void f2dVideoStream::Lock()
{
	m_csLock.Lock();
}

void f2dVideoStream::Unlock()
{
	m_csLock.Unlock();
}

////////////////////////////////////////////////////////////////////////////////

f2dVideoStreamReader::f2dVideoStreamReader(f2dVideoStream *pStream, HRESULT *phr)
	: CAsyncReader(L"f2dStream Reader", NULL, pStream, phr)
{
	// 由于AVI格式只支持MEDIASUBTYPE_Avi输入，需要手动进行检查
	m_mt.majortype = MEDIATYPE_Stream;
	m_mt.subtype = checkIsAVIFormat(pStream->GetStream())? MEDIASUBTYPE_Avi : MEDIASUBTYPE_NULL;
}

fBool f2dVideoStreamReader::checkIsAVIFormat(fcyStream* pStream)
{
	// 读取用于检查的11位头
	pStream->Lock();
	fLen tOrgPos = pStream->GetPosition();
	fByte tCheckData[11];
	fLen tReaded = 0;
	fResult tHR = pStream->ReadBytes(tCheckData, 11, &tReaded);
	pStream->SetPosition(FCYSEEKORIGIN_BEG, tOrgPos);
	pStream->Unlock();

	if(FCYFAILED(tHR) || tReaded != 11)
		return false;
	if((*(int*)tCheckData) == 0x46464952 &&            // RIFF
		strncmp((char*)&tCheckData[8], "AVI", 3) == 0) // AVI
		return true;
	else
		return false;
}

STDMETHODIMP f2dVideoStreamReader::Register()
{
	return S_OK;
}

STDMETHODIMP f2dVideoStreamReader::Unregister()
{
	return S_OK;
}

#endif
