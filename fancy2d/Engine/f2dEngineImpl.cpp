#include "Engine/f2dEngineImpl.h"

#include "Input/f2dInputSysImpl.h"
#include "Sound/f2dSoundSysImpl.h"
//#include "Video/f2dVideoSysImpl.h"
#include "Renderer/f2dRendererImpl.h"

#include "RenderDev/f2dRenderDeviceImpl.h"
#include "RenderDev/f2dRenderDevice11.h"

#include "fcyOS/fcyDebug.h"
#include "fcyOS/fcyCPUID.h"

#include "f2dConfig.h"
#include "f2d.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// 导出函数
F2DEXTERNC fResult F2DDLLFUNC CreateF2DEngine(fuInt Version, f2dEngineEventListener* pListener, f2dEngine** pOut, f2dInitialErrListener* pErrListener)
{
	if(!pOut)
		return FCYERR_ILLEGAL;
	*pOut = NULL;

	if(Version != F2DVERSION)
		return FCYERR_INVAILDVERSION;
	
	try
	{
		*pOut = new f2dEngineImpl(pListener);
	}
	catch(const fcyException& e)
	{
		if(pErrListener)
			pErrListener->OnErr(e.GetTime(), e.GetSrc(), e.GetDesc());

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

F2DEXTERNC fResult F2DDLLFUNC CreateF2DEngineAndInit(fuInt Version, const fcyRect& WinPos, fcStrW Title, fBool Windowed, fBool VSync, F2DAALEVEL AA, f2dEngineEventListener* pListener, f2dEngine** pOut, f2dInitialErrListener* pErrListener)
{
	if(!pOut)
		return FCYERR_ILLEGAL;
	*pOut = NULL;

	if(Version != F2DVERSION)
		return FCYERR_INVAILDVERSION;
	
	try
	{
		*pOut = new f2dEngineImpl(WinPos, Title, Windowed, VSync, AA, pListener);
	}
	catch(const fcyException& e)
	{
		if(pErrListener)
			pErrListener->OnErr(e.GetTime(), e.GetSrc(), e.GetDesc());

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

fuInt f2dEngineImpl::UpdateAndRenderThread::ThreadJob()
{
	// 线程相关设置
	SetThreadAffinityMask(GetCurrentThread(), 1);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	// 初始化计数器和FPS控制器
	fcyStopWatch tTimer;
	f2dFPSControllerImpl tFPSController(m_MaxFPS);

	// 获得渲染设备
	f2dRenderDevice* tpRenderDev = NULL;
	if(m_pEngine->GetRenderer())
		tpRenderDev = m_pEngine->GetRenderer()->GetDevice();

	// 开始计时
	tTimer.Reset();

	// 执行渲染更新循环
	fcyCriticalSection& tLock = m_pEngine->m_Sec;
	fBool bExit = false;
	fBool bDoPresent = false;
	fDouble tTime = 0;
	while(1)
	{
		tLock.Lock();
		bExit = m_pEngine->m_bStop;
		tLock.UnLock();

		// 检查退出
		if(bExit)
			break;
		
		// 更新FPS
		tTime = tFPSController.Update(tTimer);
		
		// 执行显示事件
		//if(bDoPresent)
			//m_pEngine->DoPresent(tpRenderDev);

		// 执行更新事件
		m_pEngine->DoUpdate(tTime, &tFPSController);

		// 执行渲染事件
		bDoPresent = m_pEngine->DoRender(tTime, &tFPSController, tpRenderDev);

		// 执行显示事件
		if (bDoPresent)
			m_pEngine->DoPresent(tpRenderDev);
	}
	
	// 投递终止消息
	PostThreadMessageW(m_MainThreadID, WM_USER, 0, 0);
	
	return 0;
}

fuInt f2dEngineImpl::UpdateThread::ThreadJob()
{
	// 初始化计数器和FPS控制器
	fcyStopWatch tTimer;
	f2dFPSControllerImpl tFPSController(m_MaxFPS);

	// 开始计时
	tTimer.Reset();

	// 执行渲染更新循环
	fcyCriticalSection& tLock = m_pEngine->m_Sec;
	fBool bExit = false;
	fDouble tTime = 0;
	while(1)
	{
		tLock.Lock();
		bExit = m_pEngine->m_bStop;
		tLock.UnLock();

		// 检查退出
		if(bExit)
			break;

		// 更新FPS
		tTime = tFPSController.Update(tTimer);
		
		// 执行更新事件
		m_pEngine->DoUpdate(tTime, &tFPSController);
	}

	// 投递终止消息
	PostThreadMessageW(m_MainThreadID, WM_USER, 0, 0);

	return 0;
}

fuInt f2dEngineImpl::RenderThread::ThreadJob()
{
	// 初始化计数器和FPS控制器
	fcyStopWatch tTimer;
	f2dFPSControllerImpl tFPSController(m_MaxFPS);

	// 获得渲染设备
	f2dRenderDevice* tpRenderDev = NULL;
	if(m_pEngine->GetRenderer())
		tpRenderDev = m_pEngine->GetRenderer()->GetDevice();

	// 开始计时
	tTimer.Reset();

	// 执行渲染更新循环
	fcyCriticalSection& tLock = m_pEngine->m_Sec;
	fBool bExit = false;
	fBool bDoPresent = false;
	fDouble tTime = 0;
	while(1)
	{
		tLock.Lock();
		bExit = m_pEngine->m_bStop;
		tLock.UnLock();

		// 检查退出
		if(bExit)
			break;

		// 更新FPS
		tTime = tFPSController.Update(tTimer);

		// 执行显示事件
		if(bDoPresent)
			m_pEngine->DoPresent(tpRenderDev);
		
		// 执行渲染事件
		bDoPresent = m_pEngine->DoRender(tTime, &tFPSController, tpRenderDev);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

f2dEngineImpl::f2dEngineImpl(f2dEngineEventListener* pListener)
	: m_bStop(true), m_pListener(pListener), m_LastErrTime(0), m_PumpIndex(0),
	m_MainThreadID(GetCurrentThreadId()), m_ThreadMode(F2DENGTHREADMODE_SINGLETHREAD),
	m_WinClass(this, L"F2DRenderWindow"),
	m_pWindow(NULL), m_pSoundSys(NULL), m_pInputSys(NULL), m_pRenderer(NULL), m_pVideoSys(NULL)
{
	m_CPUString = fcyCPUID::GetCPUString();
	m_CPUBrandString = fcyCPUID::GetCPUBrand();
}

f2dEngineImpl::f2dEngineImpl(const fcyRect& WinPos, fcStrW Title, fBool Windowed, fBool VSync, F2DAALEVEL AA, f2dEngineEventListener* pListener)
	: m_bStop(true), m_pListener(pListener), m_LastErrTime(0), m_PumpIndex(0),
	m_MainThreadID(GetCurrentThreadId()), m_ThreadMode(F2DENGTHREADMODE_SINGLETHREAD),
	m_WinClass(this, L"F2DRenderWindow"),
	m_pWindow(NULL), m_pSoundSys(NULL), m_pInputSys(NULL), m_pRenderer(NULL), m_pVideoSys(NULL)
{
	m_CPUString = fcyCPUID::GetCPUString();
	m_CPUBrandString = fcyCPUID::GetCPUBrand();

	// 初始化部件
	try
	{
		m_pWindow = m_WinClass.CreateRenderWindow(WinPos, Title, false, F2DWINBORDERTYPE_FIXED);
		m_pRenderer = new f2dRendererImpl(this, (fuInt)WinPos.GetWidth(), (fuInt)WinPos.GetHeight(), Windowed, VSync, AA);
		m_pSoundSys = new f2dSoundSysImpl(this);
#ifndef _M_ARM
		m_pInputSys = new f2dInputSysImpl(this);
		//m_pVideoSys = new f2dVideoSysImpl(this);
#endif
	}
	catch(...)
	{
		//FCYSAFEKILL(m_pVideoSys);
		FCYSAFEKILL(m_pRenderer);
		FCYSAFEKILL(m_pInputSys);
		FCYSAFEKILL(m_pSoundSys);
		FCYSAFEKILL(m_pWindow);

		throw;
	}
}

f2dEngineImpl::~f2dEngineImpl()
{
	// 销毁组件
	//FCYSAFEKILL(m_pVideoSys);
	FCYSAFEKILL(m_pRenderer);
	FCYSAFEKILL(m_pInputSys);
	FCYSAFEKILL(m_pSoundSys);
	FCYSAFEKILL(m_pWindow);
}

void f2dEngineImpl::ThrowException(const fcyException& e)
{
	FCYDEBUGEXCPT(e);
	
	// 设置最近一次错误
	m_LastErrTime = e.GetTime();
	m_LastErrSrc = e.GetSrc();
	m_LastErrDesc = e.GetDesc();
	
	// 封装并抛出消息
	f2dMsgMemHelper<fcyException>* tObjMem = new f2dMsgMemHelper<fcyException>(e);
	SendMsg(
		F2DMSG_APP_ONEXCEPTION,
		e.GetTime(), 
		(fuLong)tObjMem->GetObj().GetSrc(),
		(fuLong)tObjMem->GetObj().GetDesc(),
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}

fResult f2dEngineImpl::InitWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border)
{
	if(m_pWindow)
		return FCYERR_ILLEGAL;

	try
	{
		m_pWindow = m_WinClass.CreateRenderWindow(Pos, Title, false, F2DWINBORDERTYPE_FIXED);
	}
	catch(const fcyException& e)
	{
		ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dEngineImpl::InitSoundSys()
{
	if(m_pSoundSys || !m_pWindow)
		return FCYERR_ILLEGAL;

	try
	{
		m_pSoundSys = new f2dSoundSysImpl(this);
	}
	catch(const fcyException& e)
	{
		ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dEngineImpl::InitInputSys()
{
#ifndef _M_ARM
	if(m_pInputSys || !m_pWindow)
		return FCYERR_ILLEGAL;

	try
	{
		m_pInputSys = new f2dInputSysImpl(this);
	}
	catch(const fcyException& e)
	{
		ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
#else
	return FCYERR_NOTSUPPORT;
#endif
}

fResult f2dEngineImpl::InitRenderer(fuInt BufferWidth, fuInt BufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
{
	if(m_pRenderer || !m_pWindow)
		return FCYERR_ILLEGAL;
	
	try
	{
		m_pRenderer = new f2dRendererImpl(this, BufferWidth, BufferHeight, Windowed, VSync, AALevel);
	}
	catch(const fcyException& e)
	{
		ThrowException(e);
		return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}

fResult f2dEngineImpl::InitVideoSys()
{
#ifndef _M_ARM
	//if(m_pVideoSys || !m_pRenderer)
		return FCYERR_ILLEGAL;
	
	try
	{
		//m_pVideoSys = new f2dVideoSysImpl(this);
	}
	catch(const fcyException& e)
	{
		ThrowException(e);
		return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
#else
	return FCYERR_NOTSUPPORT;
#endif
}

f2dSoundSys* f2dEngineImpl::GetSoundSys() { return m_pSoundSys; }
f2dInputSys* f2dEngineImpl::GetInputSys() { return m_pInputSys; }
f2dRenderer* f2dEngineImpl::GetRenderer() { return m_pRenderer; }
//f2dVideoSys* f2dEngineImpl::GetVideoSys() { return m_pVideoSys; }
f2dVideoSys* f2dEngineImpl::GetVideoSys() { return NULL; }

void f2dEngineImpl::Abort()
{
	m_Sec.Lock();
	m_bStop = true;
	m_Sec.UnLock();
}

void f2dEngineImpl::Run(F2DENGTHREADMODE ThreadMode, fuInt UpdateMaxFPS, fuInt RenderMaxFPS)
{
	m_Sec.Lock();
	m_bStop = false;
	m_Sec.UnLock();
	m_ThreadMode = ThreadMode;
	switch(ThreadMode)
	{
	case F2DENGTHREADMODE_SINGLETHREAD:
		Run_SingleThread(UpdateMaxFPS);
		break;
	case F2DENGTHREADMODE_MULTITHREAD:
		Run_MultiThread(UpdateMaxFPS);
		break;
	case F2DENGTHREADMODE_FULLMULTITHREAD:
		Run_FullMultiThread(UpdateMaxFPS, RenderMaxFPS);
		break;
	}
}

fResult f2dEngineImpl::SendMsg(const f2dMsg& Msg, f2dInterface* pMemObj)
{
	m_Sec.Lock();
	m_MsgPump[m_PumpIndex].Push(Msg, pMemObj);
	m_Sec.UnLock();
	return FCYERR_OK;
}

void f2dEngineImpl::Run_SingleThread(fuInt UpdateMaxFPS)
{
	// 线程相关设置
	SetThreadAffinityMask(GetCurrentThread(), 1);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	// 初始化计数器和FPS控制器
	fcyStopWatch tTimer;
	fcyStopWatch tMsgTimer;
	f2dFPSControllerImpl tFPSController(UpdateMaxFPS);

	// 获得渲染设备
	f2dRenderDevice* tpRenderDev = NULL;
	if(m_pRenderer)
		tpRenderDev = m_pRenderer->GetDevice();

	// 开始计时
	tTimer.Reset();

	// 执行程序循环
	fBool bExit = false;
	fBool bDoPresent = false;
	fDouble tTime = 0;
	fDouble tMsgTime = 0;
	MSG tMsg;
	while(1)
	{
		m_Sec.Lock();
		bExit = m_bStop;
		m_Sec.UnLock();

		if(bExit)
			break;
		
		tTime = tFPSController.Update(tTimer);
		
		// 应用程序消息处理
		{
			//tMsgTimer.Reset();
			if(PeekMessageW(&tMsg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&tMsg);
				DispatchMessageW(&tMsg);

				// 发送退出消息
				if(tMsg.message == WM_QUIT)
					SendMsg(F2DMSG_APP_ONEXIT);
			}
			//tMsgTime = tMsgTimer.GetElapsed();
		}
		
		// 更新FPS
		//tTime = tFPSController.Update(tTimer) - tMsgTime;  // 修正由于处理消息额外耗费的时间
		
		
		// 执行显示事件
		//if(bDoPresent)
			//DoPresent(tpRenderDev);
		
		// 执行更新事件
		DoUpdate(tTime, &tFPSController);
		
		// 执行渲染事件
		bDoPresent = DoRender(tTime, &tFPSController, tpRenderDev);
		
		// 执行显示事件
		if (bDoPresent)
			DoPresent(tpRenderDev);
	}
}

void f2dEngineImpl::Run_MultiThread(fuInt UpdateMaxFPS)
{
	// 创建工作线程并执行
	UpdateAndRenderThread tThread(this, UpdateMaxFPS);
	tThread.Resume();
	
	// 执行程序循环
	fBool bExit = false;
	MSG tMsg = {};
	while(1)
	{
		m_Sec.Lock();
		bExit = m_bStop;
		m_Sec.UnLock();

		if(bExit)
		{
			while(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)tThread.GetHandle(), 10))
			{
				if(PeekMessageW(&tMsg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&tMsg);
					DispatchMessageW(&tMsg);
				}
			}
			
			break;
		}
		
		// 应用程序消息处理
		if(GetMessageW(&tMsg, 0, 0, 0))
		{
			TranslateMessage(&tMsg);
			DispatchMessageW(&tMsg);
		}
		else
		{
			// 发送退出消息
			if(tMsg.message == WM_QUIT)
			{
				SendMsg(F2DMSG_APP_ONEXIT);
			}
		}
	}
	
	// 等待工作线程
	tThread.Wait();
}

void f2dEngineImpl::Run_FullMultiThread(fuInt UpdateMaxFPS, fuInt RenderMaxFPS)
{
	// 创建工作线程并执行
	UpdateThread tUpdateThread(this, UpdateMaxFPS);
	RenderThread tRenderThread(this, RenderMaxFPS);
	tUpdateThread.Resume();
	tRenderThread.Resume();
	
	// 执行程序循环
	fBool bExit = false;
	MSG tMsg;
	while(1)
	{
		m_Sec.Lock();
		bExit = m_bStop;
		m_Sec.UnLock();
		
		if(bExit)
		{
			while(1)
			{
				if(WAIT_TIMEOUT != WaitForSingleObject((HANDLE)tUpdateThread.GetHandle(), 10))
					if(WAIT_TIMEOUT != WaitForSingleObject((HANDLE)tRenderThread.GetHandle(), 10))
						break;
				
				if(PeekMessageW(&tMsg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&tMsg);
					DispatchMessageW(&tMsg);
				}
			}
			
			break;
		}
		
		// 应用程序消息处理
		if(GetMessageW(&tMsg, 0, 0, 0))
		{
			TranslateMessage(&tMsg);
			DispatchMessageW(&tMsg);
		}
		else
		{
			// 发送退出消息
			if(tMsg.message == WM_QUIT)
				SendMsg(F2DMSG_APP_ONEXIT);
		}
	}
	
	// 等待工作线程
	tUpdateThread.Wait();
	tRenderThread.Wait();
}

void f2dEngineImpl::DoUpdate(fDouble ElapsedTime, f2dFPSControllerImpl* pFPSController)
{
	f2dMsgPump* tPump = NULL;

	// 交换消息泵并清空消息泵
	m_Sec.Lock();
	tPump = &m_MsgPump[ m_PumpIndex ];
	m_PumpIndex = !m_PumpIndex;
	m_MsgPump[m_PumpIndex].Clear();
	m_Sec.UnLock();

	// 更新输入设备
	if(m_pInputSys)
		m_pInputSys->Update();

	// 执行监听器
	if(m_pListener)
	{
		if(false == m_pListener->OnUpdate(ElapsedTime, pFPSController, tPump))
			Abort();
	}
}

bool f2dEngineImpl::DoRender(fDouble ElapsedTime, f2dFPSControllerImpl* pFPSController, f2dRenderDevice* pDev)
{
	// 同步设备状态，处理设备丢失
#ifdef F2D_GRAPHIC_API_D3D11
	if(pDev && FCYOK(((f2dRenderDevice11*)pDev)->SyncDevice()))
#else
	if (pDev && FCYOK(((f2dRenderDeviceImpl*)pDev)->SyncDevice()))
#endif
	{
		if(m_pListener) // 触发渲染事件
			return m_pListener->OnRender(ElapsedTime, pFPSController);
	}

	return false;
}

void f2dEngineImpl::DoPresent(f2dRenderDevice* pDev)
{
	// 递交画面
#ifdef F2D_GRAPHIC_API_D3D11
	((f2dRenderDevice11*)pDev)->Present();
#else
	((f2dRenderDeviceImpl*)pDev)->Present();
#endif
}
