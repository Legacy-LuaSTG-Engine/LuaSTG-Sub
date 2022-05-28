#include "Engine/f2dWindowImpl.h"
#include "Engine/f2dEngineImpl.h"

#include <fcyException.h>

inline fcyRect fcyRect_from(LuaSTG::Core::RectI v)
{
	return fcyRect(v.a.x, v.a.y, v.b.x, v.b.y);
}
LuaSTG::Core::RectI coreRect_from(fcyRect v)
{
	return LuaSTG::Core::RectI(v.a.x, v.a.y, v.b.x, v.b.y);
}

////////////////////////////////////////////////////////////////////////////////

f2dWindowImpl::f2dWindowImpl(f2dEngineImpl* pEngine, const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border, bool DisableIME)
	: m_DefaultListener(pEngine, this)
	, m_pListener(&m_DefaultListener)
{
	m_window = dynamic_cast<LuaSTG::Core::Graphics::Window_Win32*>(pEngine->GGetAppModel()->getWindow());

	m_window->addEventListener(this);
}
f2dWindowImpl::~f2dWindowImpl()
{
	m_window->removeEventListener(this);
}

void f2dWindowImpl::AddNativeMessageCallback(fHandle pWndProc)
{
	RemoveNativeMessageCallback(pWndProc);
	m_fProc.push_back(pWndProc);
}
void f2dWindowImpl::RemoveNativeMessageCallback(fHandle pWndProc)
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
	return (fHandle)m_window->getNativeHandle();
}

fResult f2dWindowImpl::SetBorderType(F2DWINBORDERTYPE Type)
{
	m_window->setFrameStyle((LuaSTG::Core::Graphics::WindowFrameStyle)Type);
	return FCYERR_OK;
}

fResult f2dWindowImpl::SetCaption(fcStr Caption)
{
	m_window->setTitleText(Caption);
	return FCYERR_OK;
}

fResult f2dWindowImpl::SetVisiable(fBool Visiable)
{
	if (Visiable)
		m_window->setLayer(LuaSTG::Core::Graphics::WindowLayer::Normal);
	else
		m_window->setLayer(LuaSTG::Core::Graphics::WindowLayer::Invisible);
	return FCYERR_OK;
}

fResult f2dWindowImpl::SetClientRect(const fcyRect& Range)
{
	LuaSTG::Core::Vector2I _size(Range.GetWidth(), Range.GetHeight());
	m_window->setSize(_size);
	return FCYERR_OK;
}

fResult f2dWindowImpl::SetTopMost(fBool TopMost)
{
	if (TopMost)
		m_window->setLayer(LuaSTG::Core::Graphics::WindowLayer::TopMost);
	else
		m_window->setLayer(LuaSTG::Core::Graphics::WindowLayer::Normal);
	return FCYERR_OK;
}

void f2dWindowImpl::HideMouse(fBool bHide)
{
	if (bHide)
		m_window->setCursor(LuaSTG::Core::Graphics::WindowCursor::None);
	else
		m_window->setCursor(LuaSTG::Core::Graphics::WindowCursor::Arrow);
}

void f2dWindowImpl::SetIMEEnable(bool enable)
{
	m_window->setIMEState(enable);
}

float f2dWindowImpl::GetDPIScaling()
{
	return m_window->getDPIScaling();
}

fcyVec2 f2dWindowImpl::GetMonitorSize()
{
	auto v = m_window->getMonitorSize();
	return fcyVec2(v.x, v.y);
};
void f2dWindowImpl::MoveToCenter()
{
	m_window->setCentered();
}
void f2dWindowImpl::EnterFullScreen()
{
	m_window->setFullScreen();
};

fuInt f2dWindowImpl::GetMonitorCount()
{
	return m_window->getMonitorCount();
}
fcyRect f2dWindowImpl::GetMonitorRect(fuInt index)
{
	return fcyRect_from(m_window->getMonitorRect(index));
}
void f2dWindowImpl::MoveToMonitorCenter(fuInt index)
{
	m_window->setMonitorCentered(index);
}
void f2dWindowImpl::EnterMonitorFullScreen(fuInt index)
{
	m_window->setMonitorFullScreen(index);
}

void f2dWindowImpl::SetCustomMoveSizeEnable(fBool v)
{
	m_window->setCustomSizeMoveEnable(v);
}
void f2dWindowImpl::SetCustomMinimizeButtonRect(fcyRect v)
{
	m_window->setCustomMoveButtonRect(coreRect_from(v));
}
void f2dWindowImpl::SetCustomCloseButtonRect(fcyRect v)
{
	m_window->setCustomMoveButtonRect(coreRect_from(v));
}
void f2dWindowImpl::SetCustomMoveButtonRect(fcyRect v)
{
	m_window->setCustomMoveButtonRect(coreRect_from(v));
}

void f2dWindowImpl::MoveMouseToRightBottom()
{
	m_window->setCursorToRightBottom();
}
