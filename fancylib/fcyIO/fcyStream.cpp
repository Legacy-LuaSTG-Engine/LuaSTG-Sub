#include "fcyIO/fcyStream.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyFileStream::fcyFileStream(fcStrW Path, fBool Writable)
	: m_sPath(Path), m_bWritable(Writable)
{
	m_hFile = CreateFileW(
		m_sPath.c_str(), 
		Writable? GENERIC_WRITE|GENERIC_READ : GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		Writable? OPEN_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if(m_hFile == INVALID_HANDLE_VALUE)
		throw fcyWin32Exception("fcyFileStream::fcyFileStream", "CreateFile Failed");
}

fcyFileStream::~fcyFileStream()
{
	CloseHandle(m_hFile);
}

fBool fcyFileStream::CanWrite()
{
	return m_bWritable;
}

fBool fcyFileStream::CanResize()
{
	return m_bWritable;
}

fLen fcyFileStream::GetLength()
{
	return GetFileSize(m_hFile, NULL);
}

fResult fcyFileStream::SetLength(fLen Length)
{
	LARGE_INTEGER tValue;
	tValue.QuadPart = Length;

	if(INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, tValue.LowPart, &tValue.HighPart, FILE_BEGIN))
		return FCYERR_INTERNALERR;
	if(FALSE == SetEndOfFile(m_hFile))
		return FCYERR_INTERNALERR;
	return FCYERR_OK;
}

fLen fcyFileStream::GetPosition()
{
	LARGE_INTEGER tValue;
	tValue.QuadPart = 0;

	tValue.LowPart = SetFilePointer(m_hFile, tValue.LowPart, &tValue.HighPart, FILE_CURRENT);
	return tValue.QuadPart;
}

fResult fcyFileStream::SetPosition(FCYSEEKORIGIN Origin, fLong Offset)
{
	fuInt tOrigin;
	switch(Origin)
	{
	case FCYSEEKORIGIN_CUR:
		tOrigin = FILE_CURRENT;
		break;
	case FCYSEEKORIGIN_BEG:
		tOrigin = FILE_BEGIN;
		break;
	case FCYSEEKORIGIN_END:
		tOrigin = FILE_END;
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}

	LARGE_INTEGER tValue;
	tValue.QuadPart = Offset;
	if(INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, tValue.LowPart, &tValue.HighPart, tOrigin))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult fcyFileStream::ReadBytes(fData pData, fLen Length, fLen* pBytesRead)
{
	DWORD tRealReaded = 0;

    if(pBytesRead)
        *pBytesRead = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pData)
		return FCYERR_INVAILDPARAM;
	if(FALSE == ReadFile(m_hFile, pData, (DWORD)Length, (LPDWORD)&tRealReaded, NULL))
		return FCYERR_INTERNALERR;
    if(pBytesRead)
        *pBytesRead = tRealReaded;
	if(Length != tRealReaded)
		return FCYERR_OUTOFRANGE;
	else
		return FCYERR_OK;
}

fResult fcyFileStream::WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite)
{
	if(!m_bWritable)
		return FCYERR_ILLEGAL;
	
	DWORD tRealWritten = 0;

	if(pBytesWrite)
		*pBytesWrite = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pSrc)
		return FCYERR_INVAILDPARAM;
	if(FALSE == WriteFile(m_hFile, pSrc, (DWORD)Length, (LPDWORD)&tRealWritten, NULL))
		return FCYERR_INTERNALERR;
	if(tRealWritten != Length)
		return FCYERR_OUTOFRANGE;
	else
		return FCYERR_OK;
}

void fcyFileStream::Lock()
{
	m_CriticalSec.Lock();
}

fResult fcyFileStream::TryLock()
{
	return m_CriticalSec.TryLock() ? FCYERR_OK : FCYERR_ILLEGAL;
}

void fcyFileStream::Unlock()
{
	m_CriticalSec.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

fcyMemStream::fcyMemStream(fcData Src, fLen Length, fBool Writable, fBool Resizable)
	: m_bResizable(Resizable), m_bWritable(Writable), m_cPointer(0)
{
	m_Data.resize((vector<fByte>::size_type)Length);
	if(Src)
		memcpy(&m_Data[0], Src, (size_t)Length);
}

fcyMemStream::~fcyMemStream()
{
}

fData fcyMemStream::GetInternalBuffer()
{
	return (fData)m_Data.data();
}

fBool fcyMemStream::CanWrite()
{
	return m_bWritable;
}

fBool fcyMemStream::CanResize()
{
	return m_bResizable;
}

fLen fcyMemStream::GetLength()
{
	return m_Data.size();
}

fResult fcyMemStream::SetLength(fLen Length)
{
	if(m_bResizable)
	{
		m_Data.resize((vector<fByte>::size_type)Length);
		if(m_cPointer > m_Data.size())
			m_cPointer = m_Data.size();
		return FCYERR_OK;
	}
	else
		return FCYERR_ILLEGAL;
}

fLen fcyMemStream::GetPosition()
{
	return m_cPointer;
}

fResult fcyMemStream::SetPosition(FCYSEEKORIGIN Origin, fLong Offset)
{
	switch(Origin)
	{
	case FCYSEEKORIGIN_CUR:
		break;
	case FCYSEEKORIGIN_BEG:
		m_cPointer = 0;
		break;
	case FCYSEEKORIGIN_END:
		m_cPointer = m_Data.size();
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if(Offset<0 && ((fuInt)(-Offset))>m_cPointer)
	{
		m_cPointer = 0;
		return FCYERR_OUTOFRANGE;
	}
	else if(Offset>0 && Offset+m_cPointer>=m_Data.size())
	{
		m_cPointer = m_Data.size();
		return FCYERR_OUTOFRANGE;
	}
	m_cPointer+=Offset;
	return FCYERR_OK;
}

fResult fcyMemStream::ReadBytes(fData pData, fLen Length, fLen* pBytesRead)
{
	if(pBytesRead)
		*pBytesRead = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pData)
		return FCYERR_INVAILDPARAM;

	fLen tRestSize = m_Data.size() - m_cPointer;

	if(tRestSize == 0)
		return FCYERR_OUTOFRANGE;

	if(tRestSize<Length)
	{
		memcpy(pData, &m_Data[(vector<fByte>::size_type)m_cPointer], (size_t)tRestSize);
		m_cPointer += tRestSize;
		if(pBytesRead)
			*pBytesRead = tRestSize;
		return FCYERR_OUTOFRANGE;
	}
	else
	{
		memcpy(pData, &m_Data[(vector<fByte>::size_type)m_cPointer], (size_t)Length);
		m_cPointer += Length;
		if(pBytesRead)
			*pBytesRead = Length;
		return FCYERR_OK;
	}
}

fResult fcyMemStream::WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite)
{
	if(!m_bWritable)
		return FCYERR_ILLEGAL;
	
	if(pBytesWrite)
		*pBytesWrite = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pSrc)
		return FCYERR_INVAILDPARAM;

	fLen tRestSize = m_Data.size() - m_cPointer;

	if(tRestSize < Length)
	{
		if(!m_bResizable)
		{
			if(tRestSize == 0)
				return FCYERR_OUTOFRANGE;
			else
			{
				memcpy(&m_Data[(vector<fByte>::size_type)m_cPointer], pSrc, (size_t)tRestSize);
				m_cPointer += tRestSize;
				if(pBytesWrite)
					*pBytesWrite = tRestSize;
				return FCYERR_OUTOFRANGE;
			}
		}
		else
		{
			m_Data.resize((vector<fByte>::size_type)(m_Data.size() + (Length - tRestSize)));
		}
	}
	memcpy(&m_Data[(vector<fByte>::size_type)m_cPointer], pSrc, (size_t)Length);
	m_cPointer += Length;
	if(pBytesWrite)
		*pBytesWrite = Length;
	return FCYERR_OK;
}

void fcyMemStream::Lock()
{
	m_CriticalSec.Lock();
}

fResult fcyMemStream::TryLock()
{
	return m_CriticalSec.TryLock() ? FCYERR_OK : FCYERR_ILLEGAL;
}

void fcyMemStream::Unlock()
{
	m_CriticalSec.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

fcyPartialStream::fcyPartialStream(fcyStream* OrgStream, fLen Offset, fLen Size)
	: m_Offset(Offset), m_Len(Size), m_pPos(0), m_pOrgStream(OrgStream)
{
	if(OrgStream == NULL)
		throw fcyWin32Exception("fcyPartialStream::fcyPartialStream", "Invalid Pointer.");
	if(Offset + Size > OrgStream->GetLength())
		throw fcyWin32Exception("fcyPartialStream::fcyPartialStream", "Partial stream is bigger than orginal stream.");
	m_pOrgStream->AddRef();
}

fcyPartialStream::~fcyPartialStream()
{
	FCYSAFEKILL(m_pOrgStream);
}

fBool fcyPartialStream::CanWrite()
{
	return m_pOrgStream->CanWrite();
}

fBool fcyPartialStream::CanResize()
{
	return m_pOrgStream->CanResize();
}

fLen fcyPartialStream::GetLength()
{
	return m_Len;
}

fResult fcyPartialStream::SetLength(fLen Length)
{
	return FCYERR_ILLEGAL;
}

fLen fcyPartialStream::GetPosition()
{
	return m_pPos;
}

fResult fcyPartialStream::SetPosition(FCYSEEKORIGIN Origin, fLong Offset)
{
	switch(Origin)
	{
	case FCYSEEKORIGIN_CUR:
		break;
	case FCYSEEKORIGIN_BEG:
		m_pPos = 0;
		break;
	case FCYSEEKORIGIN_END:
		m_pPos = m_Len;
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if(Offset<0 && ((fuInt)(-Offset))>m_pPos)
	{
		m_pPos = 0;
		return FCYERR_OUTOFRANGE;
	}
	else if(Offset>0 && Offset+m_pPos>=m_Len)
	{
		m_pPos = m_Len;
		return FCYERR_OUTOFRANGE;
	}
	m_pPos+=Offset;
	return FCYERR_OK;
}

fResult fcyPartialStream::ReadBytes(fData pData, fLen Length, fLen* pBytesRead)
{
	if(pBytesRead)
		*pBytesRead = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pData)
		return FCYERR_INVAILDPARAM;

	fLen tRestSize = m_Len - m_pPos;

	if(tRestSize == 0)
		return FCYERR_OUTOFRANGE;

	if(tRestSize<Length)
	{
		fLen tTempPos = m_pOrgStream->GetPosition();
		m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
		m_pOrgStream->ReadBytes(pData, tRestSize, pBytesRead);
		m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);

		m_pPos += tRestSize;
		return FCYERR_OUTOFRANGE;
	}
	else
	{
		fLen tTempPos = m_pOrgStream->GetPosition();
		m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
		m_pOrgStream->ReadBytes(pData, Length, pBytesRead);
		m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);

		m_pPos += Length;
		if(pBytesRead)
			*pBytesRead = Length;
		return FCYERR_OK;
	}
}

fResult fcyPartialStream::WriteBytes(fcData pSrc, fLen Length, fLen* pBytesWrite)
{
	if(!CanWrite())
		return FCYERR_ILLEGAL;
	
	if(pBytesWrite)
		*pBytesWrite = 0;
	if(Length == 0)
		return FCYERR_OK;
	if(!pSrc)
		return FCYERR_INVAILDPARAM;

	fLen tRestSize = m_Len - m_pPos;

	if(tRestSize < Length)
	{
		if(tRestSize == 0)
			return FCYERR_OUTOFRANGE;
		else
		{
			fLen tTempPos = m_pOrgStream->GetPosition();
			m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
			m_pOrgStream->WriteBytes(pSrc, tRestSize, pBytesWrite);
			m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);
			
			m_pPos += tRestSize;
			return FCYERR_OUTOFRANGE;
		}
	}

	fLen tTempPos = m_pOrgStream->GetPosition();
	m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
	m_pOrgStream->WriteBytes(pSrc, Length, pBytesWrite);
	m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);

	m_pPos += Length;
	if(pBytesWrite)
		*pBytesWrite = Length;
	return FCYERR_OK;
}

void fcyPartialStream::Lock()
{
	m_pOrgStream->Lock();
}

fResult fcyPartialStream::TryLock()
{
	return m_pOrgStream->TryLock();
}

void fcyPartialStream::Unlock()
{
	m_pOrgStream->Unlock();
}

////////////////////////////////////////////////////////////////////////////////

fResult fcyStreamHelper::FillStream(fcyStream* Src, fcyStream* Dest, fLen DataLength)
{
	fByte tBuffer[8192];
	fLen tDataReaded = 0;
	fLen tTotalReaded = 0;

	while(tTotalReaded != DataLength && FCYOK(Src->ReadBytes(tBuffer, (DataLength - tTotalReaded > 8192)?8192:DataLength - tTotalReaded, &tDataReaded)))
	{
		tTotalReaded += tDataReaded;

		Dest->WriteBytes(tBuffer, tDataReaded, NULL);
	}

	if(tTotalReaded != DataLength)
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}