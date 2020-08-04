#include "Sound/f2dVorbisDecoder.h"

#include "fcyException.h"

#include "fcyOS/fcyDebug.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

size_t f2dVorbisDecoder::streamReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	size_t tLen = (size_t)(((fcyStream*)datasource)->GetLength() - ((fcyStream*)datasource)->GetPosition());
	
	fuInt tCountToRead = (tLen / size);
	if(tCountToRead > nmemb)
		tCountToRead = nmemb;

	((fcyStream*)datasource)->ReadBytes((fData)ptr, tCountToRead * size, NULL);

	return tCountToRead;
}

int f2dVorbisDecoder::streamSeekFunc(void *datasource, ogg_int64_t offset, int whence)
{
	FCYSEEKORIGIN tOrigin = FCYSEEKORIGIN_BEG;
	switch(whence)
	{
	case 0:
		tOrigin = FCYSEEKORIGIN_BEG;
		break;
	case 1:
		tOrigin = FCYSEEKORIGIN_CUR;
		break;
	case 2:
		tOrigin = FCYSEEKORIGIN_END;
		break;
	}
	if(FCYFAILED(((fcyStream*)datasource)->SetPosition(tOrigin, offset)))
		return -1;
	else
		return 0;
}

int f2dVorbisDecoder::streamCloseFunc(void *datasource)
{
	return 0;
}

long f2dVorbisDecoder::streamTellFunc(void *datasource)
{
	return (long)((fcyStream*)datasource)->GetPosition();
}

f2dVorbisDecoder::f2dVorbisDecoder(fcyStream* pStream)
	: m_CurrentSec(0), m_CurrentPos(0), m_pStream(pStream)
{
	ov_callbacks tCallbacks = 
	{
		(size_t (*)(void *, size_t, size_t, void *))  streamReadFunc,
		(int (*)(void *, ogg_int64_t, int))           streamSeekFunc,
		(int (*)(void *))                             streamCloseFunc,
		(long (*)(void *))                            streamTellFunc
	};

	if(!m_pStream)
		throw fcyException("f2dVorbisDecoder::f2dVorbisDecoder", "Invalid Pointer.");
	m_pStream->AddRef();

	m_pStream->Lock();
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, m_CurrentPos);

	int tRet = ov_open_callbacks(m_pStream, &m_OggFile, NULL, 0, tCallbacks);
	
	m_CurrentPos = m_pStream->GetPosition();
 	m_pStream->Unlock();

	if(tRet != 0)
	{
		FCYSAFEKILL(m_pStream);
		throw fcyException("f2dVorbisDecoder::f2dVorbisDecoder", "Invalid Stream.");
	}
}

f2dVorbisDecoder::~f2dVorbisDecoder()
{
	m_pStream->Lock();
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, m_CurrentPos);
	
	ov_clear(&m_OggFile);
	
	m_pStream->Unlock();

	FCYSAFEKILL(m_pStream);
}

fuInt f2dVorbisDecoder::GetBufferSize()
{
	return (fuInt)ov_pcm_total(&m_OggFile, -1) * GetBlockAlign();
}

fuInt f2dVorbisDecoder::GetAvgBytesPerSec()
{
	return GetBlockAlign() * GetSamplesPerSec();
}

fuShort f2dVorbisDecoder::GetBlockAlign()
{
	return GetChannelCount() * 2;  // 16 bits
}

fuShort f2dVorbisDecoder::GetChannelCount()
{
	vorbis_info* tInfo = ov_info(&m_OggFile, -1);
	return (fuShort)tInfo->channels;
}

fuInt f2dVorbisDecoder::GetSamplesPerSec()
{
	vorbis_info* tInfo = ov_info(&m_OggFile, -1);
	return (fuInt)tInfo->rate;
}

fuShort f2dVorbisDecoder::GetFormatTag()
{
	return 1;
}

fuShort f2dVorbisDecoder::GetBitsPerSample()
{
	return 16; // 16bits
}

fLen f2dVorbisDecoder::GetPosition()
{
	return (fLen)(ov_time_tell(&m_OggFile) * GetSamplesPerSec()) * GetBlockAlign();
}

fResult f2dVorbisDecoder::SetPosition(F2DSEEKORIGIN Origin, fInt Offset)
{
	// 计算目标位置
	fuInt tPCMPointer = (fuInt)GetPosition();

	switch(Origin)
	{
	case FCYSEEKORIGIN_CUR:
		break;
	case FCYSEEKORIGIN_BEG:
		tPCMPointer = 0;
		break;
	case FCYSEEKORIGIN_END:
		tPCMPointer = GetBufferSize();
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if(Offset<0 && ((fuInt)(-Offset))>tPCMPointer)
	{
		tPCMPointer = 0;
		return FCYERR_OUTOFRANGE;
	}
	else if(Offset>0 && Offset+tPCMPointer>=GetBufferSize())
	{
		tPCMPointer = GetBufferSize();
		return FCYERR_OUTOFRANGE;
	}
	tPCMPointer+=Offset;

	// 进行Seek操作
	m_CurrentSec = 0;
	
	m_pStream->Lock();  // 锁定流
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, m_CurrentPos);

	int tRet = ov_time_seek(&m_OggFile, tPCMPointer / (double)(GetSamplesPerSec() * GetBlockAlign()));
	
	m_CurrentPos = m_pStream->GetPosition();
	m_pStream->Unlock();

    return tRet == 0;
}

fResult f2dVorbisDecoder::Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)
{
	if(pSizeRead)
		*pSizeRead = 0;

	m_pStream->Lock(); // 锁定流
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, m_CurrentPos);

	fuInt tSizeRead = 0;
	char* tBuffer = (char*)pBuffer;
	while(tSizeRead < SizeToRead)
	{
		long tRet = ov_read(&m_OggFile, tBuffer, SizeToRead - tSizeRead, 0, 2, 1, &m_CurrentSec);
		if(tRet<0)     // 错误
		{
			m_CurrentPos = m_pStream->GetPosition();
			m_pStream->Unlock();
			return FCYERR_INTERNALERR;
		}
		if(tRet == 0)  // 到尾部
			break;
		tSizeRead += tRet;
		tBuffer += tRet;
	}

	if(pSizeRead)
		*pSizeRead = tSizeRead;

	m_CurrentPos = m_pStream->GetPosition();
	m_pStream->Unlock();

    return FCYERR_OK;
}
