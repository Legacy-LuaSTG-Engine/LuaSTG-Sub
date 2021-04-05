#include "Sound/f2dSoundBufferPull.h"

////////////////////////////////////////////////////////////////////////////////

f2dSoundBufferPull::SoundBufferFiller::SoundBufferFiller(f2dSoundBufferPull* pParent)
	: m_pParent(pParent)
{}

f2dSoundBufferPull::SoundBufferFiller::~SoundBufferFiller()
{}

fuInt f2dSoundBufferPull::SoundBufferFiller::ThreadJob()
{
	bool tRunning = true;
	HANDLE tWaitList[3] =
	{
		(HANDLE)m_pParent->m_EvtBegin.GetHandle(),
		(HANDLE)m_pParent->m_EvtHalf.GetHandle(),
		(HANDLE)m_pParent->m_EvtStop.GetHandle()
	};

	while (tRunning)
	{
		DWORD tRet = WaitForMultipleObjects(3, tWaitList, FALSE, -1);
		switch (tRet)
		{
		case WAIT_TIMEOUT:
			break;
		case WAIT_FAILED:
			break;
		default:
		{
			// 检查线程退出
			if (m_pParent->isThreadHalt())
				return 0;

			fuInt nIndex = tRet - WAIT_OBJECT_0;
			switch (nIndex)
			{
			case 0:
				m_pParent->m_Sec.Lock();
				m_pParent->fillBuffer(1);
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
				if (m_pParent->IsLoop())
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

f2dSoundBufferPull::f2dSoundBufferPull(IDirectSound8* pSound, f2dSoundDecoder* pDecoder, fuInt iBufferSampleCount, fBool bGlobalFocus)
	: m_pBuffer(NULL), m_pNotify(NULL), m_pDecoder(pDecoder),
	m_pFiller(NULL), m_bHalt(false), m_iBufferSize(0)
{
	if (!m_pDecoder)
		throw fcyException("f2dSoundBufferPull::f2dSoundBufferPull", "Invalid Pointer.");
	m_pDecoder->AddRef();

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
	if (bGlobalFocus)
		tDesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
	tDesc.dwBufferBytes = m_iBufferSize = iBufferSampleCount * tFmtDesc.nBlockAlign;  // 动态缓冲
	tDesc.lpwfxFormat = &tFmtDesc;

	HRESULT tHR = pSound->CreateSoundBuffer(&tDesc, &m_pBuffer, NULL);
	if (FAILED(tHR))
	{
		FCYSAFEKILL(m_pDecoder);
		throw fcyException("f2dSoundBufferPull::f2dSoundBufferPull", "CreateSoundBuffer Failed.");
	}

	tHR = m_pBuffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&m_pNotify);
	if (FAILED(tHR))
	{
		FCYSAFEKILL(m_pBuffer);
		FCYSAFEKILL(m_pDecoder);
		throw fcyException("f2dSoundBufferPull::f2dSoundBufferPull", "QueryInterface Failed.");
	}

	preInit(0);

	// 启动监听线程
	m_pFiller = new SoundBufferFiller(this);
	m_pFiller->Resume();
}

f2dSoundBufferPull::~f2dSoundBufferPull()
{
	// 终止线程
	haltThread();

	FCYSAFEDEL(m_pFiller);
	FCYSAFEKILL(m_pDecoder);
	FCYSAFEKILL(m_pNotify);
	FCYSAFEKILL(m_pBuffer);
}

void f2dSoundBufferPull::preInit(fuInt StartPos)
{
	m_Sec.Lock();

	m_pBuffer->SetCurrentPosition(0);
	regNotify();   // 注册监听器
	fillBuffer(0); // 填充第一块
	
	m_Sec.UnLock();
}

void f2dSoundBufferPull::fillBuffer(fuInt Index)
{
	// 计算填充位置
	fuInt tPosToFill = Index * (m_iBufferSize / 2);

	// 获得指针
	fByte *tFillPtr = NULL;
	fuInt tFillSize = 0, tRealSize = 0;
	m_pBuffer->Lock(tPosToFill, m_iBufferSize / 2, (void**)&tFillPtr, (DWORD*)&tFillSize, NULL, NULL, 0);

	memset(tFillPtr, 0, tFillSize);
	m_pDecoder->Read(tFillPtr, tFillSize, &tRealSize);
	if (tRealSize<tFillSize)
	{
		// 插入一个终止信号
		fuInt tStopPos = tPosToFill + tRealSize;

		regStopNotify(tStopPos);
	}

	m_pBuffer->Unlock(tFillPtr, tFillSize, NULL, 0);
}

void f2dSoundBufferPull::regNotify()
{
	DSBPOSITIONNOTIFY tNotifyList[2] =
	{
		{ 0, (HANDLE)m_EvtBegin.GetHandle() },
		{ m_iBufferSize / 2, (HANDLE)m_EvtHalf.GetHandle() }
	};

	m_pNotify->SetNotificationPositions(2, tNotifyList);
}

void f2dSoundBufferPull::regStopNotify(fuInt Pos)
{
	m_pBuffer->Stop();

	DSBPOSITIONNOTIFY tNotifyList[1] =
	{
		{ Pos, (HANDLE)m_EvtStop.GetHandle() }
	};

	m_pNotify->SetNotificationPositions(1, tNotifyList);

	m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

fBool f2dSoundBufferPull::isThreadHalt()
{
	bool tValue;

	m_Sec.Lock();
	tValue = m_bHalt;
	m_Sec.UnLock();

	return tValue;
}

void f2dSoundBufferPull::haltThread()
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

fBool f2dSoundBufferPull::IsDynamic()
{
	return true;
}

void f2dSoundBufferPull::Play()
{
	m_Sec.Lock();
	m_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
	m_Sec.UnLock();
}

void f2dSoundBufferPull::Stop()
{
}

void f2dSoundBufferPull::Pause()
{
	m_Sec.Lock();
	m_pBuffer->Stop();
	m_Sec.UnLock();
}

fBool f2dSoundBufferPull::IsLoop()
{
	return false;
}

void f2dSoundBufferPull::SetLoop(fBool bValue)
{
}

fBool f2dSoundBufferPull::IsPlaying()
{
	DWORD tV = 0;
	m_Sec.Lock();
	m_pBuffer->GetStatus(&tV);
	m_Sec.UnLock();
	return tV & DSBSTATUS_PLAYING;
}

fDouble f2dSoundBufferPull::GetTotalTime()
{
	return 0.;
}

fDouble f2dSoundBufferPull::GetTime()
{
	return 0.;
}

fResult f2dSoundBufferPull::SetTime(fDouble Time)
{
	return FCYERR_NOTSUPPORT;
}

fFloat f2dSoundBufferPull::GetVolume()
{
	LONG tRet = 0;

	m_Sec.Lock();
	m_pBuffer->GetVolume(&tRet);
	m_Sec.UnLock();

	return (tRet - DSBVOLUME_MIN) / (float)(DSBVOLUME_MAX - DSBVOLUME_MIN);
}

fResult f2dSoundBufferPull::SetVolume(fFloat Value)
{
	if (Value<0.f)
		Value = 0.f;
	else if (Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * (DSBVOLUME_MAX - DSBVOLUME_MIN) + DSBVOLUME_MIN);
	HRESULT tHR;
	m_Sec.Lock();
	tHR = m_pBuffer->SetVolume(tValue);
	m_Sec.UnLock();

	return (FAILED(tHR) ? FCYERR_INTERNALERR : FCYERR_OK);
}

fFloat f2dSoundBufferPull::GetPan()
{
	LONG tRet = 0;

	m_Sec.Lock();
	m_pBuffer->GetPan(&tRet);
	m_Sec.UnLock();
	return (tRet) / (float)(DSBPAN_RIGHT);
}

fResult f2dSoundBufferPull::SetPan(fFloat Value)
{
	if (Value<-1.f)
		Value = -1.f;
	else if (Value>1.f)
		Value = 1.f;

	LONG tValue = (LONG)(Value * DSBPAN_RIGHT);
	HRESULT tHR;
	m_Sec.Lock();
	tHR = m_pBuffer->SetPan(tValue);
	m_Sec.UnLock();

	return (FAILED(tHR) ? FCYERR_INTERNALERR : FCYERR_OK);
}

fuInt f2dSoundBufferPull::GetFrequency()
{
	DWORD tRet = 0;
	m_pBuffer->GetFrequency(&tRet);
	return tRet;
}

fResult f2dSoundBufferPull::SetFrequency(fuInt Value)
{
	if (FAILED(m_pBuffer->SetFrequency(Value)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}
