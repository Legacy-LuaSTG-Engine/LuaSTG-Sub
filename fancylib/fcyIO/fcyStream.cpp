#include "fcyIO/fcyStream.h"
#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

fcyFileStream::fcyFileStream(fcStrW Path, bool Writable)
    : m_sPath(Path), m_bWritable(Writable) {
    m_hFile = (fHandle) ::CreateFileW(
        m_sPath.c_str(),
        Writable ? GENERIC_WRITE | GENERIC_READ : GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        Writable ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (m_hFile == (fHandle) INVALID_HANDLE_VALUE)
        throw fcyWin32Exception("fcyFileStream::fcyFileStream", "CreateFile Failed");
}

fcyFileStream::~fcyFileStream() {
    ::CloseHandle((HANDLE) m_hFile);
}

bool fcyFileStream::CanWrite() {
    return m_bWritable;
}

bool fcyFileStream::CanResize() {
    return m_bWritable;
}

uint64_t fcyFileStream::GetLength() {
    LARGE_INTEGER tValue = {};
    if (FALSE == ::GetFileSizeEx((HANDLE) m_hFile, &tValue))
        return 0;
    return (uint64_t) tValue.QuadPart;
}

fResult fcyFileStream::SetLength(uint64_t Length) {
    LARGE_INTEGER tValue = {};
    tValue.QuadPart = Length;
    if (INVALID_SET_FILE_POINTER == ::SetFilePointer((HANDLE) m_hFile, tValue.LowPart, &tValue.HighPart, FILE_BEGIN))
        return FCYERR_INTERNALERR;
    if (FALSE == ::SetEndOfFile((HANDLE) m_hFile))
        return FCYERR_INTERNALERR;
    return FCYERR_OK;
}

uint64_t fcyFileStream::GetPosition() {
    LARGE_INTEGER tValue = {};
    tValue.LowPart = ::SetFilePointer((HANDLE) m_hFile, tValue.LowPart, &tValue.HighPart, FILE_CURRENT);
    return tValue.QuadPart;
}

fResult fcyFileStream::SetPosition(FCYSEEKORIGIN Origin, int64_t Offset) {
    uint32_t tOrigin;
    switch (Origin) {
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
    LARGE_INTEGER tValue = {};
    tValue.QuadPart = Offset;
    if (INVALID_SET_FILE_POINTER == ::SetFilePointer((HANDLE) m_hFile, tValue.LowPart, &tValue.HighPart, tOrigin))
        return FCYERR_INTERNALERR;
    else
        return FCYERR_OK;
}

fResult fcyFileStream::ReadBytes(fData pData, uint64_t Length, uint64_t* pBytesRead) {
    DWORD tRealReaded = 0;
    
    if (pBytesRead)
        *pBytesRead = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pData)
        return FCYERR_INVAILDPARAM;
    if (FALSE == ::ReadFile((HANDLE) m_hFile, pData, (DWORD) Length, (LPDWORD) &tRealReaded, NULL))
        return FCYERR_INTERNALERR;
    if (pBytesRead)
        *pBytesRead = tRealReaded;
    if (Length != tRealReaded)
        return FCYERR_OUTOFRANGE;
    else
        return FCYERR_OK;
}

fResult fcyFileStream::WriteBytes(fcData pSrc, uint64_t Length, uint64_t* pBytesWrite) {
    if (!m_bWritable)
        return FCYERR_ILLEGAL;
    
    DWORD tRealWritten = 0;
    
    if (pBytesWrite)
        *pBytesWrite = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pSrc)
        return FCYERR_INVAILDPARAM;
    if (FALSE == ::WriteFile((HANDLE) m_hFile, pSrc, (DWORD) Length, (LPDWORD) &tRealWritten, NULL))
        return FCYERR_INTERNALERR;
    if (tRealWritten != Length)
        return FCYERR_OUTOFRANGE;
    else
        return FCYERR_OK;
}

void fcyFileStream::Lock() {
    m_CriticalSec.Lock();
}

fResult fcyFileStream::TryLock() {
    return m_CriticalSec.TryLock() ? FCYERR_OK : FCYERR_ILLEGAL;
}

void fcyFileStream::Unlock() {
    m_CriticalSec.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

fcyMemStream::fcyMemStream(fcData Src, uint64_t Length, bool Writable, bool Resizable)
    : m_bResizable(Resizable)
    , m_bWritable(Writable)
    , m_cPointer(0)
{
    m_Data.resize((size_t) Length);
    if (Src)
        memcpy(&m_Data[0], Src, (size_t) Length);
}

fcyMemStream::fcyMemStream(std::vector<uint8_t>&& rvData)
    : m_Data(rvData)
    , m_bResizable(false)
    , m_bWritable(false)
    , m_cPointer(0)
{}

fcyMemStream::~fcyMemStream() {
}

fData fcyMemStream::GetInternalBuffer() {
    return (fData) m_Data.data();
}

bool fcyMemStream::CanWrite() {
    return m_bWritable;
}

bool fcyMemStream::CanResize() {
    return m_bResizable;
}

uint64_t fcyMemStream::GetLength() {
    return m_Data.size();
}

fResult fcyMemStream::SetLength(uint64_t Length) {
    if (m_bResizable) {
        m_Data.resize((size_t) Length);
        if (m_cPointer > m_Data.size())
            m_cPointer = m_Data.size();
        return FCYERR_OK;
    }
    else
        return FCYERR_ILLEGAL;
}

uint64_t fcyMemStream::GetPosition() {
    return m_cPointer;
}

fResult fcyMemStream::SetPosition(FCYSEEKORIGIN Origin, int64_t Offset) {
    switch (Origin) {
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
    if (Offset < 0 && ((uint32_t) (-Offset)) > m_cPointer) {
        m_cPointer = 0;
        return FCYERR_OUTOFRANGE;
    }
    else if (Offset > 0 && Offset + m_cPointer >= m_Data.size()) {
        m_cPointer = m_Data.size();
        return FCYERR_OUTOFRANGE;
    }
    m_cPointer += Offset;
    return FCYERR_OK;
}

fResult fcyMemStream::ReadBytes(fData pData, uint64_t Length, uint64_t* pBytesRead) {
    if (pBytesRead)
        *pBytesRead = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pData)
        return FCYERR_INVAILDPARAM;
    
    uint64_t tRestSize = m_Data.size() - m_cPointer;
    
    if (tRestSize == 0)
        return FCYERR_OUTOFRANGE;
    
    if (tRestSize < Length) {
        memcpy(pData, &m_Data[(size_t) m_cPointer], (size_t) tRestSize);
        m_cPointer += tRestSize;
        if (pBytesRead)
            *pBytesRead = tRestSize;
        return FCYERR_OUTOFRANGE;
    }
    else {
        memcpy(pData, &m_Data[(size_t) m_cPointer], (size_t) Length);
        m_cPointer += Length;
        if (pBytesRead)
            *pBytesRead = Length;
        return FCYERR_OK;
    }
}

fResult fcyMemStream::WriteBytes(fcData pSrc, uint64_t Length, uint64_t* pBytesWrite) {
    if (!m_bWritable)
        return FCYERR_ILLEGAL;
    
    if (pBytesWrite)
        *pBytesWrite = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pSrc)
        return FCYERR_INVAILDPARAM;
    
    uint64_t tRestSize = m_Data.size() - m_cPointer;
    
    if (tRestSize < Length) {
        if (!m_bResizable) {
            if (tRestSize == 0)
                return FCYERR_OUTOFRANGE;
            else {
                memcpy(&m_Data[(size_t) m_cPointer], pSrc, (size_t) tRestSize);
                m_cPointer += tRestSize;
                if (pBytesWrite)
                    *pBytesWrite = tRestSize;
                return FCYERR_OUTOFRANGE;
            }
        }
        else {
            m_Data.resize((size_t) (m_Data.size() + (Length - tRestSize)));
        }
    }
    memcpy(&m_Data[(size_t) m_cPointer], pSrc, (size_t) Length);
    m_cPointer += Length;
    if (pBytesWrite)
        *pBytesWrite = Length;
    return FCYERR_OK;
}

void fcyMemStream::Lock() {
    m_CriticalSec.Lock();
}

fResult fcyMemStream::TryLock() {
    return m_CriticalSec.TryLock() ? FCYERR_OK : FCYERR_ILLEGAL;
}

void fcyMemStream::Unlock() {
    m_CriticalSec.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

fcyPartialStream::fcyPartialStream(fcyStream* OrgStream, uint64_t Offset, uint64_t Size)
    : m_Offset(Offset), m_Len(Size), m_pPos(0), m_pOrgStream(OrgStream) {
    if (OrgStream == NULL)
        throw fcyWin32Exception("fcyPartialStream::fcyPartialStream", "Invalid Pointer.");
    if (Offset + Size > OrgStream->GetLength())
        throw fcyWin32Exception("fcyPartialStream::fcyPartialStream", "Partial stream is bigger than orginal stream.");
    m_pOrgStream->AddRef();
}

fcyPartialStream::~fcyPartialStream() {
    FCYSAFEKILL(m_pOrgStream);
}

bool fcyPartialStream::CanWrite() {
    return m_pOrgStream->CanWrite();
}

bool fcyPartialStream::CanResize() {
    return m_pOrgStream->CanResize();
}

uint64_t fcyPartialStream::GetLength() {
    return m_Len;
}

fResult fcyPartialStream::SetLength(uint64_t) {
    return FCYERR_ILLEGAL;
}

uint64_t fcyPartialStream::GetPosition() {
    return m_pPos;
}

fResult fcyPartialStream::SetPosition(FCYSEEKORIGIN Origin, int64_t Offset) {
    switch (Origin) {
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
    if (Offset < 0 && ((uint32_t) (-Offset)) > m_pPos) {
        m_pPos = 0;
        return FCYERR_OUTOFRANGE;
    }
    else if (Offset > 0 && Offset + m_pPos >= m_Len) {
        m_pPos = m_Len;
        return FCYERR_OUTOFRANGE;
    }
    m_pPos += Offset;
    return FCYERR_OK;
}

fResult fcyPartialStream::ReadBytes(fData pData, uint64_t Length, uint64_t* pBytesRead) {
    if (pBytesRead)
        *pBytesRead = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pData)
        return FCYERR_INVAILDPARAM;
    
    uint64_t tRestSize = m_Len - m_pPos;
    
    if (tRestSize == 0)
        return FCYERR_OUTOFRANGE;
    
    if (tRestSize < Length) {
        uint64_t tTempPos = m_pOrgStream->GetPosition();
        m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
        m_pOrgStream->ReadBytes(pData, tRestSize, pBytesRead);
        m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);
        
        m_pPos += tRestSize;
        return FCYERR_OUTOFRANGE;
    }
    else {
        uint64_t tTempPos = m_pOrgStream->GetPosition();
        m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
        m_pOrgStream->ReadBytes(pData, Length, pBytesRead);
        m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);
        
        m_pPos += Length;
        if (pBytesRead)
            *pBytesRead = Length;
        return FCYERR_OK;
    }
}

fResult fcyPartialStream::WriteBytes(fcData pSrc, uint64_t Length, uint64_t* pBytesWrite) {
    if (!CanWrite())
        return FCYERR_ILLEGAL;
    
    if (pBytesWrite)
        *pBytesWrite = 0;
    if (Length == 0)
        return FCYERR_OK;
    if (!pSrc)
        return FCYERR_INVAILDPARAM;
    
    uint64_t tRestSize = m_Len - m_pPos;
    
    if (tRestSize < Length) {
        if (tRestSize == 0)
            return FCYERR_OUTOFRANGE;
        else {
            uint64_t tTempPos = m_pOrgStream->GetPosition();
            m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
            m_pOrgStream->WriteBytes(pSrc, tRestSize, pBytesWrite);
            m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);
            
            m_pPos += tRestSize;
            return FCYERR_OUTOFRANGE;
        }
    }
    
    uint64_t tTempPos = m_pOrgStream->GetPosition();
    m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, m_Offset + m_pPos);
    m_pOrgStream->WriteBytes(pSrc, Length, pBytesWrite);
    m_pOrgStream->SetPosition(FCYSEEKORIGIN_BEG, tTempPos);
    
    m_pPos += Length;
    if (pBytesWrite)
        *pBytesWrite = Length;
    return FCYERR_OK;
}

void fcyPartialStream::Lock() {
    m_pOrgStream->Lock();
}

fResult fcyPartialStream::TryLock() {
    return m_pOrgStream->TryLock();
}

void fcyPartialStream::Unlock() {
    m_pOrgStream->Unlock();
}

////////////////////////////////////////////////////////////////////////////////

fResult fcyStreamHelper::FillStream(fcyStream* Src, fcyStream* Dest, uint64_t DataLength) {
    uint8_t tBuffer[8192];
    uint64_t tDataReaded = 0;
    uint64_t tTotalReaded = 0;
    
    while (tTotalReaded != DataLength && FCYOK(
        Src->ReadBytes(tBuffer, (DataLength - tTotalReaded > 8192) ? 8192 : DataLength - tTotalReaded, &tDataReaded))) {
        tTotalReaded += tDataReaded;
        
        Dest->WriteBytes(tBuffer, tDataReaded, NULL);
    }
    
    if (tTotalReaded != DataLength)
        return FCYERR_INTERNALERR;
    else
        return FCYERR_OK;
}
