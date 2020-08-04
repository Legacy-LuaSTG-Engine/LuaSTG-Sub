#ifndef _M_ARM

#include "Input/f2dInputSysImpl.h"

#include "Input/f2dInputMouseImpl.h"
#include "Input/f2dInputKeyboardImpl.h"
#include "Input/f2dInputJoystickImpl.h"

#include "Engine/f2dEngineImpl.h"

#include "fcyOS/fcyDebug.h"
#include "fcyException.h"

//////////////////////////////////////////////////////////////////////////////////////////

BOOL PASCAL f2dInputSysImpl::enumMouse(LPCDIDEVICEINSTANCE pInfo, void* pThis)
{
	((f2dInputSysImpl*)pThis)->m_MouseList.push_back(*pInfo);

	return DIENUM_CONTINUE;
}

BOOL PASCAL f2dInputSysImpl::enumKeyboard(LPCDIDEVICEINSTANCE pInfo, void* pThis)
{
	((f2dInputSysImpl*)pThis)->m_KeyboardList.push_back(*pInfo);

	return DIENUM_CONTINUE;
}

BOOL PASCAL f2dInputSysImpl::enumGameCtrl(LPCDIDEVICEINSTANCE pInfo, void* pThis)
{
	((f2dInputSysImpl*)pThis)->m_GameCtrlList.push_back(*pInfo);

	return DIENUM_CONTINUE;
}

f2dInputSysImpl::f2dInputSysImpl(f2dEngineImpl* pEngine)
	: m_pEngine(pEngine), m_hWinHandle((HWND)pEngine->GetMainWindow()->GetHandle()), m_pDInput(NULL)
{
	HRESULT tHR = m_API.DLLEntry_DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&m_pDInput, NULL );
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dInputSysImpl::f2dInputSysImpl", "DirectInput8Create Failed.", tHR);

	enumDevice(); // 枚举所有设备
}

f2dInputSysImpl::~f2dInputSysImpl()
{
	// 报告可能的对象泄漏
	if(!m_pObjList.empty())
	{
		std::vector<f2dInputDevice*>::iterator i = m_pObjList.begin();
		while(i != m_pObjList.end())
		{
			char tTextBuffer[256];
			sprintf_s(tTextBuffer, "Unrelease input device at %p", *i);
#ifdef _DEBUG
			fcyDebug::Trace("[ @ f2dInputSysImpl::~f2dInputSysImpl ] %s\n", tTextBuffer);
#endif
			m_pEngine->ThrowException(fcyException("f2dInputSysImpl::~f2dInputSysImpl", tTextBuffer));

			++i;
		}
	}

	FCYSAFEKILL(m_pDInput);
}

void f2dInputSysImpl::enumDevice()
{
	// 枚举所有鼠标
	m_pDInput->EnumDevices(DI8DEVCLASS_POINTER, enumMouse, this, 0);
	// 枚举所有键盘
	m_pDInput->EnumDevices(DI8DEVCLASS_KEYBOARD, enumKeyboard, this, 0);
	// 枚举所有手柄
	m_pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumGameCtrl, this, 0);
}

fuInt f2dInputSysImpl::GetDeviceCount(F2DINPUTDEVTYPE Type)
{
	switch(Type)
	{
	case F2DINPUTDEVTYPE_MOUSE:
		return m_MouseList.size();
	case F2DINPUTDEVTYPE_KEYBOARD:
		return m_KeyboardList.size();
	case F2DINPUTDEVTYPE_GAMECTRL:
		return m_GameCtrlList.size();
	}
	return 0;
}

fcStrW f2dInputSysImpl::GetDeviceName(F2DINPUTDEVTYPE Type, fuInt Index)
{
	switch(Type)
	{
	case F2DINPUTDEVTYPE_MOUSE:
		if(Index >= m_MouseList.size())
			return NULL;
		return m_MouseList[Index].tszInstanceName;
	case F2DINPUTDEVTYPE_KEYBOARD:
		if(Index >= m_KeyboardList.size())
			return NULL;
		return m_KeyboardList[Index].tszInstanceName;
	case F2DINPUTDEVTYPE_GAMECTRL:
		if(Index >= m_GameCtrlList.size())
			return NULL;
		return m_GameCtrlList[Index].tszInstanceName;
	}
	return NULL;
}

fcStrW f2dInputSysImpl::GetDeviceProductName(F2DINPUTDEVTYPE Type, fuInt Index)
{
	switch(Type)
	{
	case F2DINPUTDEVTYPE_MOUSE:
		if(Index >= m_MouseList.size())
			return NULL;
		return m_MouseList[Index].tszProductName;
	case F2DINPUTDEVTYPE_KEYBOARD:
		if(Index >= m_KeyboardList.size())
			return NULL;
		return m_KeyboardList[Index].tszProductName;
	case F2DINPUTDEVTYPE_GAMECTRL:
		if(Index >= m_GameCtrlList.size())
			return NULL;
		return m_GameCtrlList[Index].tszProductName;
	}
	return NULL;
}

fResult f2dInputSysImpl::CreateMouse(fInt DevIndex, fBool bGlobalFocus, f2dInputMouse** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	GUID tGuid = GUID_SysMouse;
	
	if(DevIndex != -1)
	{
		if(DevIndex<0 || (fuInt)DevIndex>=m_MouseList.size())
			return FCYERR_INVAILDPARAM;

		tGuid = m_MouseList[DevIndex].guidInstance;
	}

	f2dInputMouse* pRet = NULL;
	try
	{
		pRet = new f2dInputMouseImpl(this, m_hWinHandle, tGuid, bGlobalFocus);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pRet;

	return FCYERR_OK;
}

fResult f2dInputSysImpl::CreateKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	GUID tGuid = GUID_SysKeyboard;
	
	if(DevIndex != -1)
	{
		if(DevIndex<0 || (fuInt)DevIndex>=m_KeyboardList.size())
			return FCYERR_INVAILDPARAM;

		tGuid = m_KeyboardList[DevIndex].guidInstance;
	}

	f2dInputKeyboard* pRet = NULL;
	try
	{
		pRet = new f2dInputKeyboardImpl(this, m_hWinHandle, tGuid, bGlobalFocus);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pRet;

	return FCYERR_OK;
}

fResult f2dInputSysImpl::CreateDefaultKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut)
{
	if (pOut)
		* pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;

	GUID tGuid = GUID_SysKeyboard;

	if (DevIndex != -1)
	{
		if (DevIndex < 0 || (fuInt)DevIndex >= m_KeyboardList.size())
			return FCYERR_INVAILDPARAM;

		tGuid = m_KeyboardList[DevIndex].guidInstance;
	}

	f2dInputKeyboard* pRet = NULL;
	try
	{
		pRet = new f2dInputKeyboardImpl2(this, m_hWinHandle, tGuid, bGlobalFocus);
	}
	catch (const fcyException & e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pRet;

	return FCYERR_OK;
}

fResult f2dInputSysImpl::CreateJoystick(fInt DevIndex, fBool bGlobalFocus, f2dInputJoystick** pOut)
{
	if(pOut)
		*pOut = NULL;
	else
		return FCYERR_INVAILDPARAM;
	if(DevIndex<0 || (fuInt)DevIndex>=m_GameCtrlList.size())
		return FCYERR_INVAILDPARAM;

	GUID tGuid = m_GameCtrlList[DevIndex].guidInstance;

	f2dInputJoystick* pRet = NULL;
	try
	{
		pRet = new f2dInputJoystickImpl(this, m_hWinHandle, tGuid, bGlobalFocus);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	*pOut = pRet;

	return FCYERR_OK;
}

#endif
