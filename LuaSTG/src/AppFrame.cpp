#include <string>
#include <fstream>
#include "resource.h"
#include "AppFrame.h"
#include "Utility.h"
#include "ImGuiExtension.h"
#include "LuaWrapper/LuaAppFrame.hpp"
#include "Graphic/Test.h"

using namespace LuaSTGPlus;

////////////////////////////////////////////////////////////////////////////////
/// AppFrame
////////////////////////////////////////////////////////////////////////////////

LNOINLINE AppFrame& AppFrame::GetInstance()
{
	static AppFrame s_Instance;
	return s_Instance;
}

AppFrame::AppFrame() LNOEXCEPT
{
}

AppFrame::~AppFrame() LNOEXCEPT
{
	if (m_iStatus != AppStatus::NotInitialized && m_iStatus != AppStatus::Destroyed)
	{
		// 若没有销毁框架，则执行销毁
		Shutdown();
	}
}

#pragma region 脚本接口

void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改窗口化模式");
}

void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	m_OptionFPSLimit = (v > 1u) ? v : 1u; // 最低也得有1FPS每秒
}

void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改垂直同步模式");
}

void AppFrame::SetResolution(fuInt width, fuInt height)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionResolution.Set((float)width, (float)height);
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改分辨率");
}

void AppFrame::SetTitle(const char* v)LNOEXCEPT
{
	try
	{
		m_OptionTitle = std::move(fcyStringHelper::MultiByteToWideChar(v, CP_UTF8));
		if (m_pMainWindow)
			m_pMainWindow->SetCaption(m_OptionTitle.c_str());
	}
	catch (const std::bad_alloc&)
	{
		LERROR("修改窗口标题时无法分配内存");
	}
}

void AppFrame::SetSplash(bool v)LNOEXCEPT
{
	m_OptionCursor = v;
	if (m_pMainWindow)
		m_pMainWindow->HideMouse(!m_OptionCursor);
}

LNOINLINE bool AppFrame::ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		// 切换到新的视频选项
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)width,
			(fuInt)height,
			windowed,
			false,
			vsync,
			F2DAALEVEL_NONE)))
		{
			LINFO("视频模式切换成功 (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
				width, height, vsync, windowed);

			m_OptionResolution.Set((float)width, (float)height);
			m_OptionWindowed = windowed;
			m_OptionVsync = vsync;

			// 切换窗口大小
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
				);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();
			return true;
		}
		else
		{
			// 改变交换链大小失败后将窗口模式设为true
			m_OptionWindowed = true;

			// 切换窗口大小
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
				);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();

			LINFO("视频模式切换失败 (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
				width, height, vsync, windowed);
		}
	}
	return false;
}

LNOINLINE bool AppFrame::UpdateVideoMode()LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		// 切换到新的视频选项
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)m_OptionResolution.x,
			(fuInt)m_OptionResolution.y,
			m_OptionWindowed,
			m_OptionVsync,
			false,
			F2DAALEVEL_NONE)))
		{
			LINFO("视频模式切换成功 (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
			// 切换窗口大小
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
				);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();
			return true;
		}
		else
		{
			// 改变交换链大小失败后将窗口模式设为true
			m_OptionWindowed = true;
			
			// 切换窗口大小
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
				);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();

			LINFO("视频模式切换失败 (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
		}
	}
	return false;
}

LNOINLINE int AppFrame::LoadTextFile(lua_State* L, const char* path, const char *packname)LNOEXCEPT
{
    if (ResourceMgr::GetResourceLoadingLog()) {
        LINFO("读取文本文件'%m'", path);
    }
	fcyRefPointer<fcyMemStream> tMemStream;
	if (!m_ResourceMgr.LoadFile(path, tMemStream, packname)) {
		LWARNING("无法加载文件'%m'.", path);
		return 0;
	}
	lua_pushlstring(L, (char*)tMemStream->GetInternalBuffer(), (size_t)tMemStream->GetLength());
	tMemStream = nullptr;
	return 1;
}

LNOINLINE void AppFrame::SnapShot(const char* path)LNOEXCEPT
{
	LASSERT(m_pRenderDev);
	
	try
	{
		const std::wstring wpath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		if (FCYFAILED(m_pRenderDev->SaveScreen(wpath.c_str())))
			LERROR("Snapshot: 保存截图到'%s'失败", wpath.c_str());
	}
	catch (const std::bad_alloc&)
	{
		LERROR("Snapshot: 内存不足");
	}
	catch (const fcyException& e)
	{
		LERROR("Snapshot: 保存截图失败 (异常信息'%m' 源'%m')", e.GetDesc(), e.GetSrc());
	}
}

LNOINLINE void AppFrame::SaveTexture(f2dTexture2D* Tex, const char* path)LNOEXCEPT
{
	LASSERT(m_pRenderDev);
	
	try
	{
		const std::wstring wpath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		if (FCYFAILED(m_pRenderDev->SaveTexture(wpath.c_str(), Tex)))
			LERROR("Snapshot: 保存纹理到'%s'失败", wpath.c_str());
	}
	catch (const std::bad_alloc&)
	{
		LERROR("Snapshot: 内存不足");
	}
	catch (const fcyException& e)
	{
		LERROR("Snapshot: 保存纹理失败 (异常信息'%m' 源'%m')", e.GetDesc(), e.GetSrc());
	}
}

#pragma endregion

#pragma region 框架函数

bool AppFrame::Init()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::NotInitialized);
	LINFO("开始初始化 版本: %s", LVERSION);
	m_iStatus = AppStatus::Initializing;
	
	//////////////////////////////////////// Lua初始化部分
	// 开启Lua引擎
	if (!OnOpenLuaEngine())
	{
		return false;
	}
	
	// 加载初始化脚本（可选）
	if (!OnLoadLaunchScriptAndFiles())
	{
		return false;
	}
	
	//////////////////////////////////////// 初始化引擎
	{
		// 为对象池分配空间
		LINFO("初始化对象池 上限=%u", LOBJPOOL_SIZE);
		try
		{
			m_GameObjectPool = std::make_unique<GameObjectPool>(L);
		}
		catch (const std::bad_alloc&)
		{
			LERROR("无法为对象池分配足够内存");
			return false;
		}
		
		// 初始化fancy2d引擎
		LINFO("初始化fancy2d 版本 %d.%d (分辨率: %dx%d 垂直同步: %b 窗口化: %b)",
			(F2DVERSION & 0xFFFF0000) >> 16, F2DVERSION & 0x0000FFFF,
			(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
		struct : public f2dInitialErrListener
		{
			void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
			{
				LERROR("初始化fancy2d失败 (异常信息'%m' 源'%m')", Desc, Src);
			}
		} tErrListener;
		
		if (FCYFAILED(CreateF2DEngineAndInit(
			F2DVERSION,
			fcyRect(0.f, 0.f, m_OptionResolution.x, m_OptionResolution.y),
			m_OptionTitle.c_str(),
			m_OptionWindowed,
			m_OptionVsync,
			F2DAALEVEL_NONE,
			this,
			&m_pEngine,
			&tErrListener
			)))
		{
			return false;
		}
		
		// 获取组件
		m_pMainWindow = m_pEngine->GetMainWindow();
		m_pRenderer = m_pEngine->GetRenderer();
		m_pRenderDev = m_pRenderer->GetDevice();
		m_pSoundSys = m_pEngine->GetSoundSys();
		m_pInputSys = m_pEngine->GetInputSys();
		
		// 打印设备信息
		f2dCPUInfo stCPUInfo = { 0 };
		m_pEngine->GetCPUInfo(stCPUInfo);
		LINFO("CPU %m %m / GPU %m", stCPUInfo.CPUBrandString, stCPUInfo.CPUString, m_pRenderDev->GetDeviceName());
		
		// 创建渲染器
		//原来的大小只有20000，20000
		if (FCYFAILED(m_pRenderDev->CreateGraphics2D(16384, 24576, &m_Graph2D)))
		{
			LERROR("无法创建渲染器 (fcyRenderDevice::CreateGraphics2D failed)");
			return false;
		}
		m_Graph2DLastBlendMode = BlendMode::AddAlpha;
		m_Graph2DBlendState = m_Graph2D->GetBlendState();
		m_Graph2DColorBlendState = m_Graph2D->GetColorBlendType();
		m_bRenderStarted = false;
		
		// 创建文字渲染器
		if (FCYFAILED(m_pRenderer->CreateFontRenderer(nullptr, &m_FontRenderer)))
		{
			LERROR("无法创建字体渲染器 (fcyRenderer::CreateFontRenderer failed)");
			return false;
		}
		m_FontRenderer->SetZ(0.5f);
		
		// 创建图元渲染器
		if (FCYFAILED(m_pRenderer->CreateGeometryRenderer(&m_GRenderer)))
		{
			LERROR("无法创建图元渲染器 (fcyRenderer::CreateGeometryRenderer failed)");
			return false;
		}
		
		// 创建3D渲染器
		if (FCYFAILED(m_pRenderDev->CreateGraphics3D(nullptr, &m_Graph3D)))
		{
			LERROR("无法创建3D渲染器 (fcyRenderDevice::CreateGraphics3D failed)");
			return false;
		}
		m_Graph3DLastBlendMode = BlendMode::AddAlpha;
		m_Graph3DBlendState = m_Graph3D->GetBlendState();
		
		// 默认关闭深度缓冲
		m_pRenderDev->SetZBufferEnable(false);
		m_pRenderDev->ClearZBuffer();
		
		//创建鼠标输入
		m_pInputSys->CreateMouse(-1, false, &m_Mouse);
		if (!m_Mouse)
			LWARNING("无法创建鼠标设备，将使用窗口消息作为输入源 (f2dInputSys::CreateMouse failed.)");
		// 创建键盘输入
		m_pInputSys->CreateKeyboard(-1, false, &m_Keyboard);
		if (!m_Keyboard)
			LWARNING("无法创建键盘设备，将使用窗口消息作为输入源 (f2dInputSys::CreateKeyboard failed.)");
		m_pInputSys->CreateDefaultKeyboard(-1, false, &m_Keyboard2);
		if (!m_Keyboard2)
			LWARNING("无法创建键盘设备，将使用窗口消息作为输入源 (f2dInputSys::CreateDefaultKeyboard failed.)");
		
		//创建手柄输入
		try
		{
			m_DirectInput = std::make_unique<native::DirectInput>((ptrdiff_t)m_pMainWindow->GetHandle());
			{
				m_DirectInput->refresh(); // 这里因为窗口还没显示，所以应该会出现一个Aquire设备失败的错误信息，忽略即可
				uint32_t cnt = m_DirectInput->count();
				for (uint32_t i = 0; i < cnt; i += 1)
				{
					LINFO("侦测到%s手柄 设备名：%s 产品名：%s",
						m_DirectInput->isXInputDevice(i) ? L"XInput" : L"DirectInput",
						m_DirectInput->getDeviceName(i),
						m_DirectInput->getProductName(i));
				}
				LINFO("成功创建了%d个DirectInput手柄", (int)cnt);
			}
		}
		catch (const std::bad_alloc&)
		{
			LERROR("无法创建DirectInput");
		}
		
		// 初始化ImGui
		imgui::bindEngine();
		// test
		slow::Graphic::_bindEngine((void*)m_pMainWindow->GetHandle());
		// 初始化自定义后处理特效
		//slow::effect::bindEngine();
		
		// 设置窗口图标
		HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
		SendMessage((HWND)m_pMainWindow->GetHandle(), WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
		SendMessage((HWND)m_pMainWindow->GetHandle(), WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
		DestroyIcon(hIcon);
		
		// 显示窗口
		m_pMainWindow->SetBorderType(F2DWINBORDERTYPE_FIXED);
		m_pMainWindow->SetClientRect(fcyRect(0.f, 0.f, m_OptionResolution.x, m_OptionResolution.y));
		m_pMainWindow->MoveToCenter();
		m_pMainWindow->SetVisiable(true);
		m_pMainWindow->HideMouse(!m_OptionCursor);
		resetKeyStatus(); // clear key status first
	}
	
	// 装载main脚本
	if (!OnLoadMainScriptAndFiles())
	{
		return false;
	}
	
	//////////////////////////////////////// 初始化完成
	m_iStatus = AppStatus::Initialized;
	LINFO("初始化成功完成");
	
	//////////////////////////////////////// 调用GameInit
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineInit)) {
		return false;
	}
	
	return true;
}

void AppFrame::Shutdown()LNOEXCEPT
{
	SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineStop);
	
	m_GameObjectPool = nullptr;
	LINFO("已清空对象池");

	m_ResourceMgr.ClearAllResource();
	LINFO("已清空所有资源");
	
	// 卸载自定义后处理特效
	//slow::effect::unbindEngine();
	// test
	slow::Graphic::_unbindEngine();
	// 卸载ImGui
	imgui::unbindEngine();
	
	m_DirectInput = nullptr;
	m_Mouse = nullptr;
	m_Keyboard = nullptr;
	m_Keyboard2 = nullptr;
	m_Graph3D = nullptr;
	m_GRenderer = nullptr;
	m_FontRenderer = nullptr;
	m_Graph2D = nullptr;
	m_pInputSys = nullptr;
	m_pSoundSys = nullptr;
	m_pRenderDev = nullptr;
	m_pRenderer = nullptr;
	m_pMainWindow = nullptr;
	m_pEngine = nullptr;
	LINFO("已卸载fancy2d");

	if (L)
	{
		lua_close(L);
		L = nullptr;
		LINFO("已卸载Lua虚拟机");
	}

	m_FileManager.UnloadAllArchive();
	LINFO("已卸载所有资源包");

	m_iStatus = AppStatus::Destroyed;
	LINFO("框架销毁");
}

void AppFrame::Run()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::Initialized);
	LINFO("开始游戏循环");

	m_fFPS = 0.f;
#if (defined LDEVVERSION) || (defined LDEBUG)
	m_UpdateTimer = 0.f;
	m_RenderTimer = 0.f;
	m_PerformanceUpdateTimer = 0.f;
	m_PerformanceUpdateCounter = 0.f;
	m_FPSTotal = 0.f;
	m_ObjectTotal = 0.f;
	m_UpdateTimerTotal = 0.f;
	m_RenderTimerTotal = 0.f;
#endif

	// 启动游戏循环
	m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, m_OptionFPSLimit);

	LINFO("结束游戏循环");
}

#pragma endregion

#pragma region 游戏循环

fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)
{
	#if (defined LDEVVERSION) || (defined LDEBUG)
	TimerScope tProfileScope(m_UpdateTimer);
	#endif
	
	m_fFPS = (float)pFPSController->GetFPS();
	pFPSController->SetLimitedFPS(m_OptionFPSLimit);
	
	m_LastKey = 0;
	
	// 处理消息
	f2dMsg tMsg;
	bool bUpdateDevice = false;
	while (FCYOK(pMsgPump->GetMsg(&tMsg)))
	{
		switch (tMsg.Type)
		{
		case F2DMSG_WINDOW_ONCLOSE:
		{
			return false;  // 关闭窗口时结束循环
		}
		case F2DMSG_WINDOW_ONGETFOCUS:
		{
			resetKeyStatus(); // clear input status
			m_pInputSys->Reset(); // clear input status
			if (m_DirectInput.get())
			{
				m_DirectInput->reset();
			}
			if (m_LastInputTextEnable)
			{
				m_InputTextEnable = true;
			}
			
			lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, true);
			SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);
			
			if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusGainFunc))
				return false;
			break;
		}
		case F2DMSG_WINDOW_ONLOSTFOCUS:
		{
			m_LastInputTextEnable = m_InputTextEnable;
			m_InputTextEnable = false;
			resetKeyStatus(); // clear input status
			m_pInputSys->Reset(); // clear input status
			if (m_DirectInput.get())
			{
				m_DirectInput->reset();
			}
			
			lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowActive);
			lua_pushboolean(L, false);
			SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 2, 0);
			
			if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_FocusLoseFunc))
				return false;
			break;
		}
		case F2DMSG_WINDOW_ONRESIZE:
		{
			lua_pushinteger(L, (lua_Integer)LuaSTG::LuaEngine::EngineEvent::WindowResize);
			lua_pushinteger(L, (lua_Integer)tMsg.Param1);
			lua_pushinteger(L, (lua_Integer)tMsg.Param2);
			SafeCallGlobalFunctionB(LuaSTG::LuaEngine::G_CALLBACK_EngineEvent, 3, 0);
			break;
		}
		case F2DMSG_WINDOW_ONCHARINPUT:
		{
			if (m_InputTextEnable)
			{
				OnTextInputChar((fCharW)tMsg.Param1);
			}
			break;
		}
		case F2DMSG_WINDOW_ONKEYDOWN:
		{
			#ifdef USING_CTRL_ENTER_SWITCH
			// ctrl+enter全屏
			if (tMsg.Param1 == VK_RETURN && !m_KeyStateMap[VK_RETURN] && m_KeyStateMap[VK_CONTROL])  // 防止反复触发
			{
				ChangeVideoMode((int)m_OptionResolution.x, (int)m_OptionResolution.y, !m_OptionWindowed, m_OptionVsync);
			}
			#endif
			// text input
			if (m_InputTextEnable)
			{
				if (tMsg.Param1 == VK_BACK)
				{
					OnTextInputDeleteBack();
				}
				else if (tMsg.Param1 == VK_DELETE)
				{
					OnTextInputDeleteFront();
				}
				else if (tMsg.Param1 == 0x56 && !m_KeyStateMap[0x56] && m_KeyStateMap[VK_CONTROL]) // VK_RETURN + VK_V
				{
					OnTextInputPasting();
				}
			}
			// key
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
			{
				m_LastKey = (fInt)tMsg.Param1;
				m_KeyStateMap[tMsg.Param1] = true;
			}
			break;
		}
		case F2DMSG_WINDOW_ONKEYUP:
		{
			if (m_LastKey == tMsg.Param1)
			{
				m_LastKey = 0;
			}
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
			{
				m_KeyStateMap[tMsg.Param1] = false;
			}
			break;
		}
		case F2DMSG_WINDOW_ONMOUSEMOVE:
		{
			m_MousePosition_old.x = (float)static_cast<fInt>(tMsg.Param1);
			m_MousePosition_old.y = m_OptionResolution.y - (float)static_cast<fInt>(tMsg.Param2);  // ! 潜在大小不匹配问题
			m_MousePosition.x = (float)static_cast<fInt>(tMsg.Param1);
			m_MousePosition.y = (float)static_cast<fInt>(tMsg.Param2);
			break;
		}
		case F2DMSG_SYSTEM_ON_DEVICE_CHANGE:
		{
			bUpdateDevice = true;
			break;
		}
		default:
			break;
		}
	}
	if (bUpdateDevice)
	{
		if (m_DirectInput.get())
		{
			m_DirectInput->refresh();
		}
	}
	
	if (!slow::Graphic::_update())
		return false;
	
	// 执行帧函数
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineUpdate, 1))
		return false;
	bool tAbort = lua_toboolean(L, -1) == 0 ? false : true;
	lua_pop(L, 1);
	
	#if (defined LDEVVERSION) || (defined LDEBUG)
	// 刷新性能计数器
	m_PerformanceUpdateTimer += static_cast<float>(ElapsedTime);
	m_PerformanceUpdateCounter += 1.f;
	m_FPSTotal += static_cast<float>(m_fFPS);
	m_ObjectTotal += (float)m_GameObjectPool->GetObjectCount();
	m_UpdateTimerTotal += m_UpdateTimer;
	m_RenderTimerTotal += m_RenderTimer;
	if (m_PerformanceUpdateTimer > LPERFORMANCEUPDATETIMER)
	{
		//m_FPSTotal / m_PerformanceUpdateCounter,
		//m_ObjectTotal / m_PerformanceUpdateCounter,
		//m_UpdateTimerTotal / m_PerformanceUpdateCounter,
		//m_RenderTimerTotal / m_PerformanceUpdateCounter
		m_PerformanceUpdateTimer = 0.f;
		m_PerformanceUpdateCounter = 0.f;
		m_FPSTotal = 0.f;
		m_ObjectTotal = 0.f;
		m_UpdateTimerTotal = 0.f;
		m_RenderTimerTotal = 0.f;
	}
	#endif
	
	return !tAbort;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)
{
	#if (defined LDEVVERSION) || (defined LDEBUG)
	TimerScope tProfileScope(m_RenderTimer);
	#endif
	
	if (slow::Graphic::_draw())
	{
		return false;
	}
	
	m_pRenderDev->Clear();
	
	// 执行渲染函数
	m_bRenderStarted = true;
	
	if (!SafeCallGlobalFunction(LuaSTG::LuaEngine::G_CALLBACK_EngineDraw))
		m_pEngine->Abort();
	if (!m_stRenderTargetStack.empty())
	{
		LWARNING("OnRender: 渲染结束时没有推出所有的RenderTarget.");
		while (!m_stRenderTargetStack.empty())
			PopRenderTarget();
	}
	m_bRenderStarted = false;
	
	return true;
}

#pragma endregion
