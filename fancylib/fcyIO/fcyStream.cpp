#include "fcyIO/fcyStream.h"
#include <Windows.h>

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
