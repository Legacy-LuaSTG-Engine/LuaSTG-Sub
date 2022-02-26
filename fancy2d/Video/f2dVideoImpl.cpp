#ifndef _M_ARM

#include "Video/f2dVideoImpl.h"

#include "Video/f2dVideoSysImpl.h"

#include <stack>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dVideoImpl::f2dVideoImpl(f2dVideoSysImpl* pParent, f2dStream* pStream, fuInt Flag)
	: m_ObjectTableEntry(0),
	m_pStream(NULL), m_pReader(NULL),
	m_pFilterGraph(NULL), m_pMediaControl(NULL), m_pMediaEvent(NULL), m_pMediaSeek(NULL),
	m_pVideoStep(NULL), m_pAudioControl(NULL),
	m_pRenderer(NULL), m_pTex(NULL),
	m_bLoop(false)
{
	HRESULT tHR = S_OK;

	// --- 构建流 ---
	m_pStream = new f2dVideoStream(pStream);
	m_pReader = new f2dVideoStreamReader(m_pStream, &tHR);
	if(FAILED(tHR))
	{
		FCYSAFEDEL(m_pReader);
		FCYSAFEDEL(m_pStream);

		throw fcyWin32COMException("f2dVideoImpl::f2dVideoImpl", "Create Stream Failed.", tHR);
	}

	m_pReader->AddRef();

	// --- 创建过滤器管理器 ---
	if(FAILED(tHR = 
		CoCreateInstance(
			CLSID_FilterGraph, 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			IID_IGraphBuilder, 
			(void **)&m_pFilterGraph
			)))
	{
		clearUp();
		throw fcyWin32COMException("f2dVideoImpl::f2dVideoImpl", "CoCreateInstance(CLSID_FilterGraph) Failed.", tHR);
	}
	
	// 查询接口，获得媒体控制和媒体事件接口
	m_pFilterGraph->QueryInterface(IID_IMediaControl,  (void**)&m_pMediaControl);
	m_pFilterGraph->QueryInterface(IID_IMediaEventEx,  (void**)&m_pMediaEvent);
	m_pFilterGraph->QueryInterface(IID_IMediaSeeking, (void**)&m_pMediaSeek);
	m_pFilterGraph->QueryInterface(IID_IVideoFrameStep,(void**)&m_pVideoStep);
	m_pFilterGraph->QueryInterface(IID_IBasicAudio,    (void**)&m_pAudioControl);

	// --- 设置时间单位 ---
	m_pMediaSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

	// --- 挂接调试器 ---
	if(Flag & F2DVIDEOFLAG_DEBUG)
		registerDebug();

	// --- 加载Filter ---
	for(fuInt i = 0; i<pParent->m_FliterGUIDList.size(); ++i)
		addFilter(pParent->m_FliterGUIDList[i]);

	// --- 挂接视频渲染器 ---
	tHR = S_OK;
	m_pRenderer = new f2dVideoRenderer(&tHR);
	if(FAILED(tHR))
	{
		FCYSAFEDEL(m_pRenderer);
		clearUp();
		throw fcyWin32COMException("f2dVideoImpl::f2dVideoImpl", "f2dVideoRenderer Init Failed.", tHR);
	}
	m_pRenderer->AddRef();
	
	tHR = m_pFilterGraph->AddFilter(m_pRenderer, L"f2dVideoRenderer");
	if(FAILED(tHR))
	{
		clearUp();
		throw fcyWin32COMException("f2dVideoImpl::f2dVideoImpl", "IGraphBuilder::AddFilter Failed.", tHR);
	}

	// --- 挂接流 ---
	m_pFilterGraph->AddFilter(m_pReader, L"f2dVideoStream");

	// --- 渲染流 ---
	if(FAILED(tHR = m_pFilterGraph->Render(m_pReader->GetPin(0))))
	{
		clearUp();
		throw fcyWin32COMException("f2dVideoImpl::f2dVideoImpl", "IGraphBuilder::Render Failed.", tHR);
	}
	
	// --- 检查视频流是否已被选用，若没有，返回失败 ---
	if(m_pRenderer->GetInternalPtr(0) == NULL)
	{
		clearUp();
		throw fcyException("f2dVideoImpl::f2dVideoImpl", "Video Render Connect Failed.");
	}

	// --- 创建内部纹理 ---
	if(! (Flag & F2DVIDEOFLAG_NOMANAGEDTEX))
	{
		if(FCYFAILED(pParent->GetRenderDevice()->CreateDynamicTexture(
			m_pRenderer->GetVideoWidth(),
			m_pRenderer->GetVideoHeight(),
			&m_pTex
			)))
		{
			clearUp();
			throw fcyException("f2dVideoImpl::f2dVideoImpl", "CreateDynamicTexture Failed.");
		}
	}

	// --- 清除没用的Filter ---
	clearUselessFilter();
}

f2dVideoImpl::~f2dVideoImpl()
{
	clearUp();
}

void f2dVideoImpl::clearUp()
{
	// 如果已在播放则先终止
	if(m_pMediaControl)
		m_pMediaControl->Stop();

	// 卸载调试器
	if(m_ObjectTableEntry)
		unregisterDebug();

	// 卸载DShow组件
	FCYSAFEKILL(m_pAudioControl);
	FCYSAFEKILL(m_pVideoStep);
	FCYSAFEKILL(m_pMediaSeek);
	FCYSAFEKILL(m_pMediaEvent);
	FCYSAFEKILL(m_pMediaControl);
	FCYSAFEKILL(m_pFilterGraph);

	// 卸载纹理
	FCYSAFEKILL(m_pTex);

	// 卸载视频对象
	FCYSAFEKILL(m_pRenderer);

	// 卸载流
	FCYSAFEDEL(m_pStream);
	FCYSAFEKILL(m_pReader);
}

void f2dVideoImpl::registerDebug()
{
	IMoniker * pMoniker = 0;
    IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
		WCHAR wsz[256];
		swprintf_s(wsz, L"FilterGraph %08p pid %08x", (void*)m_pFilterGraph, GetCurrentProcessId());
		HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
		if (SUCCEEDED(hr)) 
		{
			hr = objectTable->Register(0, m_pFilterGraph, pMoniker, &m_ObjectTableEntry);
			pMoniker->Release();
		}
		objectTable->Release();
	}
}

void f2dVideoImpl::unregisterDebug()
{
	IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
        objectTable->Revoke(m_ObjectTableEntry);
        objectTable->Release();
		m_ObjectTableEntry = 0;
    }
}

fBool f2dVideoImpl::addFilter(const f2dVideoFilterInfo& Filter)
{
	IBaseFilter* pFilter = NULL;
	HRESULT tHR = S_OK;

	if(Filter.hDLLInstance == NULL)
	{
		tHR = CoCreateInstance(Filter.FilterGUID, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pFilter);
		if(FAILED(tHR))
			return false;
	}
	else
	{
		BOOL tRet = f2dVideoSysImpl::createInstanceFromDll(Filter.hDLLInstance, Filter.FilterGUID, IID_IBaseFilter, (void**)&pFilter);
		if(tRet == FALSE)
			return false;
	}

	tHR = m_pFilterGraph->AddFilter(pFilter, NULL);

	FCYSAFEKILL(pFilter);

	return SUCCEEDED(tHR);
}

void f2dVideoImpl::clearUselessFilter()
{
	IEnumFilters* pEnum = NULL;
	IBaseFilter* pFilter = NULL;
	ULONG tFetchCount = 0;
	stack<IBaseFilter*> tFilterToRemove;

	if(FAILED(m_pFilterGraph->EnumFilters(&pEnum)))
		return;

	while(SUCCEEDED(pEnum->Next(1, &pFilter, &tFetchCount)))
	{
		if(tFetchCount == 0)
			break;

		IEnumPins* pEnumPins = NULL;
		IPin* pPin = NULL;
		ULONG tFetchPinCount = 0;
		fBool bConnected = false;

		if(SUCCEEDED(pFilter->EnumPins(&pEnumPins)))
		{
			while(SUCCEEDED(pEnumPins->Next(1, &pPin, &tFetchPinCount)))
			{
				if(tFetchPinCount == 0)
					break;

				IPin* pAnother = NULL;

				pPin->ConnectedTo(&pAnother);

				if(pAnother)
				{
					bConnected = true;
					FCYSAFEKILL(pAnother);
					FCYSAFEKILL(pPin);
					break;
				}

				FCYSAFEKILL(pPin);
			}

			FCYSAFEKILL(pEnumPins);
		}

		if(!bConnected)
			tFilterToRemove.push(pFilter);

		FCYSAFEKILL(pFilter);
	}

	FCYSAFEKILL(pEnum);

	while(!tFilterToRemove.empty())
	{
		m_pFilterGraph->RemoveFilter(tFilterToRemove.top());
		tFilterToRemove.pop();
	}
}

f2dVideoRendererListener* f2dVideoImpl::GetRendererListener()
{
	return m_pRenderer->GetRendererListener();
}

void f2dVideoImpl::SetRendererListener(f2dVideoRendererListener* pListener)
{
	m_pRenderer->SetRendererListener(pListener);
}

fResult f2dVideoImpl::Update()
{
	if(m_pTex)
		m_pRenderer->CopyDataToTexture(m_pTex);

	// 更新状态
	long tEvtCode = 0;
	LONG_PTR tEvtParam1 = NULL;
	LONG_PTR tEvtParam2 = NULL;
	while(SUCCEEDED(m_pMediaEvent->GetEvent(&tEvtCode, &tEvtParam1, &tEvtParam2, 0)))
	{
		m_pMediaEvent->FreeEventParams(tEvtCode, tEvtParam1, tEvtParam2);

		switch (tEvtCode)
		{
		case EC_COMPLETE:
			if(m_bLoop)
			{
				LONGLONG tCur = 0;
				m_pMediaSeek->SetPositions(&tCur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
			}
			else
			{
				LONGLONG tCur = 0;
				m_pMediaSeek->SetPositions(&tCur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
				m_pMediaControl->StopWhenReady();
			}
			break;
		};
	};

	return FCYERR_OK;
}

f2dTexture* f2dVideoImpl::GetVideoTex()
{
	return m_pTex;
}

fuInt f2dVideoImpl::GetVideoWidth()
{
	return m_pRenderer->GetVideoWidth();
}

fuInt f2dVideoImpl::GetVideoHeight()
{
	return m_pRenderer->GetVideoHeight();
}

fFloat f2dVideoImpl::GetVideoDataRate()
{
	return m_pRenderer->GetDataPerSecond();
}

fFloat f2dVideoImpl::GetVideoRenderFPS()
{
	return m_pRenderer->GetVideoRenderFPS();
}

fResult f2dVideoImpl::CopyDataToTexture(f2dTexture2D* pTex)
{
	return m_pRenderer->CopyDataToTexture(pTex);
}

fBool f2dVideoImpl::IsLoop()
{
	return m_bLoop;
}

void f2dVideoImpl::SetLoop(fBool bLoop)
{
	m_bLoop = bLoop;
}

F2DVIDEOSTATE f2dVideoImpl::GetState()
{
	F2DVIDEOSTATE tRet = F2DVIDEOSTATE_STOPPED;
	HRESULT tHR = m_pMediaControl->GetState(0, (OAFilterState*)&tRet);
	return tRet;
}

fResult f2dVideoImpl::Play()
{
	return SUCCEEDED(m_pMediaControl->Run())?FCYERR_OK : FCYERR_INTERNALERR;
}

fResult f2dVideoImpl::Pause()
{
	return SUCCEEDED(m_pMediaControl->Pause())?FCYERR_OK : FCYERR_INTERNALERR;
}

fResult f2dVideoImpl::Stop()
{
	LONGLONG tCur = 0;
	m_pMediaSeek->SetPositions(&tCur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);

	return SUCCEEDED(m_pMediaControl->StopWhenReady())?FCYERR_OK : FCYERR_INTERNALERR;
}

fDouble f2dVideoImpl::GetTotalTime()
{
	LONGLONG tTime = 0;

	m_pMediaSeek->GetDuration(&tTime);

	return tTime/1000./10000.;
}

fDouble f2dVideoImpl::GetCurTime()
{
	LONGLONG tTime = 0;

	m_pMediaSeek->GetPositions(&tTime, NULL);

	return tTime/1000./10000.;
}

fResult f2dVideoImpl::SetCurTime(fDouble Time)
{
	LONGLONG tTime = (LONGLONG)(Time * 1000 * 10000);

	if(FAILED(m_pMediaSeek->SetPositions(&tTime, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning)))
		return FCYERR_INTERNALERR;
	else
	{
		if(GetState() == F2DVIDEOSTATE_STOPPED)
			m_pMediaControl->StopWhenReady();
		else if(GetState() == F2DVIDEOSTATE_PAUSED)
		{
			m_pMediaControl->StopWhenReady();
			m_pMediaControl->Pause();
		}

		return FCYERR_OK;
	}
}

fResult f2dVideoImpl::Seek(fuInt KeyFrame)
{
	LONGLONG tTime = KeyFrame;

	m_pMediaSeek->SetTimeFormat(&TIME_FORMAT_FRAME);

	if(FAILED(m_pMediaSeek->SetPositions(&tTime, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, NULL, AM_SEEKING_NoPositioning)))
	{
		m_pMediaSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
		return FCYERR_INTERNALERR;
	}
	else
	{
		if(GetState() == F2DVIDEOSTATE_STOPPED)
			m_pMediaControl->StopWhenReady();
		else if(GetState() == F2DVIDEOSTATE_PAUSED)
		{
			m_pMediaControl->StopWhenReady();
			m_pMediaControl->Pause();
		}
	}

	m_pMediaSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

	return FCYERR_OK;
}

fDouble f2dVideoImpl::GetRate()
{
	double tRet = 1.;
	m_pMediaSeek->GetRate(&tRet);
	return tRet;
}

fResult f2dVideoImpl::SetRate(fDouble Rate)
{
	if(Rate < 0.)
		Rate = 0.;
	
	if(FCYFAILED(m_pMediaSeek->SetRate(Rate)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fFloat f2dVideoImpl::GetVolume()
{
	long pRet = 0;
	m_pAudioControl->get_Volume(&pRet);
	return (pRet + 10000) / 10000.f;
}

fResult f2dVideoImpl::SetVolume(fFloat Volume)
{
	if(Volume > 1.f)
		Volume = 1.f;
	else if(Volume < 0.f)
		Volume = 0.f;
	if(FCYFAILED(m_pAudioControl->put_Volume((long)((Volume - 1.f) * 10000.f))))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fFloat f2dVideoImpl::GetPan()
{
	LONG tRet = 0;
	m_pAudioControl->get_Balance(&tRet);
	return tRet/10000.f;
}

fResult f2dVideoImpl::SetPan(fFloat Pan)
{
	if(Pan < -1.f)
		Pan = -1.f;
	else if(Pan > 1.f)
		Pan = 1.f;
	
	if(FAILED(m_pAudioControl->put_Balance((long)(Pan * 10000))))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

#endif
