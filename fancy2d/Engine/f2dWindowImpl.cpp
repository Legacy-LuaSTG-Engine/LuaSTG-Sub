#include "Engine/f2dWindowImpl.h"

#include "Engine/f2dEngineImpl.h"

#include <fcyException.h>
#include <fcyOS/fcyDebug.h>
#include <Dbt.h> // DBT_DEVNODES_CHANGED

//#define _IME_DEBUG
//#define _FANCY2D_IME_ENABLE // 妈的，这IME支持还不如不写，一堆bug

using namespace std;

////////////////////////////////////////////////////////////////////////////
// 常量
#define F2DWINDOWSTYLENONEBORDER     ( WS_POPUP )
#define F2DWINDOWSTYLEFIXEDBORDER    ( WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME )
#define F2DWINDOWSTYLESIZEABLEBORDER ( WS_OVERLAPPEDWINDOW )

////////////////////////////////////////////////////////////////////////////////
unordered_map<HWND, f2dWindowImpl*> f2dWindowClass::s_WindowCallBack;

LRESULT CALLBACK f2dWindowClass::WndProc(HWND Handle, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// 提取指针
	f2dWindowImpl* pWindow = s_WindowCallBack[Handle];
	
	// 如果无指针，交给系统处理
	if (pWindow == nullptr)
	{
		return DefWindowProc(Handle,Msg,wParam,lParam);
	}
	
	// 提取监听器指针
	f2dWindowEventListener* pListener = pWindow->GetListener();
	
	// 处理并派送消息
	switch(Msg)
	{
	case WM_USER:
		// 主线程委托
		if(lParam)
		{
			((f2dMainThreadDelegate*)lParam)->Excute();
			((f2dMainThreadDelegate*)lParam)->Release();
		}
		break;
	
	#ifdef _FANCY2D_IME_ENABLE
	case WM_CREATE:
		// 初始化IME上下文
		pWindow->HandleIMELanguageChanged();
		pWindow->InitIMEContext();
		break;
	case WM_DESTROY:
		// 销毁IME上下文
		pWindow->UninitIMEContext();
		break;
	#endif
	
		// 普通回调
	case WM_ACTIVATEAPP:
		if (wParam == TRUE)
		{
			if(pListener) pListener->OnGetFocus();
		}
		else if (wParam == FALSE)
		{
			if(pListener) pListener->OnLostFocus();
		}
		break;
	case WM_CLOSE:
		if(pListener) pListener->OnClose();
		return 0;
	case WM_PAINT:
		if(pListener) pListener->OnPaint();
		break;
	case WM_SIZE:
		{
			fuInt cx = LOWORD(lParam);
			fuInt cy = HIWORD(lParam); 
			if(pListener) pListener->OnSize(cx,cy);
		}
		break;
	case WM_KEYDOWN:
		if(pListener) pListener->OnKeyDown(wParam,lParam);
		break;
	case WM_KEYUP:
		if(pListener) pListener->OnKeyUp(wParam,lParam);
		break;
	case WM_CHAR:
		if(pListener) pListener->OnCharInput((wchar_t)wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		if(pListener) pListener->OnMouseMove(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_MOUSEWHEEL:
		if(pListener) pListener->OnMouseWheel(LOWORD(lParam),HIWORD(lParam),((short)HIWORD(wParam))/(float)WHEEL_DELTA,LOWORD(wParam));
		break;
	case WM_LBUTTONDOWN:
		if(pListener) pListener->OnMouseLBDown(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_LBUTTONUP:
		if(pListener) pListener->OnMouseLBUp(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_LBUTTONDBLCLK:
		if(pListener) pListener->OnMouseLBDouble(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_RBUTTONDOWN:
		if(pListener) pListener->OnMouseRBDown(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_RBUTTONUP:
		if(pListener) pListener->OnMouseRBUp(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_RBUTTONDBLCLK:
		if(pListener) pListener->OnMouseRBDouble(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_MBUTTONDOWN:
		if(pListener) pListener->OnMouseMBDown(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_MBUTTONUP:
		if(pListener) pListener->OnMouseMBUp(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_MBUTTONDBLCLK:
		if(pListener) pListener->OnMouseMBDouble(LOWORD(lParam),HIWORD(lParam),wParam);
		break;
	case WM_SETFOCUS:
		#ifdef _FANCY2D_IME_ENABLE
		pWindow->HandleIMELanguageChanged();
		#endif
		//if(pListener) pListener->OnGetFocus();
		break;
	case WM_KILLFOCUS:
		//if(pListener) pListener->OnLostFocus();
		break;
	case WM_DEVICECHANGE:
		if (wParam == DBT_DEVNODES_CHANGED)
		{
			if(pListener) pListener->OnDeviceChange();
		}
		break;
	
	#ifdef _FANCY2D_IME_ENABLE
	// IME消息部分
	case WM_INPUTLANGCHANGE:
		pWindow->HandleIMELanguageChanged();
		break;
	case WM_IME_STARTCOMPOSITION:
#ifdef _IME_DEBUG
		fcyDebug::Trace(L"[ @ f2dWindowClass::WndProc ] IME start composition.\n");
#endif

		if(pListener) pListener->OnIMEStartComposition();
		break;
	case WM_IME_ENDCOMPOSITION:
#ifdef _IME_DEBUG
		fcyDebug::Trace(L"[ @ f2dWindowClass::WndProc ] IME end composition.\n");
#endif

		if(pListener) pListener->OnIMEEndComposition();
		break;
	case WM_IME_COMPOSITION:
		pWindow->HandleIMEComposition();

		if(pListener) pListener->OnIMEComposition(pWindow->GetIMECompString(), (wchar_t)wParam);
		break;
	case WM_IME_SETCONTEXT:
		// 隐藏输入法的文本框
		if(pWindow->IsHideIME())
		{
			lParam = 0;
			wParam = 0;
		}
		break;
	case WM_IME_NOTIFY:  // 输入法事件
		switch (wParam)
		{
		case IMN_OPENCANDIDATE: // 打开选词表
#ifdef _IME_DEBUG
			fcyDebug::Trace(L"[ @ f2dWindowClass::WndProc ] IME open candidate.\n");
#endif

			pWindow->HandleIMECandidate();

			if(pListener)
			{
				f2dIMECandidateListImpl tList(pWindow);
				pListener->OnIMEOpenCandidate(&tList);
			}
			break;
		case IMN_CLOSECANDIDATE: // 关闭选字表
#ifdef _IME_DEBUG
			fcyDebug::Trace(L"[ @ f2dWindowClass::WndProc ] IME close candidate.\n");
#endif

			pWindow->HandleIMECandidate();

			if(pListener)
			{
				f2dIMECandidateListImpl tList(pWindow);
				pListener->OnIMECloseCandidate(&tList);
			}
			break;

		case IMN_CHANGECANDIDATE:// 选字表翻页
#ifdef _IME_DEBUG
			fcyDebug::Trace(L"[ @ f2dWindowClass::WndProc ] IME candidate list changed.\n");
#endif
			pWindow->HandleIMECandidate();

			if(pListener)
			{
				f2dIMECandidateListImpl tList(pWindow);
				pListener->OnIMEChangeCandidate(&tList);
			}
			break;
		}
		break;
	#endif
	
	default:
		break;
	}
	
	// 提取Win32消息回调
	if (pWindow->HandleNativeMessageCallback(Handle, Msg, wParam, lParam))
		return TRUE;
	
	// 处理消息返回值
	return DefWindowProc(Handle,Msg,wParam,lParam);
}

f2dWindowClass::f2dWindowClass(f2dEngineImpl* pEngine, fcStrW ClassName)
	: m_pEngine(pEngine), m_ClsName(ClassName)
{
	m_WndClass.cbSize = sizeof(WNDCLASSEXW);
	m_WndClass.style = CS_HREDRAW | CS_VREDRAW;
	m_WndClass.lpfnWndProc = WndProc;
	m_WndClass.cbClsExtra = 0;
	m_WndClass.cbWndExtra = 0;
	m_WndClass.hInstance = ::GetModuleHandleW(NULL);
	m_WndClass.hIcon = NULL;
	m_WndClass.hCursor = ::LoadCursorW(NULL, IDC_ARROW);
	m_WndClass.hbrBackground = NULL;
	m_WndClass.lpszMenuName = NULL;
	m_WndClass.lpszClassName = m_ClsName.c_str();
	m_WndClass.hIconSm = NULL;
	
	m_Atom = ::RegisterClassExW(&m_WndClass);
	if(m_Atom == 0)
		throw fcyWin32Exception("f2dWindowClass::f2dWindowClass", "RegisterClass Failed.");
}

f2dWindowClass::~f2dWindowClass()
{
	if (m_Atom != 0)
		UnregisterClassW(m_WndClass.lpszClassName, m_WndClass.hInstance);
}

fcStrW f2dWindowClass::GetName()const
{
	return m_ClsName.c_str();
}

f2dWindowImpl* f2dWindowClass::CreateRenderWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border, bool DisableIME)
{
	return new f2dWindowImpl(m_pEngine, this, Pos, Title, Visiable, Border, DisableIME);
}

////////////////////////////////////////////////////////////////////////////////

f2dWindowDC::f2dWindowDC()
{
	m_pBits = NULL;
    m_hBmp  = NULL;
	m_hDC	= NULL;
	m_Width = 0;
	m_Height= 0;
}

f2dWindowDC::~f2dWindowDC()
{
	Delete();
}

void f2dWindowDC::Delete()
{
	if (m_hBmp)
		DeleteObject(m_hBmp);
	if (m_hDC)
		DeleteDC(m_hDC);
}

void f2dWindowDC::Create(int nWidth, int nHeight)
{
	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(BITMAPINFOHEADER));
	bih.biSize	      =	sizeof(BITMAPINFOHEADER);
	bih.biWidth       =	nWidth;
	bih.biHeight      = -nHeight;
	bih.biPlanes      = 1;
	bih.biBitCount    = 32;
	bih.biCompression = BI_RGB;
	m_hBmp = NULL;
	m_hDC  = CreateCompatibleDC(NULL);
	m_hBmp = ::CreateDIBSection(
		GetSafeHdc(), (BITMAPINFO*)&bih,
		DIB_RGB_COLORS, (void**)(&m_pBits), NULL, 0);
	SelectObject(m_hDC, m_hBmp);

	m_Width = nWidth;
	m_Height = nHeight;
}

////////////////////////////////////////////////////////////////////////////////
// 候选词
f2dIMECandidateListImpl::f2dIMECandidateListImpl(f2dWindowImpl* pWindow)
{
	m_IMETotalCandidate = pWindow->m_IMETotalCandidate;
	m_IMESelectedCandidate = pWindow->m_IMESelectedCandidate;
	m_IMEPageStartCandidate = pWindow->m_IMEPageStartCandidate;
	m_IMEPageCandidateCount = pWindow->m_IMEPageCandidateCount;
	m_IMECandidateList = pWindow->m_IMECandidateList;
}

f2dIMECandidateListImpl::f2dIMECandidateListImpl(const f2dIMECandidateListImpl& Right)
{
	m_IMETotalCandidate = Right.m_IMETotalCandidate;
	m_IMESelectedCandidate = Right.m_IMESelectedCandidate;
	m_IMEPageStartCandidate = Right.m_IMEPageStartCandidate;
	m_IMEPageCandidateCount = Right.m_IMEPageCandidateCount;
	m_IMECandidateList = Right.m_IMECandidateList;
}

f2dIMECandidateListImpl::~f2dIMECandidateListImpl()
{
}

fuInt f2dIMECandidateListImpl::GetCount()
{
	return m_IMETotalCandidate;
}

fuInt f2dIMECandidateListImpl::GetCurIndex()
{
	return m_IMESelectedCandidate;
}

fuInt f2dIMECandidateListImpl::GetPageSize()
{
	return m_IMEPageCandidateCount;
}

fuInt f2dIMECandidateListImpl::GetPageStart()
{
	return m_IMEPageStartCandidate;
}

fcStrW f2dIMECandidateListImpl::GetCandidateStr(fuInt Index)
{
	if(Index > GetCount())
		return NULL;
	else
		return m_IMECandidateList[Index].c_str();
}

////////////////////////////////////////////////////////////////////////////////
// 监听器
void f2dWindowImpl::DefaultListener::OnClose() 
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONCLOSE);
}
void f2dWindowImpl::DefaultListener::OnPaint()
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONPAINT);
}
void f2dWindowImpl::DefaultListener::OnSize(fuInt ClientWidth, fuInt ClientHeight)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONRESIZE, ClientWidth, ClientHeight);
}
void f2dWindowImpl::DefaultListener::OnKeyDown(fuInt KeyCode, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONKEYDOWN, KeyCode);
}
void f2dWindowImpl::DefaultListener::OnKeyUp(fuInt KeyCode, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONKEYUP, KeyCode);
}
void f2dWindowImpl::DefaultListener::OnCharInput(fCharW CharCode, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONCHARINPUT, CharCode);
}
void f2dWindowImpl::DefaultListener::OnIMEStartComposition()
{
	m_pEngine->SendMsg(F2DMSG_IME_ONSTARTCOMPOSITION);
}
void f2dWindowImpl::DefaultListener::OnIMEEndComposition()
{
	m_pEngine->SendMsg(F2DMSG_IME_ONENDCOMPOSITION);
}
void f2dWindowImpl::DefaultListener::OnIMEComposition(fcStrW String, fCharW CharCode)
{
	// 封装字符串并发送消息
	f2dMsgMemHelper<std::wstring>* tObjMem = new f2dMsgMemHelper<std::wstring>(String);
	m_pEngine->SendMsg(
		F2DMSG_IME_ONCOMPOSITION,
		(fuLong)tObjMem->GetObj().c_str(), 
		CharCode,
		0,
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}
void f2dWindowImpl::DefaultListener::OnIMEActivated(fcStrW Desc)
{
	// 封装字符串并发送消息
	f2dMsgMemHelper<std::wstring>* tObjMem = new f2dMsgMemHelper<std::wstring>(Desc);
	m_pEngine->SendMsg(
		F2DMSG_IME_ONACTIVATE,
		(fuLong)tObjMem->GetObj().c_str(), 
		0,
		0,
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}
void f2dWindowImpl::DefaultListener::OnIMEClosed()
{
	m_pEngine->SendMsg(F2DMSG_IME_ONCLOSE);
}
void f2dWindowImpl::DefaultListener::OnIMEChangeCandidate(f2dIMECandidateList* pList)
{
	// 封装并发送消息
	f2dMsgMemHelper<f2dIMECandidateListImpl>* tObjMem = new f2dMsgMemHelper<f2dIMECandidateListImpl>(*(f2dIMECandidateListImpl*)pList);
	m_pEngine->SendMsg(
		F2DMSG_IME_ONCHANGECANDIDATE,
		(fuLong)&tObjMem->GetObj(), 
		0,
		0,
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}
void f2dWindowImpl::DefaultListener::OnIMEOpenCandidate(f2dIMECandidateList* pList)
{
	// 封装并发送消息
	f2dMsgMemHelper<f2dIMECandidateListImpl>* tObjMem = new f2dMsgMemHelper<f2dIMECandidateListImpl>(*(f2dIMECandidateListImpl*)pList);
	m_pEngine->SendMsg(
		F2DMSG_IME_ONOPENCANDIDATE,
		(fuLong)&tObjMem->GetObj(), 
		0,
		0,
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}
void f2dWindowImpl::DefaultListener::OnIMECloseCandidate(f2dIMECandidateList* pList)
{
	// 封装并发送消息
	f2dMsgMemHelper<f2dIMECandidateListImpl>* tObjMem = new f2dMsgMemHelper<f2dIMECandidateListImpl>(*(f2dIMECandidateListImpl*)pList);
	m_pEngine->SendMsg(
		F2DMSG_IME_ONCLOSECANDIDATE,
		(fuLong)&tObjMem->GetObj(), 
		0,
		0,
		0,
		tObjMem
		);
	FCYSAFEKILL(tObjMem);
}
void f2dWindowImpl::DefaultListener::OnMouseMove(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEMOVE, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseWheel(fShort X, fShort Y, fFloat Wheel, fuInt Flag)
{
	fDouble tValue = Wheel;
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEWHEEL, X, Y, *(fuLong*)&tValue);
}
void f2dWindowImpl::DefaultListener::OnMouseLBDown(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSELDOWN, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseLBUp(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSELUP, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseLBDouble(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSELDOUBLE, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseMBDown(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEMDOWN, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseMBUp(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEMUP, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseMBDouble(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEMDOUBLE, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseRBDown(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSERDOWN, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseRBUp(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSERUP, X, Y);
}
void f2dWindowImpl::DefaultListener::OnMouseRBDouble(fShort X, fShort Y, fuInt Flag)
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSERDOUBLE, X, Y);
}
void f2dWindowImpl::DefaultListener::OnGetFocus()
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONGETFOCUS);
}
void f2dWindowImpl::DefaultListener::OnLostFocus()
{
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONLOSTFOCUS);
}
void f2dWindowImpl::DefaultListener::OnDeviceChange()
{
	m_pEngine->SendMsg(F2DMSG_SYSTEM_ON_DEVICE_CHANGE);
}

////////////////////////////////////////////////////////////////////////////////

f2dWindowImpl::f2dWindowImpl(f2dEngineImpl* pEngine, f2dWindowClass* WinCls, const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border, bool DisableIME)
	: m_DefaultListener(pEngine, this), m_pListener(&m_DefaultListener), m_hWnd(NULL), m_bShow(false), m_CaptionText(Title),
	m_bHideIME(true), m_hIMC(NULL), m_IMETotalCandidate(0), m_IMESelectedCandidate(0), m_IMEPageStartCandidate(0), m_IMEPageCandidateCount(0)
{
	// 定义窗口样式
	fuInt tWinStyle;
	switch(Border)
	{
	case F2DWINBORDERTYPE_NONE:
		tWinStyle = F2DWINDOWSTYLENONEBORDER;
		break;
	case F2DWINBORDERTYPE_FIXED:
		tWinStyle = F2DWINDOWSTYLEFIXEDBORDER;
		break;
	case F2DWINBORDERTYPE_SIZEABLE:
		tWinStyle = F2DWINDOWSTYLESIZEABLEBORDER;
		break;
	default:
		throw fcyException("f2dWindowImpl::f2dWindowImpl", "Invalid F2DWINBORDERTYPE.");
	}

	// 计算窗口大小
	RECT tWinRect = { (int)Pos.a.x , (int)Pos.a.y , (int)Pos.b.x , (int)Pos.b.y};
	AdjustWindowRectEx(&tWinRect, tWinStyle, FALSE, 0);
	fuInt tRealWidth = tWinRect.right  - tWinRect.left ;
	fuInt tRealHeight = tWinRect.bottom  - tWinRect.top;
	
	if (DisableIME)
	{
		ImmDisableIME(0);//不需要的时候屏蔽输入法
	}
	
	// 创建窗口
	m_hWnd = CreateWindowExW(
		0,
		WinCls->GetName(),
		m_CaptionText.c_str(),
		tWinStyle,
		tWinRect.left,
		tWinRect.top,
		tRealWidth,
		tRealHeight,
		NULL,
		NULL,
		GetModuleHandleW(NULL),
		NULL
		);
	
	if(!m_hWnd)
		throw fcyWin32Exception("f2dWindowImpl::f2dWindowImpl", "CreateWindowEx Failed.");

	// 注册窗口
	f2dWindowClass::s_WindowCallBack[m_hWnd] = this;

	// 显示窗口
	if(m_bShow)
	{
		SetVisiable(true);
	}
}

f2dWindowImpl::~f2dWindowImpl()
{
	// 销毁窗口
	DestroyWindow(m_hWnd);

	// 取消注册
	unordered_map<HWND, f2dWindowImpl*>::iterator i = f2dWindowClass::s_WindowCallBack.find(m_hWnd);
	if(i != f2dWindowClass::s_WindowCallBack.end())
		f2dWindowClass::s_WindowCallBack.erase(i);
}

void f2dWindowImpl::InitIMEContext()
{
	if (!m_bHideIME) {
		m_hIMC = ImmCreateContext();
		m_hIMC = ImmAssociateContext(m_hWnd, m_hIMC);
	}
	else {
		m_hIMC = ImmAssociateContext(m_hWnd, NULL);
	}
}

void f2dWindowImpl::UninitIMEContext()
{
	if (!m_bHideIME) {
		m_hIMC = ImmAssociateContext(m_hWnd, m_hIMC);
		ImmDestroyContext(m_hIMC);
	}
}

void f2dWindowImpl::HandleIMELanguageChanged()
{
	HKL hKL = ::GetKeyboardLayout(0);
	int iSize = ::ImmGetDescription(hKL, NULL, 0);
	if (iSize == 0)
	{
		// 输入法关闭
		m_CurIMEDesc.clear();

		if (m_pListener)
			m_pListener->OnIMEClosed();

#ifdef _IME_DEBUG
		fcyDebug::Trace(L"[ @ f2dWindowImpl::HandleIMELanguageChanged ] IME closed.\n");
#endif
	}
	else
	{
		// 输入法切换
		m_CurIMEDesc.clear();
		m_CurIMEDesc.resize(iSize);
		ImmGetDescription(hKL, &m_CurIMEDesc[0], iSize);

		if (m_pListener)
			m_pListener->OnIMEActivated(GetIMEDesc());

#ifdef _IME_DEBUG
		fcyDebug::Trace(L"[ @ f2dWindowImpl::HandleIMELanguageChanged ] IME changed as : %s.\n", m_CurIMEDesc.c_str());
#endif
	}
}

void f2dWindowImpl::HandleIMEComposition()
{
	if (!m_bHideIME) {
		HIMC hIMC = ImmGetContext(m_hWnd);
		LONG lSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, 0, 0);

		if (lSize == 0)
			m_CurIMEComposition.clear();
		else
		{
			m_CurIMEComposition.clear();
			m_CurIMEComposition.resize(lSize);
			ImmGetCompositionString(hIMC, GCS_COMPSTR, &m_CurIMEComposition[0], lSize);

#ifdef _IME_DEBUG
			fcyDebug::Trace(L"[ @ f2dWindowImpl::HandleIMEComposition ] Current composition : %s.\n", m_CurIMEComposition.c_str());
#endif

			ImmReleaseContext(m_hWnd, hIMC);
		}
	}
	else {
		m_CurIMEComposition.clear();
	}
}

void f2dWindowImpl::HandleIMECandidate()
{
	if (!m_bHideIME) {
		// 获得上下文
		HIMC hIMC = ImmGetContext(m_hWnd);

		// 获得候选词列表大小
		LONG dwSize = ImmGetCandidateList(hIMC, 0, NULL, 0);

		if (dwSize == 0)
		{
			m_IMEPageCandidateCount = m_IMEPageStartCandidate = m_IMETotalCandidate = m_IMESelectedCandidate = 0;
			m_IMECandidateList.clear();

			return;
		}

		// 申请全局空间来存放候选词
		LPCANDIDATELIST pList = (LPCANDIDATELIST)GlobalAlloc(GPTR, dwSize);

		// 获得候选词列表
		if (pList)
			ImmGetCandidateList(hIMC, 0, pList, dwSize);
		else
			return;  // 内存分配失败

		m_IMEPageCandidateCount = pList->dwPageSize;
		m_IMEPageStartCandidate = pList->dwPageStart;
		m_IMETotalCandidate = pList->dwCount;
		m_IMESelectedCandidate = pList->dwSelection;

		// 获得候选词列表
		m_IMECandidateList.clear();
		m_IMECandidateList.reserve(pList->dwCount);
		for (fuInt i = 0; i < pList->dwCount; ++i)
		{
			fcStrW pStr = (fcStrW)((size_t)pList + (size_t)pList->dwOffset[i]);

			m_IMECandidateList.push_back(pStr);
		}

#ifdef _IME_DEBUG
		wstring tDebugStr;
		for (fuInt i = 0; i < pList->dwCount; ++i)
		{
			tDebugStr += L"\t" + m_IMECandidateList[i] + L"\n";
		}

		fcyDebug::Trace(L"[ @ f2dWindowImpl::HandleIMECandidate ] Candidate info.\n\tCandidate: %d/%d PageStart: %d PageSize: %d\nList :\n%s\n",
			m_IMESelectedCandidate, m_IMETotalCandidate, m_IMEPageStartCandidate, m_IMEPageCandidateCount,
			tDebugStr.c_str());
#endif

		// 擦屁股
		GlobalFree(pList);
		ImmReleaseContext(m_hWnd, hIMC);
	}
	else {
		m_IMEPageCandidateCount = m_IMEPageStartCandidate = m_IMETotalCandidate = m_IMESelectedCandidate = 0u;
		m_IMECandidateList.clear();
	}
}

f2dWindowEventListener* f2dWindowImpl::GetListener()
{
	return m_pListener;
}

fResult f2dWindowImpl::SetListener(f2dWindowEventListener* pListener)
{
	m_pListener = pListener;
	return FCYERR_OK;
}

void f2dWindowImpl::AddNativeMessageCallback(ptrdiff_t pWndProc)
{
	RemoveNativeMessageCallback(pWndProc);
	m_fProc.push_back(pWndProc);
}

void f2dWindowImpl::RemoveNativeMessageCallback(ptrdiff_t pWndProc)
{
	for (auto it = m_fProc.begin(); it != m_fProc.end();)
	{
		if (*it == pWndProc)
		{
			it = m_fProc.erase(it);
		}
		else
		{
			it++;
		}
	}
}

typedef LRESULT (*NATIVE_WNDPROC)(HWND, UINT, WPARAM, LPARAM);
bool f2dWindowImpl::HandleNativeMessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool should_skip = false;
	for (auto f : m_fProc)
	{
		if (0 != ((NATIVE_WNDPROC)(f))(hWnd, msg, wParam, lParam))
		{
			should_skip = true;
		}
	}
	return should_skip;
}

fInt f2dWindowImpl::GetHandle()
{
	return (fInt)m_hWnd;
}

F2DWINBORDERTYPE f2dWindowImpl::GetBorderType()
{
	fuInt tStyle = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	switch(tStyle)
	{
	case F2DWINDOWSTYLENONEBORDER:
		return F2DWINBORDERTYPE_NONE;
	case F2DWINDOWSTYLEFIXEDBORDER:
		return F2DWINBORDERTYPE_FIXED;
	case F2DWINDOWSTYLESIZEABLEBORDER:
		return F2DWINBORDERTYPE_SIZEABLE;
	default:
		return F2DWINBORDERTYPE_NONE;
	}
}

fResult f2dWindowImpl::SetBorderType(F2DWINBORDERTYPE Type)
{
	switch(Type)
	{
	case F2DWINBORDERTYPE_NONE:
		SetWindowLongPtr(m_hWnd, GWL_STYLE, F2DWINDOWSTYLENONEBORDER);
		break;
	case F2DWINBORDERTYPE_FIXED:
		SetWindowLongPtr(m_hWnd, GWL_STYLE, F2DWINDOWSTYLEFIXEDBORDER);
		break;
	case F2DWINBORDERTYPE_SIZEABLE:
		SetWindowLongPtr(m_hWnd, GWL_STYLE, F2DWINDOWSTYLESIZEABLEBORDER);
		break;
	default:
		return FCYERR_ILLEGAL;
	}

	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
		m_bShow
			? SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE
			: SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
	return FCYERR_OK;
}

fcStrW f2dWindowImpl::GetCaption()
{
	wchar_t tTemp[1024];
	::GetWindowTextW(m_hWnd, tTemp, 1024);
	m_CaptionText = tTemp;
	return m_CaptionText.c_str();
}

fResult f2dWindowImpl::SetCaption(fcStrW Caption)
{
	m_CaptionText = Caption;
	return ::SetWindowTextW(m_hWnd, Caption)==TRUE?FCYERR_OK : FCYERR_INTERNALERR;
}

fBool f2dWindowImpl::GetVisiable()
{
	return m_bShow;
}

fResult f2dWindowImpl::SetVisiable(fBool Visiable)
{
	m_bShow = Visiable;
	return ShowWindow(m_hWnd, m_bShow?SW_SHOW:SW_HIDE)==TRUE?FCYERR_OK : FCYERR_INTERNALERR;
}

fcyRect f2dWindowImpl::GetRect()
{
	RECT tRect;
	::GetWindowRect(m_hWnd, &tRect);
	fcyRect tRet((float)tRect.left, (float)tRect.top, (float)tRect.right, (float)tRect.bottom);
	return tRet;
}

fResult f2dWindowImpl::SetRect(const fcyRect& Range)
{
	return ::SetWindowPos(m_hWnd, 0,
		(int)Range.a.x, (int)Range.a.y,
		(int)Range.GetWidth(), (int)Range.GetHeight(),
		SWP_NOZORDER) != FALSE ? FCYERR_OK : FCYERR_INTERNALERR;
}

fcyRect f2dWindowImpl::GetClientRect()
{
	RECT tRect;
	::GetClientRect(m_hWnd, &tRect);
	fcyRect tRet((float)tRect.left, (float)tRect.top, (float)tRect.right, (float)tRect.bottom);
	return tRet;
}

fResult f2dWindowImpl::SetClientRect(const fcyRect& Range)
{
	RECT tWinRect = { (int)Range.a.x , (int)Range.a.y , (int)Range.b.x , (int)Range.b.y};
	AdjustWindowRectEx(&tWinRect, GetWindowLongPtr(m_hWnd, GWL_STYLE), FALSE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE));
	return SetWindowPos(m_hWnd, 0,
		tWinRect.left,
		tWinRect.top,
		tWinRect.right - tWinRect.left,
		tWinRect.bottom - tWinRect.top,
		SWP_NOZORDER) == TRUE ? FCYERR_OK : FCYERR_INTERNALERR;
}

void f2dWindowImpl::MoveToCenter()
{
	// 获得关联的显示器
	HMONITOR monitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if (monitor)
	{
		// 获得显示器信息
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			// 获得窗口位置
			RECT rect = {};
			if (FALSE != ::GetWindowRect(m_hWnd, &rect))
			{
				// 计算值
				const auto& area = moninfo.rcMonitor;
				const auto sx = area.left;
				const auto sy = area.top;
				const auto sw = area.right - area.left;
				const auto sh = area.bottom - area.top;
				const auto ww = rect.right - rect.left;
				const auto wh = rect.bottom - rect.top;
				// 修改窗口位置
				::SetWindowPos(m_hWnd, NULL, sx + (sw / 2) - (ww / 2), sy + (sh / 2) - (wh / 2), ww, wh, SWP_NOZORDER | SWP_SHOWWINDOW);
			}
		}
	}
}

fBool f2dWindowImpl::IsTopMost()
{
	if(WS_EX_TOPMOST & GetWindowLongPtr(m_hWnd, GWL_EXSTYLE))
		return true;
	else
		return false;
}

fResult f2dWindowImpl::SetTopMost(fBool TopMost)
{
	if(SetWindowPos(m_hWnd, TopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW) != FALSE)
		return FCYERR_OK;
	else
		return FCYERR_INTERNALERR;
}

void f2dWindowImpl::HideMouse(fBool bHide)
{
	ShowCursor((BOOL)!bHide);
}
