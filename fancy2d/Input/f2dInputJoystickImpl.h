////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputJoystickImpl.h
/// @brief fancy2D输入系统 手柄实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#include "f2dInputSys.h"

#include "Input/f2dInputSysAPI.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 游戏手柄实现
////////////////////////////////////////////////////////////////////////////////
class f2dInputJoystickImpl :
	public fcyRefObjImpl<f2dInputJoystick>
{
	friend class f2dInputSysImpl;
protected:
	class DefaultListener :
		public f2dInputJoystickEventListener
	{
	protected:
		f2dEngineImpl* m_pEngine;
	public:
		void OnJoystickBtnDown(f2dInputJoystick* p, fuInt Index);
		void OnJoystickBtnUp(f2dInputJoystick* p, fuInt Index);
		void OnJoystickXAxisPosChange(f2dInputJoystick* p, fFloat Value);
		void OnJoystickYAxisPosChange(f2dInputJoystick* p, fFloat Value);
		void OnJoystickZAxisPosChange(f2dInputJoystick* p, fFloat Value);
		void OnJoystickXAxisRotationChange(f2dInputJoystick* p, fFloat Value);
		void OnJoystickYAxisRotationChange(f2dInputJoystick* p, fFloat Value);
		void OnJoystickZAxisRotationChange(f2dInputJoystick* p, fFloat Value);
	public:
		DefaultListener(f2dInputSysImpl* pInputSys);
	};
private:
	static const fuInt BufferSize;
	static const DIDATAFORMAT DIDF_Joystick;
	static const DIOBJECTDATAFORMAT DIODF_Joystick[44];
private:
	f2dInputSysImpl* m_pSys;
	IDirectInputDevice8* m_pDev;
	DefaultListener m_DefaultListener;
	f2dInputJoystickEventListener* m_pListener;

	// 范围值
	int m_lXHalf;
	int m_lXHalfLen;
	int m_lYHalf;
	int m_lYHalfLen;
	int m_lZHalf;
	int m_lZHalfLen;
	int m_lRxHalf;
	int m_lRxHalfLen;
	int m_lRyHalf;
	int m_lRyHalfLen;
	int m_lRzHalf;
	int m_lRzHalfLen;

	// 状态
	float m_lX;               // X轴位置
    float m_lY;               // Y轴位置
    float m_lZ;               // Z轴位置
    float m_lRx;              // X轴旋转
    float m_lRy;              // Y轴旋转
    float m_lRz;              // Z轴旋转
	bool m_ButtonDown[32];    // 按钮状态
	int m_Slider[2];          // Slider
	fuInt m_POV[4];            // POV
private:
	void initStates();
public:
	f2dInputMouse* ToMouse();
	f2dInputKeyboard* ToKeyboard();
	f2dInputJoystick* ToJoystick();

	fResult UpdateState();
	void ResetState();

	f2dInputJoystickEventListener* GetListener();
	fResult SetListener(f2dInputJoystickEventListener* pListener);

	// 状态查询
	fFloat GetXPosition();
	fFloat GetYPosition();
	fFloat GetZPosition();
	fFloat GetXRotation();
	fFloat GetYRotation();
	fFloat GetZRotation();
	fBool IsButtonDown(fuInt Index);
	fInt GetSlider(fuInt Index);
	fuInt GetPOV(fuInt Index);

	fFloat GetDeadZone();
	fResult SetDeadZone(fFloat Percent);
	fFloat GetSaturation();
	fResult SetSaturation(fFloat Percent);
protected: // 禁止直接new/delete
	f2dInputJoystickImpl(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus);
	~f2dInputJoystickImpl();
};
