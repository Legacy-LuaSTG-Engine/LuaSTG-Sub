////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputSysImpl.h
/// @brief fancy2D输入系统实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Input/f2dInputSysAPI.h"

#include "f2dInputSys.h"

#include <vector>

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 输入系统实现
////////////////////////////////////////////////////////////////////////////////
class f2dInputSysImpl :
	public fcyRefObjImpl<f2dInputSys>
{
private:
	f2dEngineImpl* m_pEngine;

	f2dInputSysAPI m_API;

	IDirectInput8* m_pDInput;
	HWND m_hWinHandle;
	std::vector<DIDEVICEINSTANCEW> m_MouseList;    // 鼠标列表
	std::vector<DIDEVICEINSTANCEW> m_KeyboardList; // 键盘列表
	std::vector<DIDEVICEINSTANCEW> m_GameCtrlList; // 控制器列表

	std::vector<f2dInputDevice*> m_pObjList;       // 设备列表
private:
	static BOOL PASCAL enumMouse(LPCDIDEVICEINSTANCE pInfo, void* pThis);    // 枚举鼠标
	static BOOL PASCAL enumKeyboard(LPCDIDEVICEINSTANCE pInfo, void* pThis); // 枚举键盘
	static BOOL PASCAL enumGameCtrl(LPCDIDEVICEINSTANCE pInfo, void* pThis); // 枚举游戏手柄
private:
	void enumDevice();  // 枚举所有设备
public:  // 内部方法
	f2dEngineImpl* GetEngine()
	{
		return m_pEngine;
	}

	IDirectInput8* GetHandle()
	{
		return m_pDInput;
	}

	void RegisterDevice(f2dInputDevice* pDev)
	{
		m_pObjList.push_back(pDev);
	}
	void UnregisterDevice(f2dInputDevice* pDev)
	{
		std::vector<f2dInputDevice*>::iterator i = m_pObjList.begin();
		while(i != m_pObjList.end())
		{
			if(*i == pDev)
			{
				m_pObjList.erase(i);
				break;
			}
			else
				++i;
		}
	}
public:  // 接口实现
	fuInt GetDeviceCount(F2DINPUTDEVTYPE Type);
	fcStrW GetDeviceName(F2DINPUTDEVTYPE Type, fuInt Index);
	fcStrW GetDeviceProductName(F2DINPUTDEVTYPE Type, fuInt Index);
	
	fResult CreateMouse(fInt DevIndex, fBool bGlobalFocus, f2dInputMouse** pOut);
	fResult CreateKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut);
	fResult CreateDefaultKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut);
	fResult CreateJoystick(fInt DevIndex, fBool bGlobalFocus, f2dInputJoystick** pOut);
	
	void Update() {
		std::vector<f2dInputDevice*>::iterator i = m_pObjList.begin();
		while(i != m_pObjList.end())
		{
			(*i)->UpdateState();

			++i;
		}
	}
	void Reset() {
		std::vector<f2dInputDevice*>::iterator i = m_pObjList.begin();
		while(i != m_pObjList.end())
		{
			(*i)->ResetState();

			++i;
		}
	}
public:
	f2dInputSysImpl(f2dEngineImpl* pEngine);
protected:
	~f2dInputSysImpl();
};
