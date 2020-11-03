#include "Sound/f2dSoundBufferStatic.h"

////////////////////////////////////////////////////////////////////////////////

f2dSoundBufferStatic::f2dSoundBufferStatic(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus)
	: m_bLoop(false), m_pBuffer(NULL)
{
	if(!pDecoder)
		throw fcyException("f2dSoundBufferStatic::f2dSoundBufferStatic", "Invalid Pointer.");

	// 计算常量
	m_BufferSize = pDecoder->GetBufferSize();
	m_psSize = pDecoder->GetSamplesPerSec() * pDecoder->GetBlockAlign();

	WAVEFORMATEX tFmtDesc;
	memset(&tFmtDesc, 0, sizeof(tFmtDesc));
	tFmtDesc.nAvgBytesPerSec = pDecoder->GetAvgBytesPerSec();
	tFmtDesc.nBlockAlign = pDecoder->GetBlockAlign();
	tFmtDesc.nChannels = pDecoder->GetChannelCount();
	tFmtDesc.nSamplesPerSec = pDecoder->GetSamplesPerSec();
	tFmtDesc.wFormatTag = pDecoder->GetFormatTag();
	tFmtDesc.wBitsPerSample = pDecoder->GetBitsPerSample();
	tFmtDesc.cbSize = sizeof(tFmtDesc);

	DSBUFFERDESC tDesc;
	memset(&tDesc, 0, sizeof(DSBUFFERDESC)); 
	tDesc.dwSize = sizeof(DSBUFFERDESC); 
	tDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	if(bGlobalFocus)
		tDesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
	tDesc.dwBufferBytes = m_BufferSize;  // 静态缓冲
	tDesc.lpwfxFormat = &tFmtDesc;
	
	HRESULT tHR = pSound->CreateSoundBuffer(&tDesc, &m_pBuffer, NULL);
	if(FAILED(tHR))
		throw fcyException("f2dSoundBufferStatic::f2dSoundBufferStatic", "CreateSoundBuffer Failed.");

	// 拷贝数据
	void* pBufferData = NULL;
	DWORD pBufferSize = 0;
	tHR = m_pBuffer->Lock(0, pDecoder->GetBufferSize(), &pBufferData, &pBufferSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if(FCYFAILED(pDecoder->Read((fByte*)pBufferData, pBufferSize, NULL)))
	{
		m_pBuffer->Unlock(pBufferData, pBufferSize, NULL, 0);
		FCYSAFEKILL(m_pBuffer);
		throw fcyException("f2dSoundBufferStatic::f2dSoundBufferStatic", "f2dSoundDecoder::Read Failed.");
	}
	m_pBuffer->Unlock(pBufferData, pBufferSize, NULL, 0);
}

f2dSoundBufferStatic::f2dSoundBufferStatic(IDirectSoundBuffer* pBuffer)
	: m_bLoop(false), m_pBuffer(pBuffer), m_BufferSize(0), m_psSize(0)
{}

f2dSoundBufferStatic::~f2dSoundBufferStatic()
{
	FCYSAFEKILL(m_pBuffer);
}

fBool f2dSoundBufferStatic::IsDynamic()
{
	return false;
}

void f2dSoundBufferStatic::Play()
{
	m_pBuffer->Play(0, 0, m_bLoop?DSBPLAY_LOOPING:0);
}

void f2dSoundBufferStatic::Stop()
{
	m_pBuffer->Stop();
	m_pBuffer->SetCurrentPosition(0);
}

void f2dSoundBufferStatic::Pause()
{
	m_pBuffer->Stop();
}

fBool f2dSoundBufferStatic::IsLoop()
{
	return m_bLoop;
}

void f2dSoundBufferStatic::SetLoop(fBool bValue)
{
	m_bLoop = bValue;
	if(IsPlaying())
	{
		Pause();
		Play();
	}
}

fBool f2dSoundBufferStatic::IsPlaying()
{
	DWORD tV = 0;
	m_pBuffer->GetStatus(&tV);
	return tV & DSBSTATUS_PLAYING;
}

fDouble f2dSoundBufferStatic::GetTotalTime()
{
	return m_BufferSize/(double)m_psSize;
}

fDouble f2dSoundBufferStatic::GetTime()
{
	DWORD tPlay;
	m_pBuffer->GetCurrentPosition(&tPlay, NULL);
	return tPlay/(double)m_psSize;
}

fResult f2dSoundBufferStatic::SetTime(fDouble Time)
{
	DWORD tPos = (DWORD)(Time * m_psSize);
	if(tPos>=m_BufferSize)
		return FCYERR_INVAILDPARAM;

	if(FAILED(m_pBuffer->SetCurrentPosition(tPos)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fFloat f2dSoundBufferStatic::GetVolume()
{
	LONG tRet = 0;
	m_pBuffer->GetVolume(&tRet);
	return (tRet - DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX - DSBVOLUME_MIN);
}

fResult f2dSoundBufferStatic::SetVolume(fFloat Value)
{
	if(Value<0.f)
		Value = 0.f;
	else if(Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * (DSBVOLUME_MAX - DSBVOLUME_MIN) + DSBVOLUME_MIN);
	if(FAILED(m_pBuffer->SetVolume(tValue)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fFloat f2dSoundBufferStatic::GetPan()
{
	LONG tRet = 0;
	m_pBuffer->GetPan(&tRet);
	return (tRet)/(float)(DSBPAN_RIGHT);
}

fResult f2dSoundBufferStatic::SetPan(fFloat Value)
{
	if(Value<-1.f)
		Value = -1.f;
	else if(Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * DSBPAN_RIGHT);
	if(FAILED(m_pBuffer->SetPan(tValue)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fuInt f2dSoundBufferStatic::GetFrequency()
{
	DWORD tRet = 0;
	m_pBuffer->GetFrequency(&tRet);
	return tRet;
}

fResult f2dSoundBufferStatic::SetFrequency(fuInt Value)
{
	if(FAILED(m_pBuffer->SetFrequency(Value)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}
