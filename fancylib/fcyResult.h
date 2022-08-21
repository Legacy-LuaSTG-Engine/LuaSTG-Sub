#pragma once
#ifndef __FTYPE__H__
#include "fcyType.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// 错误
////////////////////////////////////////////////////////////////////////////////
/// @brief     判断fResult是否成功
/// @param[in] fr 要判断的返回值
#define FCYOK(fr)     (((fResult)(fr)) >= 0)

/// @brief     判断fResult是否失败
/// @param[in] fr 要判断的返回值
#define FCYFAILED(fr) (((fResult)(fr)) <  0)

/// @brief 生成一个fResult错误
/// @param[in] code 错误实体部分
#define FCYMAKEERR(code)           ((fResult) (0x80000000 | ((uint32_t)(code))))

/// @brief          生成一个fResult返回值
/// @param[in] sev  严重程度，0=成功，1=失败
/// @param[in] code 错误实体部分
#define FCYMAKERET(sev,code)       ((fResult) (((uint32_t)(sev)<<31) | ((uint32_t)(code))))

/// @brief          生成一个fResult返回值
/// @param[in] sev  严重程度，0=成功，1=失败
/// @param[in] rsv  保留值，默认取0
/// @param[in] desc 错误描述
#define FCYMAKERETEX(sev,rsv,desc) ((fResult) (((uint32_t)(sev)<<31) | ((uint32_t)(rsv)<<16) | ((uint32_t)(desc))))

/// @brief     返回错误描述
/// @param[in] fr 要处理的返回值
#define FCYRESULT_CODE(fr)     ((fr) & 0xFFFF)

/// @brief     返回错误保留值
/// @param[in] fr 要处理的返回值
#define FCYRESULT_RSV(fr)      (((fr) >> 16) & 0x7FFF)

/// @brief     返回错误严重程度
/// @param[in] fr 要处理的返回值
/// @return    0=成功， 1=失败
#define FCYRESULT_SEVERITY(fr) (((fr) >> 31) & 0x1)

////////////////////////////////////////////////////////////////////////////////
// 常见错误
////////////////////////////////////////////////////////////////////////////////
/// @addtogroup fancy常见返回值
/// @brief fancy库中的常见错误
/// @note  仅当FCYRESULT_RSV返回值为0时用来表示常见错误
/// @{

#define FCYERR_OK               ((fResult)0)   ///< @brief 正常返回值
#define FCYERR_UNKNOWN          FCYMAKEERR(0)  ///< @brief 未知返回值
#define FCYERR_INTERNALERR      FCYMAKEERR(1)  ///< @brief 内部错误
                                               ///< @details 通常表示API调用失败
#define FCYERR_ILLEGAL          FCYMAKEERR(2)  ///< @brief 无效调用
#define FCYERR_NOTIMPL          FCYMAKEERR(3)  ///< @brief 未实现
#define FCYERR_NOTSUPPORT       FCYMAKEERR(4)  ///< @brief 不支持
#define FCYERR_INVAILDDATA      FCYMAKEERR(5)  ///< @brief 无效数据
#define FCYERR_INVAILDPARAM     FCYMAKEERR(6)  ///< @brief 无效参数
#define FCYERR_INVAILDVERSION   FCYMAKEERR(7)  ///< @brief 无效版本
#define FCYERR_OBJNOTEXSIT      FCYMAKEERR(8)  ///< @brief 对象不存在
#define FCYERR_OBJEXSITED       FCYMAKEERR(9)  ///< @brief 对象已存在
#define FCYERR_OUTOFRANGE       FCYMAKEERR(10) ///< @brief 超出范围
                                               ///< @details 文件、数组访问到达结尾或越界
#define FCYERR_OUTOFMEM         FCYMAKEERR(11) ///< @brief 内存不足
