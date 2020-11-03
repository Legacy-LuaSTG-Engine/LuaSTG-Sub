#include "Sound/f2dSoundSpriteImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dSoundSpriteImpl::f2dSoundSpriteImpl(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus, fuInt MaxCount)
	: m_pParent(pSound), m_pBuffer(NULL), m_BufferSize(0), m_psSize(0), m_MaxCount(MaxCount)
{
	m_pParent->AddRef();

	if(!pDecoder)
		throw fcyException("f2dSoundSpriteImpl::f2dSoundSpriteImpl", "Invalid Pointer.");

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
		throw fcyException("f2dSoundSpriteImpl::f2dSoundSpriteImpl", "CreateSoundBuffer Failed.");

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

f2dSoundSpriteImpl::~f2dSoundSpriteImpl()
{
	deque<IDirectSoundBuffer*>::iterator i = m_BufferList.begin();
	while(i != m_BufferList.end())
	{
		IDirectSoundBuffer* pBuffer = *i;
		FCYSAFEKILL(pBuffer);
		++i;
	}

	FCYSAFEKILL(m_pBuffer);
	FCYSAFEKILL(m_pParent);
}

void f2dSoundSpriteImpl::Play()
{
	deque<IDirectSoundBuffer*>::iterator i = m_BufferList.begin();
	while(i != m_BufferList.end())
	{
		IDirectSoundBuffer* pBuffer = *i;
		DWORD tPos = 0;
		pBuffer->GetCurrentPosition(&tPos, NULL);
		if(tPos != 0)
			pBuffer->Play(0,0,0);
		++i;
	}
}

void f2dSoundSpriteImpl::Stop()
{
	deque<IDirectSoundBuffer*>::iterator i = m_BufferList.begin();
	while(i != m_BufferList.end())
	{
		IDirectSoundBuffer* pBuffer = *i;
		pBuffer->Stop();
		pBuffer->SetCurrentPosition(0);
		++i;
	}
}

void f2dSoundSpriteImpl::Pause()
{
	deque<IDirectSoundBuffer*>::iterator i = m_BufferList.begin();
	while(i != m_BufferList.end())
	{
		IDirectSoundBuffer* pBuffer = *i;
		pBuffer->Stop();
		++i;
	}
}

fDouble f2dSoundSpriteImpl::GetTotalTime()
{
	return m_BufferSize / (double)m_psSize;
}

fResult f2dSoundSpriteImpl::PlayNewSound(fFloat Volume, fFloat Pan)
{
	IDirectSoundBuffer* pTimeMax = NULL;
	DWORD tMaxPos = 0;

	IDirectSoundBuffer* pBuffer = NULL;

	deque<IDirectSoundBuffer*>::iterator i = m_BufferList.begin();
	while(i != m_BufferList.end())
	{
		IDirectSoundBuffer* tBuffer = *i;
		DWORD tFlag = 0;
		DWORD tPos = 0;
		tBuffer->GetStatus(&tFlag);
		tBuffer->GetCurrentPosition(&tPos, NULL);

		// 记录一个播放最久的缓冲区作为备选
		if(tPos > tMaxPos || !tMaxPos)
		{
			tMaxPos = tPos;
			pTimeMax = tBuffer;
		}

		// 记录一个播放停止的缓冲区作为最佳选择
		if(!(tFlag & DSBSTATUS_PLAYING) && tPos==0)
		{
			pBuffer = tBuffer;
			break;
		}
		++i;
	}

	if(!pBuffer)
	{
		// 考察是否超出缓冲上限
		if(m_BufferList.size() > m_MaxCount)
		{
			if(!pTimeMax)
				return FCYERR_INTERNALERR;

			// 寻找一个播放时间最长的
			pBuffer = pTimeMax;
		}
		else
		{
			HRESULT tHR = m_pParent->DuplicateSoundBuffer(m_pBuffer, &pBuffer);
			if(FAILED(tHR))
				return FCYERR_INTERNALERR;

			m_BufferList.push_back(pBuffer);
		}
	}

	// 修饰值
	if(Volume<0.f)
		Volume = 0.f;
	else if(Volume>1.f)
		Volume = 1.f;
	if(Pan<-1.f)
		Pan = -1.f;
	else if(Pan>1.f)
		Pan = 1.f;

	LONG tVolume = (LONG)(Volume * (DSBVOLUME_MAX - DSBVOLUME_MIN) + DSBVOLUME_MIN);
	pBuffer->SetVolume(tVolume);

	LONG tPan = (LONG)(Pan * DSBPAN_RIGHT);
	pBuffer->SetPan(tPan);

	pBuffer->SetCurrentPosition(0);
	pBuffer->Play(0, 0, 0);

	return FCYERR_OK;
}
