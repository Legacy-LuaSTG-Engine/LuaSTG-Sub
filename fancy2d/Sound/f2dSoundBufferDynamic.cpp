#include "Sound/f2dSoundBufferDynamic.h"

////////////////////////////////////////////////////////////////////////////////

const fuInt f2dSoundBufferDynamic::BufferSize = 65536;

f2dSoundBufferDynamic::SoundBufferFiller::SoundBufferFiller(f2dSoundBufferDynamic* pParent)
	: m_pParent(pParent)
{}

f2dSoundBufferDynamic::SoundBufferFiller::~SoundBufferFiller()
{}

fuInt f2dSoundBufferDynamic::SoundBufferFiller::ThreadJob()
{
	bool tRunning = true;
	HANDLE tWaitList[3] = 
	{
		m_pParent->m_EvtBegin.GetHandle(),
		m_pParent->m_EvtHalf.GetHandle(),
		m_pParent->m_EvtStop.GetHandle()
	};

	while(tRunning)
	{
		DWORD tRet = WaitForMultipleObjects(3, tWaitList, FALSE, -1);
		switch(tRet)
		{
		case WAIT_TIMEOUT:
			break;
		case WAIT_FAILED:
			break;
		default:
			{
				// 检查线程退出
				if(m_pParent->isThreadHalt())
					return 0;

				fuInt nIndex = tRet - WAIT_OBJECT_0;
				switch(nIndex)
				{
				case 0:
					m_pParent->m_Sec.Lock();
					m_pParent->fillBuffer(1);
					m_pParent->updateTime();
					m_pParent->m_EvtBegin.Reset();
					m_pParent->m_Sec.UnLock();
					break;
				case 1:
					m_pParent->m_Sec.Lock();
					m_pParent->fillBuffer(0);
					m_pParent->m_EvtHalf.Reset();
					m_pParent->m_Sec.UnLock();
					break;
				case 2:
					{
						m_pParent->Stop();
						if(m_pParent->IsLoop())
						{
							m_pParent->preInit(0);
							m_pParent->Play();
						}
						m_pParent->m_EvtStop.Reset();
					}
					break;
				}
			}
		};
	}
	return 0;
}

f2dSoundBufferDynamic::f2dSoundBufferDynamic(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fBool bGlobalFocus)
	: m_pBuffer(NULL), m_pNotify(NULL), m_pDecoder(pDecoder), 
	m_pFiller(NULL), m_bHalt(false),
	m_StartPos(0), m_PlayTime(0), m_bLoop(false), m_psSize(0), m_BufferSize(0)
{
	if(!m_pDecoder)
		throw fcyException("f2dSoundBufferDynamic::f2dSoundBufferDynamic", "Invalid Pointer.");
	m_pDecoder->AddRef();
	
	m_psSize = m_pDecoder->GetSamplesPerSec() * m_pDecoder->GetBlockAlign();
	m_BufferSize = m_pDecoder->GetBufferSize();

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
	tDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY;
	if(bGlobalFocus)
		tDesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
	tDesc.dwBufferBytes = BufferSize;  // 动态缓冲
	tDesc.lpwfxFormat = &tFmtDesc;
	
	HRESULT tHR = pSound->CreateSoundBuffer(&tDesc, &m_pBuffer, NULL);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pDecoder);
		throw fcyException("f2dSoundBufferDynamic::f2dSoundBufferDynamic", "CreateSoundBuffer Failed.");
	}

	tHR = m_pBuffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&m_pNotify);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pBuffer);
		FCYSAFEKILL(m_pDecoder);
		throw fcyException("f2dSoundBufferDynamic::f2dSoundBufferDynamic", "QueryInterface Failed.");
	}

	preInit(0);

	// 启动监听线程
	m_pFiller = new SoundBufferFiller(this);
	m_pFiller->Resume();
}

f2dSoundBufferDynamic::~f2dSoundBufferDynamic()
{
	// 终止线程
	haltThread();

	FCYSAFEDEL(m_pFiller);
	FCYSAFEKILL(m_pDecoder);
	FCYSAFEKILL(m_pNotify);
	FCYSAFEKILL(m_pBuffer);
}

void f2dSoundBufferDynamic::preInit(fuInt StartPos)
{
	m_Sec.Lock();

	m_pBuffer->SetCurrentPosition(0);
	m_StartPos = StartPos;
	m_pDecoder->SetPosition(FCYSEEKORIGIN_BEG, StartPos);
	regNotify();   // 注册监听器
	fillBuffer(0); // 填充第一块

	m_PlayTime = 0 - BufferSize;

	m_Sec.UnLock();
}

void f2dSoundBufferDynamic::updateTime()
{
	m_PlayTime += BufferSize;
}

void f2dSoundBufferDynamic::fillBuffer(fuInt Index)
{
	// 计算填充位置
	fuInt tPosToFill = Index * (BufferSize / 2);
	
	// 获得指针
	fByte *tFillPtr = NULL;
	fuInt tFillSize = 0, tRealSize = 0;
	m_pBuffer->Lock(tPosToFill, BufferSize / 2, (void**)&tFillPtr, (DWORD*)&tFillSize, NULL, NULL, 0);
	
	memset(tFillPtr, 0, tFillSize);
	m_pDecoder->Read(tFillPtr, tFillSize, &tRealSize);
	if(tRealSize<tFillSize)
	{
		// 插入一个终止信号
		fuInt tStopPos = tPosToFill + tRealSize;

		regStopNotify(tStopPos);
	}

	m_pBuffer->Unlock(tFillPtr, tFillSize, NULL, 0);
}

void f2dSoundBufferDynamic::regNotify()
{
	DSBPOSITIONNOTIFY tNotifyList[2] = 
	{
		{ 0, m_EvtBegin.GetHandle() },
		{ BufferSize / 2, m_EvtHalf.GetHandle() }
	};

	m_pNotify->SetNotificationPositions(2, tNotifyList);
}

void f2dSoundBufferDynamic::regStopNotify(fuInt Pos)
{
	m_pBuffer->Stop();

	DSBPOSITIONNOTIFY tNotifyList[1] = 
	{
		{ Pos, m_EvtStop.GetHandle() }
	};

	m_pNotify->SetNotificationPositions(1, tNotifyList);

	m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

fBool f2dSoundBufferDynamic::isThreadHalt()
{
	bool tValue;

	m_Sec.Lock();
	tValue = m_bHalt;
	m_Sec.UnLock();

	return tValue;
}

void f2dSoundBufferDynamic::haltThread()
{
	m_Sec.Lock();
	m_bHalt = true;
	m_Sec.UnLock();

	// 激活所有事件以使线程退出
	m_EvtBegin.Set();
	m_EvtHalf.Set();
	m_EvtStop.Set();

	m_pFiller->Wait();
}

fBool f2dSoundBufferDynamic::IsDynamic()
{
	return true;
}

void f2dSoundBufferDynamic::Play()
{
	m_Sec.Lock();
	m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
	m_Sec.UnLock();
}

void f2dSoundBufferDynamic::Stop()
{
	m_Sec.Lock();
	m_pBuffer->Stop();
	preInit(0);
	m_Sec.UnLock();
}

void f2dSoundBufferDynamic::Pause()
{
	m_Sec.Lock();
	m_pBuffer->Stop();
	m_Sec.UnLock();
}

fBool f2dSoundBufferDynamic::IsLoop()
{
	fBool tRet;
	m_Sec.Lock();
	tRet = m_bLoop;
	m_Sec.UnLock();

	return tRet;
}

void f2dSoundBufferDynamic::SetLoop(fBool bValue)
{
	m_Sec.Lock();
	m_bLoop = bValue;
	m_Sec.UnLock();
}

fBool f2dSoundBufferDynamic::IsPlaying()
{
	DWORD tV = 0;
	m_Sec.Lock();
	m_pBuffer->GetStatus(&tV);
	m_Sec.UnLock();
	return tV & DSBSTATUS_PLAYING;
}

fDouble f2dSoundBufferDynamic::GetTotalTime()
{
	return m_BufferSize/(double)m_psSize;
}

fDouble f2dSoundBufferDynamic::GetTime()
{
	DWORD tPlay;

	m_Sec.Lock();
	m_pBuffer->GetCurrentPosition(&tPlay, NULL);
	tPlay += m_StartPos;
	if(m_PlayTime >= 0)
		tPlay += m_PlayTime;
	m_Sec.UnLock();

	if(tPlay>m_BufferSize)
		tPlay = m_BufferSize;

	return tPlay/(double)m_psSize;
}

fResult f2dSoundBufferDynamic::SetTime(fDouble Time)
{
	fuInt tBytePos = (fuInt)(m_psSize * Time);
	
	if(tBytePos>m_BufferSize)
		return FCYERR_INVAILDPARAM;

	preInit(tBytePos);

	return FCYERR_OK;
}

fFloat f2dSoundBufferDynamic::GetVolume()
{
	LONG tRet = 0;

	m_Sec.Lock();
	m_pBuffer->GetVolume(&tRet);
	m_Sec.UnLock();

	return (tRet - DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX - DSBVOLUME_MIN);
}

fResult f2dSoundBufferDynamic::SetVolume(fFloat Value)
{
	if(Value<0.f)
		Value = 0.f;
	else if(Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * (DSBVOLUME_MAX - DSBVOLUME_MIN) + DSBVOLUME_MIN);
	HRESULT tHR;
	m_Sec.Lock();
	tHR = m_pBuffer->SetVolume(tValue);
	m_Sec.UnLock();

	return (FAILED(tHR)?FCYERR_INTERNALERR : FCYERR_OK);
}

fFloat f2dSoundBufferDynamic::GetPan()
{
	LONG tRet = 0;

	m_Sec.Lock();
	m_pBuffer->GetPan(&tRet);
	m_Sec.UnLock();
	return (tRet)/(float)(DSBPAN_RIGHT);
}

fResult f2dSoundBufferDynamic::SetPan(fFloat Value)
{
	if(Value<-1.f)
		Value = -1.f;
	else if(Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * DSBPAN_RIGHT);
	HRESULT tHR;
	m_Sec.Lock();
	tHR = m_pBuffer->SetPan(tValue);
	m_Sec.UnLock();

	return (FAILED(tHR)?FCYERR_INTERNALERR : FCYERR_OK);
}

fuInt f2dSoundBufferDynamic::GetFrequency()
{
	DWORD tRet = 0;
	m_pBuffer->GetFrequency(&tRet);
	return tRet;
}

fResult f2dSoundBufferDynamic::SetFrequency(fuInt Value)
{
	if(FAILED(m_pBuffer->SetFrequency(Value)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}
