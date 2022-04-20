#include "Engine/f2dWindowCommonMessage.h"
#include "Engine/f2dWindowImpl.h"
#include "Engine/f2dEngineImpl.h"
#include "Engine/f2dMonitorHelper.h"

#include <fcyException.h>
#include <fcyOS/fcyDebug.h>
#include <Dbt.h> // DBT_DEVNODES_CHANGED
#include <windowsx.h>
#include "platform/HighDPI.hpp"

//#define _IME_DEBUG
//#define _FANCY2D_IME_ENABLE // 妈的，这IME支持还不如不写，一堆bug

////////////////////////////////////////////////////////////////////////////
// 常量
#define F2DWINDOWSTYLENONEBORDER     ( WS_POPUP )
#define F2DWINDOWSTYLEFIXEDBORDER    ( WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME )
#define F2DWINDOWSTYLESIZEABLEBORDER ( WS_OVERLAPPEDWINDOW )

////////////////////////////////////////////////////////////////////////////////

std::unordered_map<HWND, f2dWindowImpl*> f2dWindowClass::s_WindowCallBack;

LRESULT CALLBACK f2dWindowClass::WndProc(HWND Handle, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// 提取指针
	f2dWindowImpl* pWindow = s_WindowCallBack[Handle];
	
	// 如果无指针，交给系统处理
	if (pWindow == nullptr)
	{
		return DefWindowProcW(Handle,Msg,wParam,lParam);
	}
	
	// 提取监听器指针
	f2dWindowEventListener* pListener = pWindow->GetListener();
	
	// 处理并派送消息
	switch(Msg)
	{
	// 由应用程序发送的消息 
	
	case WM_MAIN_THREAD_DELEGATE:
		{
			// 主线程委托
			if(lParam)
			{
				auto self = (f2dMainThreadDelegate*)lParam;
				self->Excute();
				self->Release();
			}
		}
		break;
	case WM_SET_IME_ENABLE:
		{
			if ((BOOL)lParam)
			{
				::ImmAssociateContext(Handle, (HIMC)wParam);
			}
			else
			{
				::ImmAssociateContext(Handle, NULL);
			}
		}
		break;
	
	// 由系统发送的消息
	
	case WM_ACTIVATEAPP:
		if (wParam == TRUE)
		{
			if (pWindow->m_pGraphicListener) pWindow->m_pGraphicListener->OnGetFocus();
			if (pListener) pListener->OnGetFocus();
		}
		else if (wParam == FALSE)
		{
			if (pWindow->m_pGraphicListener) pWindow->m_pGraphicListener->OnLostFocus();
			if (pListener) pListener->OnLostFocus();
		}
		break;
	case WM_CREATE:
		{
			platform::HighDPI::EnableNonClientDpiScaling(Handle);
			#ifdef _FANCY2D_IME_ENABLE
			// 初始化IME上下文
			pWindow->HandleIMELanguageChanged();
			pWindow->InitIMEContext();
			#endif
		}
		break;
	case WM_DESTROY:
		{
			#ifdef _FANCY2D_IME_ENABLE
			// 销毁IME上下文
			pWindow->UninitIMEContext();
			#endif
		}
		break;
	case WM_CLOSE:
		if(pListener) pListener->OnClose();
		break;
	case WM_PAINT:
		if(pListener) pListener->OnPaint();
		break;
	case WM_SIZE:
		{
			fuInt cx = LOWORD(lParam);
			fuInt cy = HIWORD(lParam); 
			if (pWindow->m_pGraphicListener) pWindow->m_pGraphicListener->OnSize(cx, cy);
			if(pListener) pListener->OnSize(cx,cy);
		}
		break;
	
	// 键盘消息
	
	case WM_KEYDOWN:
		if(pListener) pListener->OnKeyDown(wParam,lParam);
		break;
	case WM_KEYUP:
		if(pListener) pListener->OnKeyUp(wParam,lParam);
		break;
	case WM_CHAR:
		if(pListener) pListener->OnCharInput((wchar_t)wParam, lParam);
		break;
	
	// 鼠标消息
	
	case WM_MOUSEMOVE:
		if(pListener) pListener->OnMouseMove(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_MOUSEWHEEL:
		if(pListener) pListener->OnMouseWheel(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),(fFloat)GET_WHEEL_DELTA_WPARAM(wParam),LOWORD(wParam));
		break;
	case WM_LBUTTONDOWN:
		if(pListener) pListener->OnMouseLBDown(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_LBUTTONUP:
		if(pListener) pListener->OnMouseLBUp(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_LBUTTONDBLCLK:
		if(pListener) pListener->OnMouseLBDouble(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_RBUTTONDOWN:
		if(pListener) pListener->OnMouseRBDown(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_RBUTTONUP:
		if(pListener) pListener->OnMouseRBUp(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_RBUTTONDBLCLK:
		if(pListener) pListener->OnMouseRBDouble(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_MBUTTONDOWN:
		if(pListener) pListener->OnMouseMBDown(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_MBUTTONUP:
		if(pListener) pListener->OnMouseMBUp(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	case WM_MBUTTONDBLCLK:
		if(pListener) pListener->OnMouseMBDouble(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),wParam);
		break;
	
	// 其他
	
	case WM_SETFOCUS:
		{
			#ifdef _FANCY2D_IME_ENABLE
			pWindow->HandleIMELanguageChanged();
			#endif
		}
		break;
	case WM_KILLFOCUS:
		break;
	case WM_DEVICECHANGE:
		if (wParam == DBT_DEVNODES_CHANGED)
		{
			if(pListener) pListener->OnDeviceChange();
		}
		break;
	case WM_DPICHANGED:
		if (pWindow->m_bAutoResizeWindowOnDPIScaling)
		{
			pWindow->SetClientRect(pWindow->GetClientRect()); // 刷新一次尺寸（因为非客户区可能会变化）
			pWindow->MoveToCenter();
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
	
	// 特殊处理
	auto rResult = pWindow->m_SizeMoveCtrl.handleSizeMove(Handle, Msg, wParam, lParam);
	if (rResult.bReturn)
		return rResult.lResult;
	
	// 某些消息不该继续传递
	switch (Msg)
	{
	case WM_CLOSE:
		return 0;
	//case WM_GETMINMAXINFO:
	//	{
	//		MINMAXINFO* info = (MINMAXINFO*)lParam;
	//		info->ptMaxSize.x = (LONG)pWindow->m_Size.x + 2;
	//		info->ptMaxSize.y = (LONG)pWindow->m_Size.y + 2;
	//		info->ptMaxTrackSize.x = (LONG)pWindow->m_Size.x + 2;
	//		info->ptMaxTrackSize.y = (LONG)pWindow->m_Size.y + 2;
	//	}
	//	return 0;
	case WM_DPICHANGED:
		if (pWindow->m_bAutoResizeWindowOnDPIScaling)
			return 0;
		else
			break;
	default:
		break;
	};
	
	// 处理消息返回值
	return DefWindowProcW(Handle, Msg, wParam, lParam);
}

f2dWindowClass::f2dWindowClass(f2dEngineImpl* pEngine, fcStrW ClassName)
	: m_pEngine(pEngine), m_ClsName(ClassName)
{
	// platform::HighDPI::enable(); // 交给 manifest 文件
	
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
	// 四字节UTF16前二字节，先保存下来
	if (CharCode >= 0xD800 && CharCode <= 0xDBFF)
	{
		m_U16Lead = CharCode; 
	}
	// 四字节UTF16后二字节，一起发送
	else if (CharCode >= 0xDC00 && CharCode <= 0xDFFF)
	{
		// 前一个代理对存在才一起发送
		if (m_U16Lead > 0)
		{
			m_pEngine->SendMsg(F2DMSG_WINDOW_ONCHARINPUT, m_U16Lead);
			m_pEngine->SendMsg(F2DMSG_WINDOW_ONCHARINPUT, CharCode);
		}
		m_U16Lead = 0; // 总之还是要清理一下
	}
	// 双字节UTF16
	else
	{
		m_pEngine->SendMsg(F2DMSG_WINDOW_ONCHARINPUT, CharCode);
	}
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
	fuLong data = Wheel > 0.0f ? (fuLong)Wheel : (fuLong)-Wheel;
	fuLong flag = Wheel > 0.0f ? (1ull << 63) : 0;
	flag |= WHEEL_DELTA;
	m_pEngine->SendMsg(F2DMSG_WINDOW_ONMOUSEWHEEL, X, Y, data, Wheel > 0.0f ? 1 : 0);
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
	: m_DefaultListener(pEngine, this), m_pListener(&m_DefaultListener), m_pGraphicListener(nullptr), m_hWnd(NULL), m_bShow(false), m_CaptionText(Title),
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
	
	// 创建窗口
	m_hWnd = ::CreateWindowExW(
		0, WinCls->GetName(), m_CaptionText.c_str(), tWinStyle,
		0, 0, 640, 480,
		NULL, NULL, ::GetModuleHandleW(NULL), this);
	if(!m_hWnd)
		throw fcyWin32Exception("f2dWindowImpl::f2dWindowImpl", "CreateWindowEx Failed.");
	
	// 注册窗口
	f2dWindowClass::s_WindowCallBack[m_hWnd] = this;
	
	// 调整窗口大小
	RECT tWinRect = { (LONG)Pos.a.x , (LONG)Pos.a.y , (LONG)Pos.b.x , (LONG)Pos.b.y};
	platform::HighDPI::AdjustWindowRectExForDpi(&tWinRect, tWinStyle, FALSE, 0, platform::HighDPI::GetDpiForWindow(m_hWnd));
	::SetWindowPos(
		m_hWnd, NULL,
		tWinRect.left, tWinRect.top, tWinRect.right - tWinRect.left, tWinRect.bottom - tWinRect.top,
		SWP_NOZORDER);
	m_Size.x = (float)(tWinRect.right - tWinRect.left);
	m_Size.y = (float)(tWinRect.bottom - tWinRect.top);
	
	// 获取默认输入法上下文
	_defaultIMC = NULL;
	_enableIME = false;
	_defaultIMC = ::ImmAssociateContext(m_hWnd, NULL);
	if (!DisableIME)
	{
		::ImmAssociateContext(m_hWnd, _defaultIMC);
		_enableIME = true;
	}
	
	// 配置窗口挪动控制器
	m_SizeMoveCtrl.setWindow(m_hWnd);
	
	// 显示窗口
	if(m_bShow)
	{
		SetVisiable(true);
	}
}

f2dWindowImpl::~f2dWindowImpl()
{
	// 取消注册
	auto it = f2dWindowClass::s_WindowCallBack.find(m_hWnd);
	if(it != f2dWindowClass::s_WindowCallBack.end())
	{
		f2dWindowClass::s_WindowCallBack.erase(it);
	}
	
	// 关闭移动控制
	m_SizeMoveCtrl.setWindow(NULL);
	
	// 销毁窗口
	DestroyWindow(m_hWnd);
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
f2dWindowEventListener* f2dWindowImpl::GetGraphicListener()
{
	return m_pGraphicListener;
}
fResult f2dWindowImpl::SetGraphicListener(f2dWindowEventListener* pListener)
{
	m_pGraphicListener = pListener;
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

fHandle f2dWindowImpl::GetHandle()
{
	return (fHandle)m_hWnd;
}

F2DWINBORDERTYPE f2dWindowImpl::GetBorderType()
{
	auto tStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	if ((tStyle & F2DWINDOWSTYLESIZEABLEBORDER) == F2DWINDOWSTYLESIZEABLEBORDER)
		return F2DWINBORDERTYPE_SIZEABLE;
	if ((tStyle & F2DWINDOWSTYLEFIXEDBORDER) == F2DWINDOWSTYLEFIXEDBORDER)
		return F2DWINBORDERTYPE_FIXED;
	if ((tStyle & F2DWINDOWSTYLENONEBORDER) == F2DWINDOWSTYLENONEBORDER)
		return F2DWINBORDERTYPE_NONE;
	return F2DWINBORDERTYPE_NONE;
}

fResult f2dWindowImpl::SetBorderType(F2DWINBORDERTYPE Type)
{
	switch(Type)
	{
	case F2DWINBORDERTYPE_NONE:
		SetWindowLongPtrW(m_hWnd, GWL_STYLE, F2DWINDOWSTYLENONEBORDER);
		break;
	case F2DWINBORDERTYPE_FIXED:
		SetWindowLongPtrW(m_hWnd, GWL_STYLE, F2DWINDOWSTYLEFIXEDBORDER);
		break;
	case F2DWINBORDERTYPE_SIZEABLE:
		SetWindowLongPtrW(m_hWnd, GWL_STYLE, F2DWINDOWSTYLESIZEABLEBORDER);
		break;
	default:
		return FCYERR_ILLEGAL;
	}

	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
		m_bShow
			? SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE
			:                  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE);
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
	m_Size.x = Range.GetWidth();
	m_Size.y = Range.GetHeight();
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
	// 计算包括窗口框架的尺寸
	RECT tWinRect = { (LONG)Range.a.x , (LONG)Range.a.y , (LONG)Range.b.x , (LONG)Range.b.y};
	platform::HighDPI::AdjustWindowRectExForDpi(
		&tWinRect,
		GetWindowLongPtrW(m_hWnd, GWL_STYLE),
		FALSE,
		GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE),
		platform::HighDPI::GetDpiForWindow(m_hWnd));
	// 记录
	m_Size.x = (float)(tWinRect.right - tWinRect.left);
	m_Size.y = (float)(tWinRect.bottom - tWinRect.top);
	// 获取最近的显示器的位置
	if (HMONITOR monitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST))
	{
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			// 偏移到该显示器0点位置
			tWinRect.left += moninfo.rcMonitor.left;
			tWinRect.right += moninfo.rcMonitor.left;
			tWinRect.top += moninfo.rcMonitor.top;
			tWinRect.bottom += moninfo.rcMonitor.top;
		}
	}
	// 最后再应用
	return ::SetWindowPos(m_hWnd, 0,
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

void f2dWindowImpl::EnterFullScreen()
{
	// 获得关联的显示器
	HMONITOR monitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if (monitor)
	{
		// 获得显示器信息
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			// 修改窗口位置
			const auto& area = moninfo.rcMonitor;
			::SetWindowPos(m_hWnd, NULL, area.left, area.top, area.right - area.left, area.bottom - area.top, SWP_NOZORDER | SWP_SHOWWINDOW);
		}
	}
	MoveMouseToRightBottom();
};

fBool f2dWindowImpl::IsTopMost()
{
	if(WS_EX_TOPMOST & GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE))
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

void f2dWindowImpl::SetIMEEnable(bool enable)
{
	::PostMessageW(m_hWnd, WM_SET_IME_ENABLE,
		enable ? (WPARAM)_defaultIMC : (WPARAM)NULL,
		enable ? (LPARAM)TRUE        : (LPARAM)FALSE);
	_enableIME = enable;
}
bool f2dWindowImpl::GetIMEEnable()
{
	return _enableIME;
}

fBool f2dWindowImpl::IsHideIME() { return m_bHideIME; }
void f2dWindowImpl::SetHideIME(fBool v) { m_bHideIME = v; }
fcStrW f2dWindowImpl::GetIMEDesc()
{
	return m_CurIMEDesc.c_str();
}
fuInt f2dWindowImpl::GetIMEInfo(F2DIMEINFO InfoType)
{
	switch(InfoType)
	{
	case F2DIMEINFO_CANDIDATECOUNT:
		return m_IMETotalCandidate;
	case F2DIMEINFO_CANDIDATEINDEX:
		return m_IMESelectedCandidate;
	case F2DIMEINFO_PAGESIZE:
		return m_IMEPageCandidateCount;
	case F2DIMEINFO_PAGESTART:
		return m_IMEPageStartCandidate;
	}
	return 0;
}
fcStrW f2dWindowImpl::GetIMECompString()
{
	return m_CurIMEComposition.c_str();
}
fuInt f2dWindowImpl::GetIMECandidateCount()
{
	return m_IMETotalCandidate;
}
fcStrW f2dWindowImpl::GetIMECandidate(fuInt Index)
{
	if(Index > m_IMETotalCandidate)
		return NULL;
	else
		return m_IMECandidateList[Index].c_str();
}

float f2dWindowImpl::GetDPIScaling()
{
	return platform::HighDPI::GetDpiScalingForWindow(m_hWnd);
}

fcyVec2 f2dWindowImpl::GetMonitorSize()
{
	fcyVec2 size;
	
	// 获得关联的显示器
	HMONITOR monitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if (monitor)
	{
		// 获得显示器信息
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			size.x = (fFloat)(moninfo.rcMonitor.right - moninfo.rcMonitor.left);
			size.y = (fFloat)(moninfo.rcMonitor.bottom - moninfo.rcMonitor.top);
		}
	}
	
	return size;
};

fuInt f2dWindowImpl::GetMonitorCount()
{
	auto& c = f2dMonitorHelper::get();
	c.refresh();
	return c.getCount();
}
fcyRect f2dWindowImpl::GetMonitorRect(fuInt index)
{
	auto& c = f2dMonitorHelper::get();
	fcyRect rect;
	rect.a.x = c.getX(index);
	rect.a.y = c.getY(index);
	rect.b.x = rect.a.x + c.getWidth(index);
	rect.b.y = rect.a.y + c.getHeight(index);
	return rect;
}
void f2dWindowImpl::MoveToMonitorCenter(fuInt index)
{
	auto& mohelp = f2dMonitorHelper::get();
	HMONITOR monitor = (HMONITOR)mohelp.getHandle(index);
	if (monitor)
	{
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			RECT rect = {};
			if (FALSE != ::GetWindowRect(m_hWnd, &rect))
			{
				const auto& area = moninfo.rcMonitor;
				const auto sx = area.left;
				const auto sy = area.top;
				const auto sw = area.right - area.left;
				const auto sh = area.bottom - area.top;
				const auto ww = rect.right - rect.left;
				const auto wh = rect.bottom - rect.top;
				::SetWindowPos(m_hWnd, NULL, sx + (sw / 2) - (ww / 2), sy + (sh / 2) - (wh / 2), ww, wh, SWP_NOZORDER | SWP_SHOWWINDOW);
			}
		}
	}
}
void f2dWindowImpl::EnterMonitorFullScreen(fuInt index)
{
	auto& mohelp = f2dMonitorHelper::get();
	HMONITOR monitor = (HMONITOR)mohelp.getHandle(index);
	if (monitor)
	{
		MONITORINFO moninfo = { sizeof(MONITORINFO), {}, {}, 0 };
		if (FALSE != ::GetMonitorInfoA(monitor, &moninfo))
		{
			const auto& area = moninfo.rcMonitor;
			::SetWindowPos(m_hWnd, NULL, area.left, area.top, area.right - area.left, area.bottom - area.top, SWP_NOZORDER | SWP_SHOWWINDOW);
		}
	}
	MoveMouseToRightBottom();
}

void f2dWindowImpl::SetCustomMoveSizeEnable(fBool v)
{
	m_SizeMoveCtrl.setEnable(v ? TRUE : FALSE);
}
void f2dWindowImpl::SetCustomMinimizeButtonRect(fcyRect v)
{
	RECT rc = {
		.left = (LONG)v.a.x,
		.top = (LONG)v.a.y,
		.right = (LONG)v.b.x,
		.bottom = (LONG)v.b.y,
	};
	m_SizeMoveCtrl.setMinimizeButtonRect(rc);
}
void f2dWindowImpl::SetCustomCloseButtonRect(fcyRect v)
{
	RECT rc = {
		.left = (LONG)v.a.x,
		.top = (LONG)v.a.y,
		.right = (LONG)v.b.x,
		.bottom = (LONG)v.b.y,
	};
	m_SizeMoveCtrl.setCloseButtonRect(rc);
}
void f2dWindowImpl::SetCustomMoveButtonRect(fcyRect v)
{
	RECT rc = {
		.left = (LONG)v.a.x,
		.top = (LONG)v.a.y,
		.right = (LONG)v.b.x,
		.bottom = (LONG)v.b.y,
	};
	m_SizeMoveCtrl.setTitleBarRect(rc);
}

void f2dWindowImpl::MoveMouseToRightBottom()
{
	RECT rc = {};
	if (!::GetClientRect(m_hWnd, &rc))
		return;
	POINT pt = { rc.right, rc.bottom };
	if (!::ClientToScreen(m_hWnd, &pt))
		return;
	::SetCursorPos(pt.x - 2, pt.y - 2);
}
