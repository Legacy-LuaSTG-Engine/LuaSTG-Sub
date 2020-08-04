////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputKeyboardImpl.h
/// @brief fancy2D输入系统 键盘实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dInputSys.h"

#include "Input/f2dInputSysAPI.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 键盘设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dInputKeyboardImpl :
	public fcyRefObjImpl<f2dInputKeyboard>
{
	friend class f2dInputSysImpl;
protected:
	class DefaultListener :
		public f2dInputKeyboardEventListener
	{
	protected:
		f2dEngineImpl* m_pEngine;
	public:
		void OnKeyboardBtnDown(F2DINPUTKEYCODE KeyCode);
		void OnKeyboardBtnUp(F2DINPUTKEYCODE KeyCode);
	public:
		DefaultListener(f2dInputSysImpl* pInputSys);
	};
private:
	static const fuInt BufferSize; 
	static const DIDATAFORMAT DIDF_Keyboard;
	static const DIOBJECTDATAFORMAT DIODF_Keyboard[256];
private:
	f2dInputSysImpl* m_pSys;
	IDirectInputDevice8* m_pDev;
	DefaultListener m_DefaultListener;
	f2dInputKeyboardEventListener* m_pListener;
	fBool m_ButtonState[256];
public:
	f2dInputMouse* ToMouse();
	f2dInputKeyboard* ToKeyboard();
	f2dInputJoystick* ToJoystick();

	fResult UpdateState(); 

	f2dInputKeyboardEventListener* GetListener();
	fResult SetListener(f2dInputKeyboardEventListener* pListener);
	
	fBool IsKeyDown(F2DINPUTKEYCODE KeyCode);
	fBool KeyPress(DWORD KeyCode) { return false; };
protected: // 禁止直接new/delete
	f2dInputKeyboardImpl(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus);
	~f2dInputKeyboardImpl();
};

class f2dInputKeyboardImpl2 :
	public fcyRefObjImpl<f2dInputKeyboard>
{
	friend class f2dInputSysImpl;
protected:
	class DefaultListener :
		public f2dInputKeyboardEventListener
	{
	protected:
		f2dEngineImpl* m_pEngine;
	public:
		void OnKeyboardBtnDown(F2DINPUTKEYCODE KeyCode);
		void OnKeyboardBtnUp(F2DINPUTKEYCODE KeyCode);
	public:
		DefaultListener(f2dInputSysImpl* pInputSys);
	};
private:
	static const fuInt BufferSize;
private:
	f2dInputSysImpl* m_pSys;
	IDirectInputDevice8* m_pDev;
	DefaultListener m_DefaultListener;
	f2dInputKeyboardEventListener* m_pListener;
	fBool m_ButtonState[256];
public:
	f2dInputMouse* ToMouse();
	f2dInputKeyboard* ToKeyboard();
	f2dInputJoystick* ToJoystick();

	fResult UpdateState();

	f2dInputKeyboardEventListener* GetListener();
	fResult SetListener(f2dInputKeyboardEventListener* pListener);

	fBool IsKeyDown(F2DINPUTKEYCODE KeyCode) { return false; }
	fBool KeyPress(DWORD KeyCode);
protected: // 禁止直接new/delete
	f2dInputKeyboardImpl2(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus);
	~f2dInputKeyboardImpl2();
};
