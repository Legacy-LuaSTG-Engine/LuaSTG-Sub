////////////////////////////////////////////////////////////////////////////////
/// @file  f2d.h
/// @brief fancy2D头文件
/// @note  定义了导出函数，包含了所有必需的头文件
////////////////////////////////////////////////////////////////////////////////
#pragma once

// 导出函数定义
#ifdef F2DDLL
	#define F2DEXTERNC extern "C"
	#ifdef F2DEXPORT
		#define F2DDLLFUNC __declspec(dllexport)
	#else
		#define F2DDLLFUNC __declspec(dllimport)
	#endif
#else
	#define F2DEXTERNC
	#define F2DDLLFUNC
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

F2DEXTERNC fResult F2DDLLFUNC CreateF2DEngineAndInit(
	fuInt Version, 
	f2dEngineRenderWindowParam* RenderWindowParam,
	f2dEngine** pOut,
	f2dInitialErrListener* pErrListener = NULL);

/// @}
