////////////////////////////////////////////////////////////////////////////////
/// @file  fcyDebug.h
/// @brief fancy调试辅助库
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyType.h"

/// @addtogroup fancy库底层支持
/// @{

/// @brief 调试用异常
#ifdef _DEBUG
#define FCYDEBUGEXCPT(e)                                          \
{                                                                 \
	fcyDebug::Trace("[ TimeTick = %u ] fcyException @%s : %s\n",  \
		e.GetTime(), e.GetSrc(), e.GetDesc());                    \
}                                                                 
#else
#define FCYDEBUGEXCPT(e) { e; }
#endif

/// @brief fcy调试辅助库
namespace fcyDebug
{
    /// @brief     输出调试文本
	/// @note      使用格式化输出在输出窗口输出调试文本
	/// @param[in] Format 格式化文本
	/// @param[in] ...    不定参数
	void Trace(fcStr Format, ...);

	/// @brief     输出调试文本，宽字符版本
	/// @note      使用格式化输出在输出窗口输出调试文本
	/// @param[in] Format 格式化文本
	/// @param[in] ...    不定参数
	void Trace(fcStrW Format, ...);
};

/// @}