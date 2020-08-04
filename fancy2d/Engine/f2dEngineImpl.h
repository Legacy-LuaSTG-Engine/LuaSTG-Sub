////////////////////////////////////////////////////////////////////////////////
/// @file  f2dEngineImpl.h
/// @brief fancy2D引擎接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyRefObj.h>
#include <fcyIO/fcyStream.h>
#include <fcyOS/fcyMultiThread.h>

#include "Engine/f2dFPSControllerImpl.h"
#include "Engine/f2dMsgPumpImpl.h"

#include "Engine/f2dFileSysImpl.h"
#include "Engine/f2dWindowImpl.h"

class f2dWindowImpl;
class f2dSoundSysImpl;
class f2dInputSysImpl;
class f2dRendererImpl;
class f2dRenderDeviceImpl;
class f2dVideoSysImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D引擎接口实现
////////////////////////////////////////////////////////////////////////////////
class f2dEngineImpl :
	public fcyRefObjImpl<f2dEngine>
{
private:
	// 逻辑渲染线程
	class UpdateAndRenderThread :
		public fcyBaseThread
	{
	private:
		fuInt m_MaxFPS;
		DWORD m_MainThreadID;
		f2dEngineImpl* m_pEngine;
	protected:
		fuInt ThreadJob();
	public:
		UpdateAndRenderThread(f2dEngineImpl* pEngine, fuInt MaxFPS)
			: m_MaxFPS(MaxFPS), m_MainThreadID(GetCurrentThreadId()), m_pEngine(pEngine) {}
	};

	// 逻辑线程
	class UpdateThread :
		public fcyBaseThread
	{
	private:
		fuInt m_MaxFPS;
		DWORD m_MainThreadID;
		f2dEngineImpl* m_pEngine;
	protected:
		fuInt ThreadJob();
	public:
		UpdateThread(f2dEngineImpl* pEngine, fuInt MaxFPS)
			: m_MaxFPS(MaxFPS), m_MainThreadID(GetCurrentThreadId()), m_pEngine(pEngine) {}
	};

	// 渲染线程
	class RenderThread :
		public fcyBaseThread
	{
	private:
		fuInt m_MaxFPS;
		DWORD m_MainThreadID;
		f2dEngineImpl* m_pEngine;
	protected:
		fuInt ThreadJob();
	public:
		RenderThread(f2dEngineImpl* pEngine, fuInt MaxFPS)
			: m_MaxFPS(MaxFPS), m_MainThreadID(GetCurrentThreadId()), m_pEngine(pEngine) {}
	};
private:
	// 状态
	bool m_bStop;

	// 监听器
	f2dEngineEventListener* m_pListener;

	// 锁
	fcyCriticalSection m_Sec;
	DWORD m_MainThreadID;
	F2DENGTHREADMODE m_ThreadMode;

	// CPU信息
	std::string m_CPUString;
	std::string m_CPUBrandString;

	// 上一次错误
	fuInt m_LastErrTime;
	std::string m_LastErrSrc;
	std::string m_LastErrDesc;

	// 长生存期组件
	f2dWindowClass m_WinClass;
	f2dFileSysImpl m_FileSys;

	// 组件
	f2dWindowImpl*   m_pWindow;
	f2dSoundSysImpl* m_pSoundSys;
	f2dInputSysImpl* m_pInputSys;
	f2dRendererImpl* m_pRenderer;
	f2dVideoSysImpl* m_pVideoSys;

	// 消息泵
	fuInt m_PumpIndex;
	f2dMsgPumpImpl m_MsgPump[2];
protected:
	void Run_SingleThread(fuInt UpdateMaxFPS);
	void Run_MultiThread(fuInt UpdateMaxFPS);
	void Run_FullMultiThread(fuInt UpdateMaxFPS, fuInt RenderMaxFPS);

	void DoUpdate(fDouble ElapsedTime, f2dFPSControllerImpl* pFPSController);
	bool DoRender(fDouble ElapsedTime, f2dFPSControllerImpl* pFPSController, f2dRenderDeviceImpl* pDev);
	void DoPresent(f2dRenderDeviceImpl* pDev);
public: // 内部公开方法
	// 当异常被触发时调用该函数
	void ThrowException(const fcyException& e);
public: // 接口实现
	f2dEngineEventListener* GetListener()                  { return m_pListener; }
	fResult SetListener(f2dEngineEventListener* pListener) { m_pListener = pListener; return FCYERR_OK; }

	fResult InitWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border);
	fResult InitSoundSys();
	fResult InitInputSys();
	fResult InitRenderer(fuInt BufferWidth, fuInt BufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel);
	fResult InitVideoSys();

	fcyCriticalSection& GetCriticalSection() { return m_Sec; }
	f2dWindow* GetMainWindow() { return m_pWindow;   }
	f2dFileSys* GetFileSys()   { return &m_FileSys;  }
	f2dSoundSys* GetSoundSys();
	f2dInputSys* GetInputSys();
	f2dRenderer* GetRenderer();
	f2dVideoSys* GetVideoSys();

	void ClearLstErr()
	{
		m_LastErrTime = 0;
		m_LastErrSrc = m_LastErrDesc = "";
	}
	fuInt GetLastErrTime() { return m_LastErrTime;         }
	fcStr GetLastErrSrc()  { return m_LastErrSrc.c_str();  }
	fcStr GetLastErrDesc() { return m_LastErrDesc.c_str(); }

	void Abort();
	void Run(F2DENGTHREADMODE ThreadMode, fuInt UpdateMaxFPS, fuInt RenderMaxFPS);
	F2DENGTHREADMODE GetCurrentThreadMode() { return m_ThreadMode; }

	fResult SendMsg(const f2dMsg& Msg, f2dInterface* pMemObj = NULL);
	fResult SendMsg(F2DMSGTYPE Type, fuLong Param1 = 0, fuLong Param2 = 0, fuLong Param3 = 0, fuLong Param4 = 0, f2dInterface* pMemObj = NULL)
	{
		f2dMsg tMsg = { Type, Param1, Param2, Param3, Param4 };
		return SendMsg(tMsg, pMemObj);
	}

	fResult InvokeDelegate(f2dMainThreadDelegate* pDelegate)
	{
		if(!pDelegate || !m_pWindow)
			return FCYERR_INVAILDPARAM;

		pDelegate->AddRef();
		if(TRUE == PostMessage((HWND)m_pWindow->GetHandle(), WM_USER, 0, (LPARAM)pDelegate))
			return FCYERR_OK;
		else
			return FCYERR_INTERNALERR;
	}
	fResult InvokeDelegateAndWait(f2dMainThreadDelegate* pDelegate)
	{
		if(!pDelegate || !m_pWindow)
			return FCYERR_INVAILDPARAM;

		pDelegate->AddRef();
		if(TRUE == SendMessage((HWND)m_pWindow->GetHandle(), WM_USER, 0, (LPARAM)pDelegate))
			return FCYERR_OK;
		else
			return FCYERR_INTERNALERR;
	}

	void GetCPUInfo(f2dCPUInfo& tInfo)
	{
		tInfo.CPUString = m_CPUString.c_str();
		tInfo.CPUBrandString = m_CPUBrandString.c_str();
	}
public:
	f2dEngineImpl(f2dEngineEventListener* pListener);
	f2dEngineImpl(const fcyRect& WinPos, fcStrW Title, fBool Windowed, fBool VSync, F2DAALEVEL AA, f2dEngineEventListener* pListener);
	~f2dEngineImpl();
};
