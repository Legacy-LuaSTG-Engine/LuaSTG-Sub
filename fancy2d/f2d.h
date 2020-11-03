////////////////////////////////////////////////////////////////////////////////
/// @file  f2d.h
/// @brief fancy2D头文件
/// @note  定义了导出函数，包含了所有必需的头文件
////////////////////////////////////////////////////////////////////////////////
#pragma once

// 导出函数定义
#ifdef F2DEXPORT
#define F2DDLLFUNC __declspec(dllexport)
#else
/// @brief F2DDLL导入函数
#define F2DDLLFUNC __declspec(dllimport)
#endif

#include "f2dEngine.h"

/// @addtogroup f2d引擎
/// @{

/// @brief F2D版本 0.6
#define F2DVERSION ((fuInt)(0<<16 | 6))

/// @brief 初始化错误接受器
struct f2dInitialErrListener
{
	/// @brief     错误消息
	/// @param[in] TimeTick 时间戳
	/// @param[in] Src      错误源
	/// @param[in] Desc     错误描述
	virtual void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)=0;
};

/// @brief      创建fancy2D引擎
/// @param[in]  Version              必须为F2DVERSION，用于检查DLL版本
/// @param[in]  pListener            可空，用来设置引擎监听器。
/// @param[out] pOut                 输出的引擎指针
/// @param[in]  pErrListener         可空，初始化错误回调。
extern "C" fResult F2DDLLFUNC CreateF2DEngine(
	fuInt Version,
	f2dEngineEventListener* pListener,
	f2dEngine** pOut,
	f2dInitialErrListener* pErrListener = NULL);

/// @brief      创建fancy2D引擎并初始化所有组件
/// @param[in]  Version              必须为F2DVERSION，用于检查DLL版本
/// @param[in]  WinPos               窗口用户区位置
/// @param[in]  Title                窗口标题
/// @param[in]  Windowed             是否窗口化
/// @param[in]  VSync                是否垂直同步
/// @param[in]  AA                   指定抗锯齿等级
/// @param[in]  pListener            可空，用来设置引擎监听器。
/// @param[out] pOut                 输出的引擎指针
/// @param[in]  pErrListener         可空，初始化错误回调。
extern "C" fResult F2DDLLFUNC CreateF2DEngineAndInit(
	fuInt Version, 
	const fcyRect& WinPos, 
	fcStrW Title, 
	fBool Windowed, 
	fBool VSync,
	F2DAALEVEL AA,
	f2dEngineEventListener* pListener,
	f2dEngine** pOut,
	f2dInitialErrListener* pErrListener = NULL);

/// @}
