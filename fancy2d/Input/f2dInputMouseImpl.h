////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputMouseImpl.h
/// @brief fancy2D输入系统 鼠标实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"
#include "f2dInputSys.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 鼠标设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dInputMouseImpl :
	public fcyRefObjImpl<f2dInputMouse>
{
	friend class f2dInputSysImpl;
protected:
	class DefaultListener :
		public f2dInputMouseEventListener
	{
	protected:
		f2dEngineImpl* m_pEngine;
	public:
		void OnMouseMoveX(fInt Value);
		void OnMouseMoveY(fInt Value);
		void OnMouseMoveZ(fInt Value);
		void OnMouseLBtnDown();
		void OnMouseRBtnDown();
		void OnMouseMBtnDown();
		void OnMouseAdditionBtnDown(fuInt Index);
		void OnMouseLBtnUp();
		void OnMouseRBtnUp();
		void OnMouseMBtnUp();
		void OnMouseAdditionBtnUp(fuInt Index);
	public:
		DefaultListener(f2dInputSysImpl* pInputSys);
	};
private:
	static const fuInt BufferSize;
private:
	f2dInputSysImpl* m_pSys;
	IDirectInputDevice8W* m_pDev;
	DefaultListener m_DefaultListener;
	f2dInputMouseEventListener* m_pListener;
	
	int m_TotalOffsetX;
	int m_TotalOffsetY;
	int m_TotalOffsetZ;
	bool m_BtnState[8];
public: // 接口实现
	f2dInputMouse* ToMouse();
	f2dInputKeyboard* ToKeyboard();
	f2dInputJoystick* ToJoystick();

	fResult UpdateState();
	void ResetState();

	f2dInputMouseEventListener* GetListener();
	fResult SetListener(f2dInputMouseEventListener* pListener);
	
	fBool IsLeftBtnDown();
	fBool IsRightBtnDown();
	fBool IsMiddleBtnDown();
	fBool IsAdditionBtnDown(fuInt Index);
	fInt GetOffsetX();
	fInt GetOffsetY();
	fInt GetOffsetZ();
protected: // 禁止直接new/delete
	f2dInputMouseImpl(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus);
	~f2dInputMouseImpl();
};
