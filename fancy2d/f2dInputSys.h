////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputSys.h
/// @brief fancy2D输入系统接口定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

/// @addtogroup f2d输入系统
/// @brief fancy2d引擎输入系统
/// @{

struct f2dInputMouse;
struct f2dInputKeyboard;
struct f2dInputJoystick;

////////////////////////////////////////////////////////////////////////////////
/// @brief 输入设备类型
////////////////////////////////////////////////////////////////////////////////
enum F2DINPUTDEVTYPE
{
	F2DINPUTDEVTYPE_MOUSE,    ///< @brief 鼠标类型
	F2DINPUTDEVTYPE_KEYBOARD, ///< @brief 键盘类型
	F2DINPUTDEVTYPE_GAMECTRL  ///< @brief 游戏控制器
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 输入设备接口
////////////////////////////////////////////////////////////////////////////////
struct f2dInputDevice : public f2dInterface
{
	virtual f2dInputMouse* ToMouse()=0;       ///< @brief 转换到鼠标设备（不增加引用计数）
	virtual f2dInputKeyboard* ToKeyboard()=0; ///< @brief 转换到键盘设备（不增加引用计数）
	virtual f2dInputJoystick* ToJoystick()=0; ///< @brief 转换到手柄设备（不增加引用计数）

	/// @brief 更新设备状态
	/// @note  该方法已由引擎自动调用。
	virtual fResult UpdateState()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 鼠标事件监听器
////////////////////////////////////////////////////////////////////////////////
struct f2dInputMouseEventListener
{
	/// @brief     鼠标X位置移动
	/// @param[in] Value 移动的相对值
	virtual void OnMouseMoveX(fInt Value) {}

	/// @brief     鼠标Y位置移动
	/// @param[in] Value 移动的相对值
	virtual void OnMouseMoveY(fInt Value) {}

	/// @brief     鼠标Z位置移动
	/// @param[in] Value 移动的相对值
	virtual void OnMouseMoveZ(fInt Value) {}

	/// @brief 鼠标左键按下
	virtual void OnMouseLBtnDown() {}

	/// @brief 鼠标右键按下
	virtual void OnMouseRBtnDown() {}

	/// @brief 鼠标中键按下
	virtual void OnMouseMBtnDown() {}

	/// @brief     鼠标额外按键按下
	/// @param[in] Index 索引，从0开始
	virtual void OnMouseAdditionBtnDown(fuInt Index) {}

	/// @brief 鼠标左键放开
	virtual void OnMouseLBtnUp() {}

	/// @brief 鼠标右键放开
	virtual void OnMouseRBtnUp() {}

	/// @brief 鼠标中键放开
	virtual void OnMouseMBtnUp() {}

	/// @brief     鼠标额外按键放开
	/// @param[in] Index 索引，从0开始
	virtual void OnMouseAdditionBtnUp(fuInt Index) {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 鼠标设备
////////////////////////////////////////////////////////////////////////////////
struct f2dInputMouse : public f2dInputDevice
{
	// --- 监听器 ---
	/// @brief 获得监听器
	virtual f2dInputMouseEventListener* GetListener()=0;

	/// @brief 设置监听器
	virtual fResult SetListener(f2dInputMouseEventListener* pListener)=0;
	
	// --- 状态查询 ---
	virtual fBool IsLeftBtnDown()=0;                ///< @brief 左键是否按下
	virtual fBool IsRightBtnDown()=0;               ///< @brief 右键是否按下
	virtual fBool IsMiddleBtnDown()=0;              ///< @brief 滚轮是否按下
	virtual fBool IsAdditionBtnDown(fuInt Index)=0; ///< @brief     额外按钮是否按下
	                                                ///< @param[in] Index 索引从0开始
	virtual fInt GetOffsetX()=0;  ///< @brief 距离上次查询累积的X轴位移
	virtual fInt GetOffsetY()=0;  ///< @brief 距离上次查询累积的Y轴位移
	virtual fInt GetOffsetZ()=0;  ///< @brief 距离上次查询累积的Z轴位移
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 键盘按键代码
////////////////////////////////////////////////////////////////////////////////
enum F2DINPUTKEYCODE
{
	F2DINPUTKEYCODE_UNKNOWN=0x00,      ///< @brief 未知
	F2DINPUTKEYCODE_ESCAPE=0x01,       ///< @brief ESC键
	F2DINPUTKEYCODE_1=0x02,            ///< @brief 主键盘1
	F2DINPUTKEYCODE_2=0x03,            ///< @brief 主键盘2
	F2DINPUTKEYCODE_3=0x04,            ///< @brief 主键盘3
	F2DINPUTKEYCODE_4=0x05,            ///< @brief 主键盘4
	F2DINPUTKEYCODE_5=0x06,            ///< @brief 主键盘5
	F2DINPUTKEYCODE_6=0x07,            ///< @brief 主键盘6
	F2DINPUTKEYCODE_7=0x08,            ///< @brief 主键盘7
	F2DINPUTKEYCODE_8=0x09,            ///< @brief 主键盘8
	F2DINPUTKEYCODE_9=0x0A,            ///< @brief 主键盘9
	F2DINPUTKEYCODE_0=0x0B,            ///< @brief 主键盘0
	F2DINPUTKEYCODE_MINUS=0x0C,        ///< @brief 主键盘-
	F2DINPUTKEYCODE_EQUALS=0x0D,       ///< @brief =
	F2DINPUTKEYCODE_BACK=0x0E,         ///< @brief 退格
	F2DINPUTKEYCODE_TAB=0x0F,          ///< @brief TAB
	F2DINPUTKEYCODE_Q=0x10,            ///< @brief Q
	F2DINPUTKEYCODE_W=0x11,            ///< @brief W
	F2DINPUTKEYCODE_E=0x12,            ///< @brief E
	F2DINPUTKEYCODE_R=0x13,            ///< @brief R
	F2DINPUTKEYCODE_T=0x14,            ///< @brief T
	F2DINPUTKEYCODE_Y=0x15,            ///< @brief Y
	F2DINPUTKEYCODE_U=0x16,            ///< @brief U
	F2DINPUTKEYCODE_I=0x17,            ///< @brief I
	F2DINPUTKEYCODE_O=0x18,            ///< @brief O
	F2DINPUTKEYCODE_P=0x19,            ///< @brief P
	F2DINPUTKEYCODE_LBRACKET=0x1A,     ///< @brief [
	F2DINPUTKEYCODE_RBRACKET=0x1B,     ///< @brief ]
	F2DINPUTKEYCODE_RETURN=0x1C,       ///< @brief 回车
	F2DINPUTKEYCODE_LCONTROL=0x1D,     ///< @brief 左Ctrl
	F2DINPUTKEYCODE_A=0x1E,            ///< @brief A
	F2DINPUTKEYCODE_S=0x1F,            ///< @brief S
	F2DINPUTKEYCODE_D=0x20,            ///< @brief D
	F2DINPUTKEYCODE_F=0x21,            ///< @brief F
	F2DINPUTKEYCODE_G=0x22,            ///< @brief G
	F2DINPUTKEYCODE_H=0x23,            ///< @brief H
	F2DINPUTKEYCODE_J=0x24,            ///< @brief J
	F2DINPUTKEYCODE_K=0x25,            ///< @brief K
	F2DINPUTKEYCODE_L=0x26,            ///< @brief L
	F2DINPUTKEYCODE_SEMICOLON=0x27,    ///< @brief ;
	F2DINPUTKEYCODE_APOSTROPHE=0x28,   ///< @brief '
	F2DINPUTKEYCODE_GRAVE=0x29,        ///< @brief `
	F2DINPUTKEYCODE_LSHIFT=0x2A,       ///< @brief 左shift
	F2DINPUTKEYCODE_BACKSLASH=0x2B,    ///< @brief 反斜杠
	F2DINPUTKEYCODE_Z=0x2C,            ///< @brief Z
	F2DINPUTKEYCODE_X=0x2D,            ///< @brief X
	F2DINPUTKEYCODE_C=0x2E,            ///< @brief C
	F2DINPUTKEYCODE_V=0x2F,            ///< @brief V
	F2DINPUTKEYCODE_B=0x30,            ///< @brief B
	F2DINPUTKEYCODE_N=0x31,            ///< @brief N
	F2DINPUTKEYCODE_M=0x32,            ///< @brief M
	F2DINPUTKEYCODE_COMMA=0x33,        ///< @brief 逗号
	F2DINPUTKEYCODE_PERIOD=0x34,       ///< @brief 句号
	F2DINPUTKEYCODE_SLASH=0x35,        ///< @brief /
	F2DINPUTKEYCODE_RSHIFT=0x36,       ///< @brief 右shift
	F2DINPUTKEYCODE_MULTIPLY=0x37,     ///< @brief 乘号
	F2DINPUTKEYCODE_LMENU=0x38,        ///< @brief 左alt
	F2DINPUTKEYCODE_SPACE=0x39,        ///< @brief 空格
	F2DINPUTKEYCODE_CAPITAL=0x3A,      ///< @brief 大写
	F2DINPUTKEYCODE_F1=0x3B,           ///< @brief F1
	F2DINPUTKEYCODE_F2=0x3C,           ///< @brief F2
	F2DINPUTKEYCODE_F3=0x3D,           ///< @brief F3
	F2DINPUTKEYCODE_F4=0x3E,           ///< @brief F4
	F2DINPUTKEYCODE_F5=0x3F,           ///< @brief F5
	F2DINPUTKEYCODE_F6=0x40,           ///< @brief F6
	F2DINPUTKEYCODE_F7=0x41,           ///< @brief F7
	F2DINPUTKEYCODE_F8=0x42,           ///< @brief F8
	F2DINPUTKEYCODE_F9=0x43,           ///< @brief F9
	F2DINPUTKEYCODE_F10=0x44,          ///< @brief F10
	F2DINPUTKEYCODE_NUMLOCK=0x45,      ///< @brief 数字锁
	F2DINPUTKEYCODE_SCROLL=0x46,       ///< @brief 滚动锁
	F2DINPUTKEYCODE_NUMPAD7=0x47,      ///< @brief 数字键盘7
	F2DINPUTKEYCODE_NUMPAD8=0x48,      ///< @brief 数字键盘8
	F2DINPUTKEYCODE_NUMPAD9=0x49,      ///< @brief 数字键盘9
	F2DINPUTKEYCODE_SUBTRACT=0x4A,     ///< @brief 数字键盘-
	F2DINPUTKEYCODE_NUMPAD4=0x4B,      ///< @brief 数字键盘4
	F2DINPUTKEYCODE_NUMPAD5=0x4C,      ///< @brief 数字键盘5
	F2DINPUTKEYCODE_NUMPAD6=0x4D,      ///< @brief 数字键盘6
	F2DINPUTKEYCODE_ADD=0x4E,          ///< @brief 数字键盘+
	F2DINPUTKEYCODE_NUMPAD1=0x4F,      ///< @brief 数字键盘1
	F2DINPUTKEYCODE_NUMPAD2=0x50,      ///< @brief 数字键盘2
	F2DINPUTKEYCODE_NUMPAD3=0x51,      ///< @brief 数字键盘3
	F2DINPUTKEYCODE_NUMPAD0=0x52,      ///< @brief 数字键盘0
	F2DINPUTKEYCODE_DECIMAL=0x53,      ///< @brief 数字键盘.
	F2DINPUTKEYCODE_OEM_102=0x56,      
	F2DINPUTKEYCODE_F11=0x57,          ///< @brief F11
	F2DINPUTKEYCODE_F12=0x58,          ///< @brief F12
	F2DINPUTKEYCODE_F13=0x64,          ///< @brief F13
	F2DINPUTKEYCODE_F14=0x65,          ///< @brief F14
	F2DINPUTKEYCODE_F15=0x66,          ///< @brief F15
	F2DINPUTKEYCODE_KANA=0x70,
	F2DINPUTKEYCODE_ABNT_C1=0x73,
	F2DINPUTKEYCODE_CONVERT=0x79,
	F2DINPUTKEYCODE_NOCONVERT=0x7B,
	F2DINPUTKEYCODE_YEN=0x7D,
	F2DINPUTKEYCODE_ABNT_C2=0x7E,
	F2DINPUTKEYCODE_NUMPADEQUALS=0x8D,
	F2DINPUTKEYCODE_PREVTRACK=0x90,
	F2DINPUTKEYCODE_AT=0x91,
	F2DINPUTKEYCODE_COLON=0x92,
	F2DINPUTKEYCODE_UNDERLINE=0x93,
	F2DINPUTKEYCODE_KANJI=0x94,
	F2DINPUTKEYCODE_STOP=0x95,
	F2DINPUTKEYCODE_AX=0x96,
	F2DINPUTKEYCODE_UNLABELED=0x97,
	F2DINPUTKEYCODE_NEXTTRACK=0x99,
	F2DINPUTKEYCODE_NUMPADENTER=0x9C,
	F2DINPUTKEYCODE_RCONTROL=0x9D,
	F2DINPUTKEYCODE_MUTE=0xA0,
	F2DINPUTKEYCODE_CALCULATOR=0xA1,
	F2DINPUTKEYCODE_PLAYPAUSE=0xA2,
	F2DINPUTKEYCODE_MEDIASTOP=0xA4,
	F2DINPUTKEYCODE_VOLUMEDOWN=0xAE,
	F2DINPUTKEYCODE_VOLUMEUP=0xB0,
	F2DINPUTKEYCODE_WEBHOME=0xB2,
	F2DINPUTKEYCODE_NUMPADCOMMA=0xB3,
	F2DINPUTKEYCODE_DIVIDE=0xB5,
	F2DINPUTKEYCODE_SYSRQ=0xB7,
	F2DINPUTKEYCODE_RMENU=0xB8,
	F2DINPUTKEYCODE_PAUSE=0xC5,
	F2DINPUTKEYCODE_HOME=0xC7,
	F2DINPUTKEYCODE_UP=0xC8,
	F2DINPUTKEYCODE_PRIOR=0xC9,
	F2DINPUTKEYCODE_LEFT=0xCB,
	F2DINPUTKEYCODE_RIGHT=0xCD,
	F2DINPUTKEYCODE_END=0xCF,
	F2DINPUTKEYCODE_DOWN=0xD0,
	F2DINPUTKEYCODE_NEXT=0xD1,
	F2DINPUTKEYCODE_INSERT=0xD2,
	F2DINPUTKEYCODE_DELETE=0xD3,
	F2DINPUTKEYCODE_LWIN=0xDB,
	F2DINPUTKEYCODE_RWIN=0xDC,
	F2DINPUTKEYCODE_APPS=0xDD,
	F2DINPUTKEYCODE_POWER=0xDE,
	F2DINPUTKEYCODE_SLEEP=0xDF,
	F2DINPUTKEYCODE_WAKE=0xE3,
	F2DINPUTKEYCODE_WEBSEARCH=0xE5,
	F2DINPUTKEYCODE_WEBFAVORITES=0xE6,
	F2DINPUTKEYCODE_WEBREFRESH=0xE7,
	F2DINPUTKEYCODE_WEBSTOP=0xE8,
	F2DINPUTKEYCODE_WEBFORWARD=0xE9,
	F2DINPUTKEYCODE_WEBBACK=0xEA,
	F2DINPUTKEYCODE_MYCOMPUTER=0xEB,
	F2DINPUTKEYCODE_MAIL=0xEC,
	F2DINPUTKEYCODE_MEDIASELECT=0xED
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 键盘事件监听器
////////////////////////////////////////////////////////////////////////////////
struct f2dInputKeyboardEventListener
{
	/// @brief     键盘按键按下
	/// @param[in] KeyCode 按键代码
	virtual void OnKeyboardBtnDown(F2DINPUTKEYCODE KeyCode) {}

	/// @brief     键盘按键放开
	/// @param[in] KeyCode 按键代码
	virtual void OnKeyboardBtnUp(F2DINPUTKEYCODE KeyCode) {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 键盘设备
////////////////////////////////////////////////////////////////////////////////
struct f2dInputKeyboard : public f2dInputDevice
{
	// --- 监听器 ---

	/// @brief 返回监听器
	virtual f2dInputKeyboardEventListener* GetListener()=0;
	/// @brief 设置监听器
	virtual fResult SetListener(f2dInputKeyboardEventListener* pListener)=0;
	
	// --- 状态查询 ---

	//判断按键是否按下
	//param[in] KeyCode Fancy2D键代码
	virtual fBool IsKeyDown(F2DINPUTKEYCODE KeyCode)=0;

	//判断按键是否按下
	//param[in] KeyCode Dinput键代码
	virtual fBool KeyPress(DWORD KeyCode) = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 手柄事件监听器
////////////////////////////////////////////////////////////////////////////////
struct f2dInputJoystickEventListener
{
	/// @brief     按钮按下
	/// @param[in] p     手柄指针
	/// @param[in] Index 按键索引
	virtual void OnJoystickBtnDown(f2dInputJoystick* p, fuInt Index) {}

	/// @brief     按钮放开
	/// @param[in] p     手柄指针
	/// @param[in] Index 按键索引
	virtual void OnJoystickBtnUp(f2dInputJoystick* p, fuInt Index) {}

	/// @brief     X轴位置变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickXAxisPosChange(f2dInputJoystick* p, fFloat Value) {}

	/// @brief     Y轴位置变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickYAxisPosChange(f2dInputJoystick* p, fFloat Value) {}

	/// @brief     Z轴位置变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickZAxisPosChange(f2dInputJoystick* p, fFloat Value) {}

	/// @brief     X轴旋转变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickXAxisRotationChange(f2dInputJoystick* p, fFloat Value) {}

	/// @brief     Y轴旋转变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickYAxisRotationChange(f2dInputJoystick* p, fFloat Value) {}

	/// @brief     Z轴旋转变化
	/// @param[in] p     手柄指针
	/// @param[in] Value 新的位置
	virtual void OnJoystickZAxisRotationChange(f2dInputJoystick* p, fFloat Value) {}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 手柄设备
////////////////////////////////////////////////////////////////////////////////
struct f2dInputJoystick : public f2dInputDevice
{
	// --- 监听器 ---
	/// @brief 返回监听器
	virtual f2dInputJoystickEventListener* GetListener()=0;
	/// @brief 设置监听器
	virtual fResult SetListener(f2dInputJoystickEventListener* pListener)=0;

	// --- 状态查询 ---
	virtual fFloat GetXPosition()=0;  ///< @brief 手柄X位移，返回[-1,1]
	virtual fFloat GetYPosition()=0;  ///< @brief 手柄Y位移，返回[-1,1]
	virtual fFloat GetZPosition()=0;  ///< @brief 手柄Z位移，返回[-1,1]
	virtual fFloat GetXRotation()=0;  ///< @brief 手柄X旋转，返回[-1,1]
	virtual fFloat GetYRotation()=0;  ///< @brief 手柄Y旋转，返回[-1,1]
	virtual fFloat GetZRotation()=0;  ///< @brief 手柄Z旋转，返回[-1,1]
	virtual fBool IsButtonDown(fuInt Index)=0;  ///< @brief 检查按钮是否按下
	virtual fInt GetSlider(fuInt Index)=0;      ///< @brief 获得滑条值
	virtual fuInt GetPOV(fuInt Index)=0;        ///< @brief 获得POV值

	virtual fFloat GetDeadZone()=0;                  ///< @brief 返回死区，[0,1]之间值
	virtual fResult SetDeadZone(fFloat Percent)=0;   ///< @brief 设置死区，[0,1]之间值
	                                                 ///< @note  设置手柄最小响应灵敏度
	virtual fFloat GetSaturation()=0;                ///< @brief 返回饱和值，[0,1]之间值
	virtual fResult SetSaturation(fFloat Percent)=0; ///< @brief 设置饱和值，[0,1]之间值
	                                                 ///< @note  设置手柄最大响应灵敏度
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D输入系统
////////////////////////////////////////////////////////////////////////////////
struct f2dInputSys
{
	/// @brief     返回设备总数
	/// @param[in] Type 设备类型
	virtual fuInt GetDeviceCount(F2DINPUTDEVTYPE Type)=0;

	/// @brief     返回设备名称
	/// @param[in] Type  设备类型
	/// @param[in] Index 设备索引
	virtual fcStrW GetDeviceName(F2DINPUTDEVTYPE Type, fuInt Index)=0;

	/// @brief     返回设备产品名称
	/// @param[in] Type  设备类型
	/// @param[in] Index 设备索引
	virtual fcStrW GetDeviceProductName(F2DINPUTDEVTYPE Type, fuInt Index)=0;
	
	/// @brief      创建鼠标
	/// @param[in]  DevIndex     创建默认鼠标设备时设为-1
	/// @param[in]  bGlobalFocus 全局响应，当为true时程序在非激活状态时亦可捕捉消息
	/// @param[out] pOut         创建的设备指针
	virtual fResult CreateMouse(fInt DevIndex, fBool bGlobalFocus, f2dInputMouse** pOut)=0;

	/// @brief      创建键盘
	/// @param[in]  DevIndex     创建默认鼠标设备时设为-1
	/// @param[in]  bGlobalFocus 全局响应，当为true时程序在非激活状态时亦可捕捉消息
	/// @param[out] pOut         创建的设备指针
	virtual fResult CreateKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut)=0;

	//创建默认类型的键盘，不使用Fancy2D的键盘映射
	//param[in]  DevIndex     创建默认键盘设备时设为-1
	//param[in]  bGlobalFocus 全局响应，当为true时程序在非激活状态时亦可捕捉消息
	//param[out] pOut         创建的设备指针
	virtual fResult CreateDefaultKeyboard(fInt DevIndex, fBool bGlobalFocus, f2dInputKeyboard** pOut) = 0;

	/// @brief      创建键盘
	/// @param[in]  DevIndex     从0开始，可以由GetDeviceCount枚举
	/// @param[in]  bGlobalFocus 全局响应，当为true时程序在非激活状态时亦可捕捉消息
	/// @param[out] pOut         创建的设备指针
	virtual fResult CreateJoystick(fInt DevIndex, fBool bGlobalFocus, f2dInputJoystick** pOut)=0;
};

/// @}
