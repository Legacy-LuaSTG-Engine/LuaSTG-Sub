#ifndef _M_ARM

#pragma comment(lib, "dinput8.lib")//动态加载dll留下的大坑，还是得link一下

#include "Input/f2dInputKeyboardImpl.h"

#include "Engine/f2dEngineImpl.h"
#include "Input/f2dInputSysImpl.h"

//////////////////////////////////////////////////////////////////////////////////////////

f2dInputKeyboardImpl::DefaultListener::DefaultListener(f2dInputSysImpl* pInputSys)
	: m_pEngine(pInputSys->GetEngine())
{}

void f2dInputKeyboardImpl::DefaultListener::OnKeyboardBtnDown(F2DINPUTKEYCODE KeyCode)
{
	m_pEngine->SendMsg(F2DMSG_KEYBOARD_ONKEYDOWN, KeyCode);
}

void f2dInputKeyboardImpl::DefaultListener::OnKeyboardBtnUp(F2DINPUTKEYCODE KeyCode)
{
	m_pEngine->SendMsg(F2DMSG_KEYBOARD_ONKEYUP, KeyCode);
}

//////////////////////////////////////////////////////////////////////////////////////////

const fuInt f2dInputKeyboardImpl::BufferSize = 32;

const DIDATAFORMAT f2dInputKeyboardImpl::DIDF_Keyboard =  {
	sizeof(DIDATAFORMAT),
	sizeof(DIOBJECTDATAFORMAT), 
	DIDF_RELAXIS,//相对轴模式
	256,
	256,//下面数组的尺寸
	(LPDIOBJECTDATAFORMAT)&f2dInputKeyboardImpl::DIODF_Keyboard
};

const DIOBJECTDATAFORMAT f2dInputKeyboardImpl::DIODF_Keyboard[256] = 
{
	{ &GUID_Key, 0, 2147483660, 0 },
	{ &GUID_Key, 1, 2147483916, 0 },
	{ &GUID_Key, 2, 2147484172, 0 },
	{ &GUID_Key, 3, 2147484428, 0 },
	{ &GUID_Key, 4, 2147484684, 0 },
	{ &GUID_Key, 5, 2147484940, 0 },
	{ &GUID_Key, 6, 2147485196, 0 },
	{ &GUID_Key, 7, 2147485452, 0 },
	{ &GUID_Key, 8, 2147485708, 0 },
	{ &GUID_Key, 9, 2147485964, 0 },
	{ &GUID_Key, 10, 2147486220, 0 },
	{ &GUID_Key, 11, 2147486476, 0 },
	{ &GUID_Key, 12, 2147486732, 0 },
	{ &GUID_Key, 13, 2147486988, 0 },
	{ &GUID_Key, 14, 2147487244, 0 },
	{ &GUID_Key, 15, 2147487500, 0 },
	{ &GUID_Key, 16, 2147487756, 0 },
	{ &GUID_Key, 17, 2147488012, 0 },
	{ &GUID_Key, 18, 2147488268, 0 },
	{ &GUID_Key, 19, 2147488524, 0 },
	{ &GUID_Key, 20, 2147488780, 0 },
	{ &GUID_Key, 21, 2147489036, 0 },
	{ &GUID_Key, 22, 2147489292, 0 },
	{ &GUID_Key, 23, 2147489548, 0 },
	{ &GUID_Key, 24, 2147489804, 0 },
	{ &GUID_Key, 25, 2147490060, 0 },
	{ &GUID_Key, 26, 2147490316, 0 },
	{ &GUID_Key, 27, 2147490572, 0 },
	{ &GUID_Key, 28, 2147490828, 0 },
	{ &GUID_Key, 29, 2147491084, 0 },
	{ &GUID_Key, 30, 2147491340, 0 },
	{ &GUID_Key, 31, 2147491596, 0 },
	{ &GUID_Key, 32, 2147491852, 0 },
	{ &GUID_Key, 33, 2147492108, 0 },
	{ &GUID_Key, 34, 2147492364, 0 },
	{ &GUID_Key, 35, 2147492620, 0 },
	{ &GUID_Key, 36, 2147492876, 0 },
	{ &GUID_Key, 37, 2147493132, 0 },
	{ &GUID_Key, 38, 2147493388, 0 },
	{ &GUID_Key, 39, 2147493644, 0 },
	{ &GUID_Key, 40, 2147493900, 0 },
	{ &GUID_Key, 41, 2147494156, 0 },
	{ &GUID_Key, 42, 2147494412, 0 },
	{ &GUID_Key, 43, 2147494668, 0 },
	{ &GUID_Key, 44, 2147494924, 0 },
	{ &GUID_Key, 45, 2147495180, 0 },
	{ &GUID_Key, 46, 2147495436, 0 },
	{ &GUID_Key, 47, 2147495692, 0 },
	{ &GUID_Key, 48, 2147495948, 0 },
	{ &GUID_Key, 49, 2147496204, 0 },
	{ &GUID_Key, 50, 2147496460, 0 },
	{ &GUID_Key, 51, 2147496716, 0 },
	{ &GUID_Key, 52, 2147496972, 0 },
	{ &GUID_Key, 53, 2147497228, 0 },
	{ &GUID_Key, 54, 2147497484, 0 },
	{ &GUID_Key, 55, 2147497740, 0 },
	{ &GUID_Key, 56, 2147497996, 0 },
	{ &GUID_Key, 57, 2147498252, 0 },
	{ &GUID_Key, 58, 2147498508, 0 },
	{ &GUID_Key, 59, 2147498764, 0 },
	{ &GUID_Key, 60, 2147499020, 0 },
	{ &GUID_Key, 61, 2147499276, 0 },
	{ &GUID_Key, 62, 2147499532, 0 },
	{ &GUID_Key, 63, 2147499788, 0 },
	{ &GUID_Key, 64, 2147500044, 0 },
	{ &GUID_Key, 65, 2147500300, 0 },
	{ &GUID_Key, 66, 2147500556, 0 },
	{ &GUID_Key, 67, 2147500812, 0 },
	{ &GUID_Key, 68, 2147501068, 0 },
	{ &GUID_Key, 69, 2147501324, 0 },
	{ &GUID_Key, 70, 2147501580, 0 },
	{ &GUID_Key, 71, 2147501836, 0 },
	{ &GUID_Key, 72, 2147502092, 0 },
	{ &GUID_Key, 73, 2147502348, 0 },
	{ &GUID_Key, 74, 2147502604, 0 },
	{ &GUID_Key, 75, 2147502860, 0 },
	{ &GUID_Key, 76, 2147503116, 0 },
	{ &GUID_Key, 77, 2147503372, 0 },
	{ &GUID_Key, 78, 2147503628, 0 },
	{ &GUID_Key, 79, 2147503884, 0 },
	{ &GUID_Key, 80, 2147504140, 0 },
	{ &GUID_Key, 81, 2147504396, 0 },
	{ &GUID_Key, 82, 2147504652, 0 },
	{ &GUID_Key, 83, 2147504908, 0 },
	{ &GUID_Key, 84, 2147505164, 0 },
	{ &GUID_Key, 85, 2147505420, 0 },
	{ &GUID_Key, 86, 2147505676, 0 },
	{ &GUID_Key, 87, 2147505932, 0 },
	{ &GUID_Key, 88, 2147506188, 0 },
	{ &GUID_Key, 89, 2147506444, 0 },
	{ &GUID_Key, 90, 2147506700, 0 },
	{ &GUID_Key, 91, 2147506956, 0 },
	{ &GUID_Key, 92, 2147507212, 0 },
	{ &GUID_Key, 93, 2147507468, 0 },
	{ &GUID_Key, 94, 2147507724, 0 },
	{ &GUID_Key, 95, 2147507980, 0 },
	{ &GUID_Key, 96, 2147508236, 0 },
	{ &GUID_Key, 97, 2147508492, 0 },
	{ &GUID_Key, 98, 2147508748, 0 },
	{ &GUID_Key, 99, 2147509004, 0 },
	{ &GUID_Key, 100, 2147509260, 0 },
	{ &GUID_Key, 101, 2147509516, 0 },
	{ &GUID_Key, 102, 2147509772, 0 },
	{ &GUID_Key, 103, 2147510028, 0 },
	{ &GUID_Key, 104, 2147510284, 0 },
	{ &GUID_Key, 105, 2147510540, 0 },
	{ &GUID_Key, 106, 2147510796, 0 },
	{ &GUID_Key, 107, 2147511052, 0 },
	{ &GUID_Key, 108, 2147511308, 0 },
	{ &GUID_Key, 109, 2147511564, 0 },
	{ &GUID_Key, 110, 2147511820, 0 },
	{ &GUID_Key, 111, 2147512076, 0 },
	{ &GUID_Key, 112, 2147512332, 0 },
	{ &GUID_Key, 113, 2147512588, 0 },
	{ &GUID_Key, 114, 2147512844, 0 },
	{ &GUID_Key, 115, 2147513100, 0 },
	{ &GUID_Key, 116, 2147513356, 0 },
	{ &GUID_Key, 117, 2147513612, 0 },
	{ &GUID_Key, 118, 2147513868, 0 },
	{ &GUID_Key, 119, 2147514124, 0 },
	{ &GUID_Key, 120, 2147514380, 0 },
	{ &GUID_Key, 121, 2147514636, 0 },
	{ &GUID_Key, 122, 2147514892, 0 },
	{ &GUID_Key, 123, 2147515148, 0 },
	{ &GUID_Key, 124, 2147515404, 0 },
	{ &GUID_Key, 125, 2147515660, 0 },
	{ &GUID_Key, 126, 2147515916, 0 },
	{ &GUID_Key, 127, 2147516172, 0 },
	{ &GUID_Key, 128, 2147516428, 0 },
	{ &GUID_Key, 129, 2147516684, 0 },
	{ &GUID_Key, 130, 2147516940, 0 },
	{ &GUID_Key, 131, 2147517196, 0 },
	{ &GUID_Key, 132, 2147517452, 0 },
	{ &GUID_Key, 133, 2147517708, 0 },
	{ &GUID_Key, 134, 2147517964, 0 },
	{ &GUID_Key, 135, 2147518220, 0 },
	{ &GUID_Key, 136, 2147518476, 0 },
	{ &GUID_Key, 137, 2147518732, 0 },
	{ &GUID_Key, 138, 2147518988, 0 },
	{ &GUID_Key, 139, 2147519244, 0 },
	{ &GUID_Key, 140, 2147519500, 0 },
	{ &GUID_Key, 141, 2147519756, 0 },
	{ &GUID_Key, 142, 2147520012, 0 },
	{ &GUID_Key, 143, 2147520268, 0 },
	{ &GUID_Key, 144, 2147520524, 0 },
	{ &GUID_Key, 145, 2147520780, 0 },
	{ &GUID_Key, 146, 2147521036, 0 },
	{ &GUID_Key, 147, 2147521292, 0 },
	{ &GUID_Key, 148, 2147521548, 0 },
	{ &GUID_Key, 149, 2147521804, 0 },
	{ &GUID_Key, 150, 2147522060, 0 },
	{ &GUID_Key, 151, 2147522316, 0 },
	{ &GUID_Key, 152, 2147522572, 0 },
	{ &GUID_Key, 153, 2147522828, 0 },
	{ &GUID_Key, 154, 2147523084, 0 },
	{ &GUID_Key, 155, 2147523340, 0 },
	{ &GUID_Key, 156, 2147523596, 0 },
	{ &GUID_Key, 157, 2147523852, 0 },
	{ &GUID_Key, 158, 2147524108, 0 },
	{ &GUID_Key, 159, 2147524364, 0 },
	{ &GUID_Key, 160, 2147524620, 0 },
	{ &GUID_Key, 161, 2147524876, 0 },
	{ &GUID_Key, 162, 2147525132, 0 },
	{ &GUID_Key, 163, 2147525388, 0 },
	{ &GUID_Key, 164, 2147525644, 0 },
	{ &GUID_Key, 165, 2147525900, 0 },
	{ &GUID_Key, 166, 2147526156, 0 },
	{ &GUID_Key, 167, 2147526412, 0 },
	{ &GUID_Key, 168, 2147526668, 0 },
	{ &GUID_Key, 169, 2147526924, 0 },
	{ &GUID_Key, 170, 2147527180, 0 },
	{ &GUID_Key, 171, 2147527436, 0 },
	{ &GUID_Key, 172, 2147527692, 0 },
	{ &GUID_Key, 173, 2147527948, 0 },
	{ &GUID_Key, 174, 2147528204, 0 },
	{ &GUID_Key, 175, 2147528460, 0 },
	{ &GUID_Key, 176, 2147528716, 0 },
	{ &GUID_Key, 177, 2147528972, 0 },
	{ &GUID_Key, 178, 2147529228, 0 },
	{ &GUID_Key, 179, 2147529484, 0 },
	{ &GUID_Key, 180, 2147529740, 0 },
	{ &GUID_Key, 181, 2147529996, 0 },
	{ &GUID_Key, 182, 2147530252, 0 },
	{ &GUID_Key, 183, 2147530508, 0 },
	{ &GUID_Key, 184, 2147530764, 0 },
	{ &GUID_Key, 185, 2147531020, 0 },
	{ &GUID_Key, 186, 2147531276, 0 },
	{ &GUID_Key, 187, 2147531532, 0 },
	{ &GUID_Key, 188, 2147531788, 0 },
	{ &GUID_Key, 189, 2147532044, 0 },
	{ &GUID_Key, 190, 2147532300, 0 },
	{ &GUID_Key, 191, 2147532556, 0 },
	{ &GUID_Key, 192, 2147532812, 0 },
	{ &GUID_Key, 193, 2147533068, 0 },
	{ &GUID_Key, 194, 2147533324, 0 },
	{ &GUID_Key, 195, 2147533580, 0 },
	{ &GUID_Key, 196, 2147533836, 0 },
	{ &GUID_Key, 197, 2147534092, 0 },
	{ &GUID_Key, 198, 2147534348, 0 },
	{ &GUID_Key, 199, 2147534604, 0 },
	{ &GUID_Key, 200, 2147534860, 0 },
	{ &GUID_Key, 201, 2147535116, 0 },
	{ &GUID_Key, 202, 2147535372, 0 },
	{ &GUID_Key, 203, 2147535628, 0 },
	{ &GUID_Key, 204, 2147535884, 0 },
	{ &GUID_Key, 205, 2147536140, 0 },
	{ &GUID_Key, 206, 2147536396, 0 },
	{ &GUID_Key, 207, 2147536652, 0 },
	{ &GUID_Key, 208, 2147536908, 0 },
	{ &GUID_Key, 209, 2147537164, 0 },
	{ &GUID_Key, 210, 2147537420, 0 },
	{ &GUID_Key, 211, 2147537676, 0 },
	{ &GUID_Key, 212, 2147537932, 0 },
	{ &GUID_Key, 213, 2147538188, 0 },
	{ &GUID_Key, 214, 2147538444, 0 },
	{ &GUID_Key, 215, 2147538700, 0 },
	{ &GUID_Key, 216, 2147538956, 0 },
	{ &GUID_Key, 217, 2147539212, 0 },
	{ &GUID_Key, 218, 2147539468, 0 },
	{ &GUID_Key, 219, 2147539724, 0 },
	{ &GUID_Key, 220, 2147539980, 0 },
	{ &GUID_Key, 221, 2147540236, 0 },
	{ &GUID_Key, 222, 2147540492, 0 },
	{ &GUID_Key, 223, 2147540748, 0 },
	{ &GUID_Key, 224, 2147541004, 0 },
	{ &GUID_Key, 225, 2147541260, 0 },
	{ &GUID_Key, 226, 2147541516, 0 },
	{ &GUID_Key, 227, 2147541772, 0 },
	{ &GUID_Key, 228, 2147542028, 0 },
	{ &GUID_Key, 229, 2147542284, 0 },
	{ &GUID_Key, 230, 2147542540, 0 },
	{ &GUID_Key, 231, 2147542796, 0 },
	{ &GUID_Key, 232, 2147543052, 0 },
	{ &GUID_Key, 233, 2147543308, 0 },
	{ &GUID_Key, 234, 2147543564, 0 },
	{ &GUID_Key, 235, 2147543820, 0 },
	{ &GUID_Key, 236, 2147544076, 0 },
	{ &GUID_Key, 237, 2147544332, 0 },
	{ &GUID_Key, 238, 2147544588, 0 },
	{ &GUID_Key, 239, 2147544844, 0 },
	{ &GUID_Key, 240, 2147545100, 0 },
	{ &GUID_Key, 241, 2147545356, 0 },
	{ &GUID_Key, 242, 2147545612, 0 },
	{ &GUID_Key, 243, 2147545868, 0 },
	{ &GUID_Key, 244, 2147546124, 0 },
	{ &GUID_Key, 245, 2147546380, 0 },
	{ &GUID_Key, 246, 2147546636, 0 },
	{ &GUID_Key, 247, 2147546892, 0 },
	{ &GUID_Key, 248, 2147547148, 0 },
	{ &GUID_Key, 249, 2147547404, 0 },
	{ &GUID_Key, 250, 2147547660, 0 },
	{ &GUID_Key, 251, 2147547916, 0 },
	{ &GUID_Key, 252, 2147548172, 0 },
	{ &GUID_Key, 253, 2147548428, 0 },
	{ &GUID_Key, 254, 2147548684, 0 },
	{ &GUID_Key, 255, 2147548940, 0 }
};

f2dInputKeyboardImpl::f2dInputKeyboardImpl(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus)
	: m_pSys(pSys), m_pDev(NULL), m_DefaultListener(pSys), m_pListener(&m_DefaultListener)
{
	IDirectInput8* pDev = pSys->GetHandle();

	memset(m_ButtonState, 0, sizeof(m_ButtonState));

	HRESULT tHR = pDev->CreateDevice(pGUID, &m_pDev, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "CreateDevice Failed.", tHR);
	
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
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetCooperativeLevel Failed.", tHR);
	}

    // 设置数据格式
	tHR = m_pDev->SetDataFormat(&DIDF_Keyboard);
	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetDataFormat Failed.", tHR);
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
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetProperty Failed.", tHR);
	}

	// 获得设备
	tHR = m_pDev->Acquire();

	// 注册
	m_pSys->RegisterDevice(this);
}

f2dInputKeyboardImpl::~f2dInputKeyboardImpl()
{
	if(m_pDev)
		m_pDev->Unacquire();

	// 取消注册
	m_pSys->UnregisterDevice(this);

	FCYSAFEKILL(m_pDev);
}

f2dInputMouse* f2dInputKeyboardImpl::ToMouse()
{
	return NULL;
}

f2dInputKeyboard* f2dInputKeyboardImpl::ToKeyboard()
{
	return this;
}

f2dInputJoystick* f2dInputKeyboardImpl::ToJoystick()
{
	return NULL;
}

fResult f2dInputKeyboardImpl::UpdateState()
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

	for (fuInt i = 0; i < tSize; ++i)
	{
		fuInt tKeyCode = tRawData[i].dwOfs;
		bool tKeyDown = (LOWORD(tRawData[i].dwData) & 0x80) != 0;
		if(tKeyCode>255)
			continue;
		m_ButtonState[tKeyCode] = tKeyDown;

		if(m_pListener)
			if(tKeyDown)
				m_pListener->OnKeyboardBtnDown((F2DINPUTKEYCODE)tKeyCode);
			else
				m_pListener->OnKeyboardBtnUp((F2DINPUTKEYCODE)tKeyCode);
	}

	return FCYERR_OK;
}

void f2dInputKeyboardImpl::ResetState() {
	memset(&m_ButtonState, 0, sizeof(m_ButtonState));
}

f2dInputKeyboardEventListener* f2dInputKeyboardImpl::GetListener()
{
	return m_pListener;
}

fResult f2dInputKeyboardImpl::SetListener(f2dInputKeyboardEventListener* pListener)
{
	m_pListener = pListener;
	return FCYERR_OK;
}

fBool f2dInputKeyboardImpl::IsKeyDown(F2DINPUTKEYCODE KeyCode)
{
	if(KeyCode>255)
		return false;
	return m_ButtonState[KeyCode];
}

//////////////////////////////////////////////////////////////////////////////////////////

f2dInputKeyboardImpl2::DefaultListener::DefaultListener(f2dInputSysImpl* pInputSys)
{
	m_pEngine = pInputSys->GetEngine();
}

void f2dInputKeyboardImpl2::DefaultListener::OnKeyboardBtnDown(F2DINPUTKEYCODE KeyCode)
{
	//m_pEngine->SendMsg(F2DMSG_KEYBOARD_ONKEYDOWN, KeyCode);
}

void f2dInputKeyboardImpl2::DefaultListener::OnKeyboardBtnUp(F2DINPUTKEYCODE KeyCode)
{
	//m_pEngine->SendMsg(F2DMSG_KEYBOARD_ONKEYUP, KeyCode);
}

//////////////////////////////////////////////////////////////////////////////////////////

const fuInt f2dInputKeyboardImpl2::BufferSize = 64;

f2dInputKeyboardImpl2::f2dInputKeyboardImpl2(f2dInputSysImpl* pSys, HWND Win, const GUID& pGUID, fBool bGlobalFocus)
	: m_pSys(pSys), m_pDev(NULL), m_DefaultListener(pSys), m_pListener(&m_DefaultListener)
{
	IDirectInput8* pDev = pSys->GetHandle();

	memset(m_ButtonState, 0, sizeof(m_ButtonState));

	HRESULT tHR = pDev->CreateDevice(pGUID, &m_pDev, NULL);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "CreateDevice Failed.", tHR);

	// 设置协作模式
	fuInt tFlag = DISCL_NONEXCLUSIVE;
	if (bGlobalFocus)
		tFlag |= DISCL_BACKGROUND;
	else
		tFlag |= DISCL_FOREGROUND;
	tHR = m_pDev->SetCooperativeLevel(Win, tFlag);
	if (FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetCooperativeLevel Failed.", tHR);
	}

	// 设置数据格式
	tHR = m_pDev->SetDataFormat(&c_dfDIKeyboard);//默认数据格式
	if (FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetDataFormat Failed.", tHR);
	}

	// 设置缓冲区
	DIPROPDWORD tBufferProperty;

	tBufferProperty.diph.dwSize = sizeof(DIPROPDWORD);
	tBufferProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	tBufferProperty.diph.dwObj = 0;
	tBufferProperty.diph.dwHow = DIPH_DEVICE;
	tBufferProperty.dwData = BufferSize;

	tHR = m_pDev->SetProperty(DIPROP_BUFFERSIZE, &tBufferProperty.diph);
	if (FAILED(tHR))
	{
		FCYSAFEKILL(m_pDev);
		throw fcyWin32COMException("f2dInputKeyboardImpl::f2dInputKeyboardImpl", "SetProperty Failed.", tHR);
	}

	// 获得设备
	tHR = m_pDev->Acquire();

	// 注册
	m_pSys->RegisterDevice(this);
}

f2dInputKeyboardImpl2::~f2dInputKeyboardImpl2()
{
	if (m_pDev)
		m_pDev->Unacquire();

	// 取消注册
	m_pSys->UnregisterDevice(this);

	FCYSAFEKILL(m_pDev);
}

f2dInputMouse* f2dInputKeyboardImpl2::ToMouse()
{
	return NULL;
}

f2dInputKeyboard* f2dInputKeyboardImpl2::ToKeyboard()
{
	return this;
}

f2dInputJoystick* f2dInputKeyboardImpl2::ToJoystick()
{
	return NULL;
}

fResult f2dInputKeyboardImpl2::UpdateState()
{
	DIDEVICEOBJECTDATA tRawData[BufferSize];
	DWORD tSize = BufferSize;

	HRESULT tHR = m_pDev->GetDeviceData(
		sizeof(DIDEVICEOBJECTDATA),
		tRawData,
		&tSize,
		0);

	if (FAILED(tHR))
	{
		tHR = m_pDev->Acquire();

		if (FAILED(tHR))
			return FCYERR_INTERNALERR;
		else
		{
			tSize = BufferSize;
			tHR = m_pDev->GetDeviceData(
				sizeof(DIDEVICEOBJECTDATA),
				tRawData,
				&tSize,
				0);
			if (FAILED(tHR))
				return FCYERR_INTERNALERR;
		}
	}

	for (fuInt i = 0; i < tSize; ++i)
	{
		fuInt tKeyCode = tRawData[i].dwOfs;
		bool tKeyDown = (LOWORD(tRawData[i].dwData) & 0x80) != 0;
		if (tKeyCode > 255)
			continue;
		m_ButtonState[tKeyCode] = tKeyDown;

		if (m_pListener)
			if (tKeyDown)
				m_pListener->OnKeyboardBtnDown((F2DINPUTKEYCODE)tKeyCode);
			else
				m_pListener->OnKeyboardBtnUp((F2DINPUTKEYCODE)tKeyCode);
	}

	return FCYERR_OK;
}

void f2dInputKeyboardImpl2::ResetState() {
	memset(&m_ButtonState, 0, sizeof(m_ButtonState));
}

f2dInputKeyboardEventListener* f2dInputKeyboardImpl2::GetListener()
{
	return m_pListener;
}

fResult f2dInputKeyboardImpl2::SetListener(f2dInputKeyboardEventListener* pListener)
{
	m_pListener = pListener;
	return FCYERR_OK;
}

fBool f2dInputKeyboardImpl2::KeyPress(DWORD KeyCode) {
	if (KeyCode > 255)
		return false;
	return m_ButtonState[KeyCode];
}

#endif
