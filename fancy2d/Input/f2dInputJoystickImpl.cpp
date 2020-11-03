#ifndef _M_ARM

#include "Input/f2dInputJoystickImpl.h"

#include "Engine/f2dEngineImpl.h"
#include "Input/f2dInputSysImpl.h"

//////////////////////////////////////////////////////////////////////////////////////////

f2dInputJoystickImpl::DefaultListener::DefaultListener(f2dInputSysImpl* pInputSys)
	: m_pEngine(pInputSys->GetEngine()) {}

void f2dInputJoystickImpl::DefaultListener::OnJoystickBtnDown(f2dInputJoystick* p, fuInt Index)
{
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONBUTTONDOWN, Index, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickBtnUp(f2dInputJoystick* p, fuInt Index)
{
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONBUTTONUP, Index, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickXAxisPosChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONXPOSCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickYAxisPosChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONYPOSCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickZAxisPosChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONZPOSCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickXAxisRotationChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONXROTCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickYAxisRotationChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONYROTCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

void f2dInputJoystickImpl::DefaultListener::OnJoystickZAxisRotationChange(f2dInputJoystick* p, fFloat Value)
{
	fDouble tValue = Value;
	m_pEngine->SendMsg(F2DMSG_JOYSTICK_ONZROTCHANGE, *(fuLong*)&tValue, (fuLong)p);
}

//////////////////////////////////////////////////////////////////////////////////////////

const fuInt f2dInputJoystickImpl::BufferSize = 32;

const DIDATAFORMAT f2dInputJoystickImpl::DIDF_Joystick = 
{
	24, 16, 1, 80, 44,
	(LPDIOBJECTDATAFORMAT)&f2dInputJoystickImpl::DIODF_Joystick
};

const DIOBJECTDATAFORMAT f2dInputJoystickImpl::DIODF_Joystick[44] = 
{
	{ &GUID_XAxis, 0, 2164260611, 256 },
	{ &GUID_YAxis, 4, 2164260611, 256 },
	{ &GUID_ZAxis, 8, 2164260611, 256 },
	{ &GUID_RxAxis, 12, 2164260611, 256 },
	{ &GUID_RyAxis, 16, 2164260611, 256 },
	{ &GUID_RzAxis, 20, 2164260611, 256 },
	{ &GUID_Slider, 24, 2164260611, 256 },
	{ &GUID_Slider, 28, 2164260611, 256 },
	{ &GUID_POV, 32, 2164260624, 0 },
	{ &GUID_POV, 36, 2164260624, 0 },
	{ &GUID_POV, 40, 2164260624, 0 },
	{ &GUID_POV, 44, 2164260624, 0 },
	{ 00000000, 48, 2164260620, 0 },
	{ 00000000, 49, 2164260620, 0 },
	{ 00000000, 50, 2164260620, 0 },
	{ 00000000, 51, 2164260620, 0 },
	{ 00000000, 52, 2164260620, 0 },
	{ 00000000, 53, 2164260620, 0 },
	{ 00000000, 54, 2164260620, 0 },
	{ 00000000, 55, 2164260620, 0 },
	{ 00000000, 56, 2164260620, 0 },
	{ 00000000, 57, 2164260620, 0 },
	{ 00000000, 58, 2164260620, 0 },
	{ 00000000, 59, 2164260620, 0 },
	{ 00000000, 60, 2164260620, 0 },
	{ 00000000, 61, 2164260620, 0 },
	{ 00000000, 62, 2164260620, 0 },
	{ 00000000, 63, 2164260620, 0 },
	{ 00000000, 64, 2164260620, 0 },
	{ 00000000, 65, 2164260620, 0 },
	{ 00000000, 66, 2164260620, 0 },
	{ 00000000, 67, 2164260620, 0 },
	{ 00000000, 68, 2164260620, 0 },
	{ 00000000, 69, 2164260620, 0 },
	{ 00000000, 70, 2164260620, 0 },
	{ 00000000, 71, 2164260620, 0 },
	{ 00000000, 72, 2164260620, 0 },
	{ 00000000, 73, 2164260620, 0 },
	{ 00000000, 74, 2164260620, 0 },
	{ 00000000, 75, 2164260620, 0 },
	{ 00000000, 76, 2164260620, 0 },
	{ 00000000, 77, 2164260620, 0 },
	{ 00000000, 78, 2164260620, 0 },
	{ 00000000, 79, 2164260620, 0 }
};

f2dInputJoystickImpl::f2dInputJoystickImpl(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, bool bGlobalFocus)
	: m_pSys(pSys), m_pDev(NULL), m_DefaultListener(pSys), m_pListener(&m_DefaultListener),
	m_lXHalf(0), m_lXHalfLen(0), m_lYHalf(0), m_lYHalfLen(0), m_lZHalf(0), m_lZHalfLen(0), 
	m_lRxHalf(0), m_lRxHalfLen(0), m_lRyHalf(0), m_lRyHalfLen(0), m_lRzHalf(0), m_lRzHalfLen(0),
	m_lX(0.f), m_lY(0.f), m_lZ(0.f), m_lRx(0.f), m_lRy(0.f), m_lRz(0.f)
{
	IDirectInput8* pDev = pSys->GetHandle();

	memset(m_ButtonDown, 0, sizeof(m_ButtonDown));
	memset(m_Slider, 0, sizeof(m_Slider));
	memset(m_POV, 0, sizeof(m_POV));

	HRESULT tHR = pDev->CreateDevice(pGUID, &m_pDev, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dInputJoystickImpl::f2dInputJoystickImpl", "CreateDevice Failed.", tHR);
	
	// 设置协作模式
	fuInt tFlag = DISCL_NONEXCLUSIVE;
	if(bGlobalFocus)
		tFlag |= DISCL_BACKGROUND;
	else
		tFlag |= DISCL_FOREGROUND;
	tHR = m_pDev->SetCooperativeLevel(Win, tFlag);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputJoystickImpl::f2dInputJoystickImpl", "SetCooperativeLevel Failed.", tHR);
	}

    // 设置数据格式
	tHR = m_pDev->SetDataFormat(&DIDF_Joystick);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputJoystickImpl::f2dInputJoystickImpl", "SetDataFormat Failed.", tHR);
	}
	
	// 设置缓冲区
	DIPROPDWORD tBufferProperty;

    tBufferProperty.diph.dwSize = sizeof(DIPROPDWORD);
    tBufferProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    tBufferProperty.diph.dwObj = 0;
    tBufferProperty.diph.dwHow = DIPH_DEVICE;
    tBufferProperty.dwData = BufferSize;

    tHR = m_pDev->SetProperty(DIPROP_BUFFERSIZE, &tBufferProperty.diph);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputJoystickImpl::f2dInputJoystickImpl", "SetProperty Failed.", tHR);
	}

	initStates();

	// 获得设备
	tHR = m_pDev->Acquire();

	// 注册
	m_pSys->RegisterDevice(this);
}

f2dInputJoystickImpl::~f2dInputJoystickImpl()
{
	if(m_pDev)
		m_pDev->Unacquire();

	// 取消注册
	m_pSys->UnregisterDevice(this);

	FCYSAFEKILL(m_pDev);
}

void f2dInputJoystickImpl::initStates()
{
	HRESULT tHR;
	DIPROPRANGE tAxisRangeDesc;
	tAxisRangeDesc.diph.dwSize= sizeof( tAxisRangeDesc );
	tAxisRangeDesc.diph.dwHeaderSize = sizeof( tAxisRangeDesc.diph );
	tAxisRangeDesc.diph.dwHow = DIPH_BYOFFSET;
	tAxisRangeDesc.lMin = -40000;
	tAxisRangeDesc.lMax = 40000;

	// --- 设置轴参考范围 ---
	// X轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_X;
    tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	// Y轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_Y;
    tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	// Z轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_Z;
	tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	// X轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RX;
	tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	// Y轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RY;
	tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	// Z轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RZ;
	tHR = m_pDev->SetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);

	// --- 获得轴参考范围 ---
	// X轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_X;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lXHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lXHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
	// Y轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_Y;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lYHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lYHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
	// Z轴
	tAxisRangeDesc.diph.dwObj = DIJOFS_Z;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lZHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lZHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
	// X轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RX;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lRxHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lRxHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
	// Y轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RY;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lRyHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lRyHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
	// Z轴旋转
	tAxisRangeDesc.diph.dwObj = DIJOFS_RZ;
    tHR = m_pDev->GetProperty(DIPROP_RANGE, &tAxisRangeDesc.diph);
	if(SUCCEEDED(tHR))
	{
		m_lRzHalf = (tAxisRangeDesc.lMin + tAxisRangeDesc.lMax)/2;
		m_lRzHalfLen = (tAxisRangeDesc.lMax - tAxisRangeDesc.lMin)/2;
	}
}

f2dInputMouse* f2dInputJoystickImpl::ToMouse()
{
	return NULL;
}

f2dInputKeyboard* f2dInputJoystickImpl::ToKeyboard()
{
	return NULL;
}

f2dInputJoystick* f2dInputJoystickImpl::ToJoystick()
{
	return this;
}

fResult f2dInputJoystickImpl::UpdateState()
{
	DIDEVICEOBJECTDATA tRawData[BufferSize];
	DWORD tSize = BufferSize;

	HRESULT tHR = m_pDev->GetDeviceData(
		sizeof(DIDEVICEOBJECTDATA),
		tRawData,
		&tSize,
		0);
	
	if(FAILED(tHR))
	{
		tHR = m_pDev->Acquire();
		
		if(FAILED(tHR))
			return FCYERR_INTERNALERR;
		else
		{
			tSize = BufferSize;
			tHR = m_pDev->GetDeviceData(
				sizeof(DIDEVICEOBJECTDATA),
				tRawData,
				&tSize,
				0);
			if(FAILED(tHR))
				return FCYERR_INTERNALERR;
		}
	}

	for(fuInt i = 0; i<tSize; ++i)
	{
		float tValue = 0.f;
		switch(tRawData[i].dwOfs)
		{
		case DIJOFS_X:
			tValue = ((int)tRawData[i].dwData - m_lXHalf) / (float)(m_lXHalfLen);
			m_lX = tValue;
			if(m_pListener)
				m_pListener->OnJoystickXAxisPosChange(this, tValue);
			break;
		case DIJOFS_Y:
			tValue = ((int)tRawData[i].dwData - m_lYHalf) / (float)(m_lYHalfLen);
			m_lY = tValue;
			if(m_pListener)
				m_pListener->OnJoystickYAxisPosChange(this, tValue);
			break;
		case DIJOFS_Z:
			tValue = ((int)tRawData[i].dwData - m_lZHalf) / (float)(m_lZHalfLen);
			m_lZ = tValue;
			if(m_pListener)
				m_pListener->OnJoystickZAxisPosChange(this, tValue);
			break;
		case DIJOFS_RX:
			tValue = ((int)tRawData[i].dwData - m_lRxHalf) / (float)(m_lRxHalfLen);
			m_lRx = tValue;
			if(m_pListener)
				m_pListener->OnJoystickXAxisRotationChange(this, tValue);
			break;
		case DIJOFS_RY:
			tValue = ((int)tRawData[i].dwData - m_lRyHalf) / (float)(m_lRyHalfLen);
			m_lRy = tValue;
			if(m_pListener)
				m_pListener->OnJoystickYAxisRotationChange(this, tValue);
			break;
		case DIJOFS_RZ:
			tValue = ((int)tRawData[i].dwData - m_lRzHalf) / (float)(m_lRzHalfLen);
			m_lRz = tValue;
			if(m_pListener)
				m_pListener->OnJoystickZAxisRotationChange(this, tValue);
			break;
		case DIJOFS_SLIDER(0):
			m_Slider[0] = tRawData[i].dwData;
			break;
		case DIJOFS_SLIDER(1):
			m_Slider[1] = tRawData[i].dwData;
			break;
		case DIJOFS_POV(0):
			m_POV[0] = tRawData[i].dwData;
			break;
		case DIJOFS_POV(1):
			m_POV[1] = tRawData[i].dwData;
			break;
		case DIJOFS_POV(2):
			m_POV[2] = tRawData[i].dwData;
			break;
		case DIJOFS_POV(3):
			m_POV[3] = tRawData[i].dwData;
			break;
		default:
			if(tRawData[i].dwOfs>=DIJOFS_BUTTON0 && tRawData[i].dwOfs<=DIJOFS_BUTTON31)
			{
				bool tDown = (LOWORD(tRawData[i].dwData)&0x80) != 0;
				m_ButtonDown[tRawData[i].dwOfs - DIJOFS_BUTTON0] = tDown;
				if(m_pListener)
					if(tDown)
						m_pListener->OnJoystickBtnDown(this, tRawData[i].dwOfs - DIJOFS_BUTTON0);
					else
						m_pListener->OnJoystickBtnUp(this, tRawData[i].dwOfs - DIJOFS_BUTTON0);
			}
		}
	}

	return FCYERR_OK;
}

void f2dInputJoystickImpl::ResetState() {
	m_lX = 0.0f;
	m_lY = 0.0f;
	m_lZ = 0.0f;
	m_lRx = 0.0f;
	m_lRy = 0.0f;
	m_lRz = 0.0f;
	memset(&m_ButtonDown, 0, sizeof(m_ButtonDown));
	memset(&m_Slider, 0, sizeof(m_Slider));
	memset(&m_POV, 0, sizeof(m_POV));
}

f2dInputJoystickEventListener* f2dInputJoystickImpl::GetListener()
{
	return m_pListener;
}

fResult f2dInputJoystickImpl::SetListener(f2dInputJoystickEventListener* pListener)
{
	m_pListener = pListener;
	return FCYERR_OK;
}

fFloat f2dInputJoystickImpl::GetXPosition()
{
	return m_lX;
}

fFloat f2dInputJoystickImpl::GetYPosition()
{
	return m_lY;
}

fFloat f2dInputJoystickImpl::GetZPosition()
{
	return m_lZ;
}

fFloat f2dInputJoystickImpl::GetXRotation()
{
	return m_lRx;
}

fFloat f2dInputJoystickImpl::GetYRotation()
{
	return m_lRy;
}

fFloat f2dInputJoystickImpl::GetZRotation()
{
	return m_lRz;
}

fBool f2dInputJoystickImpl::IsButtonDown(fuInt Index)
{
	if(Index > 31)
		return false;

	return m_ButtonDown[Index];
}

fInt f2dInputJoystickImpl::GetSlider(fuInt Index)
{
	if(Index>1)
		return 0;
	return m_Slider[Index];
}

fuInt f2dInputJoystickImpl::GetPOV(fuInt Index)
{
	if(Index>3)
		return 0;
	return m_POV[Index];
}

fFloat f2dInputJoystickImpl::GetDeadZone()
{
	float tRet = 0.f;
	DIPROPDWORD tDeadZoneDesc;
	tDeadZoneDesc.diph.dwSize= sizeof( tDeadZoneDesc );
	tDeadZoneDesc.diph.dwHeaderSize = sizeof( tDeadZoneDesc.diph );
	tDeadZoneDesc.diph.dwHow = DIPH_BYOFFSET;
	tDeadZoneDesc.diph.dwObj = DIJOFS_X;
    HRESULT tHR = m_pDev->GetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);
	if(SUCCEEDED(tHR))
	{
		tRet = tDeadZoneDesc.dwData / 10000.f;
	}
	return tRet;
}

fResult f2dInputJoystickImpl::SetDeadZone(fFloat Percent)
{
	if(Percent<0.f || Percent>1.f)
		return FCYERR_INVAILDPARAM;
	DWORD tValue = (DWORD)(Percent * 10000);
	DIPROPDWORD tDeadZoneDesc;
	tDeadZoneDesc.diph.dwSize= sizeof( tDeadZoneDesc );
	tDeadZoneDesc.diph.dwHeaderSize = sizeof( tDeadZoneDesc.diph );
	tDeadZoneDesc.diph.dwHow = DIPH_BYOFFSET;
	tDeadZoneDesc.dwData = tValue;

	HRESULT tHR;

	tDeadZoneDesc.diph.dwObj = DIJOFS_X;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_Y;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_Z;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RX;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RY;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RZ;
    tHR = m_pDev->SetProperty(DIPROP_DEADZONE, &tDeadZoneDesc.diph);

	return FCYERR_OK;
}

fFloat f2dInputJoystickImpl::GetSaturation()
{
	float tRet = 0.f;
	DIPROPDWORD tDeadZoneDesc;
	tDeadZoneDesc.diph.dwSize= sizeof( tDeadZoneDesc );
	tDeadZoneDesc.diph.dwHeaderSize = sizeof( tDeadZoneDesc.diph );
	tDeadZoneDesc.diph.dwHow = DIPH_BYOFFSET;
	tDeadZoneDesc.diph.dwObj = DIJOFS_X;
    HRESULT tHR = m_pDev->GetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);
	if(SUCCEEDED(tHR))
	{
		tRet = tDeadZoneDesc.dwData / 10000.f;
	}
	return tRet;
}

fResult f2dInputJoystickImpl::SetSaturation(fFloat Percent)
{
	if(Percent<0.f || Percent>1.f)
		return FCYERR_INVAILDPARAM;
	DWORD tValue = (DWORD)(Percent * 10000);
	DIPROPDWORD tDeadZoneDesc;
	tDeadZoneDesc.diph.dwSize= sizeof( tDeadZoneDesc );
	tDeadZoneDesc.diph.dwHeaderSize = sizeof( tDeadZoneDesc.diph );
	tDeadZoneDesc.diph.dwHow = DIPH_BYOFFSET;
	tDeadZoneDesc.dwData = tValue;

	HRESULT tHR;

	tDeadZoneDesc.diph.dwObj = DIJOFS_X;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_Y;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_Z;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RX;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RY;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	tDeadZoneDesc.diph.dwObj = DIJOFS_RZ;
    tHR = m_pDev->SetProperty(DIPROP_SATURATION, &tDeadZoneDesc.diph);

	return FCYERR_OK;
}

#endif
