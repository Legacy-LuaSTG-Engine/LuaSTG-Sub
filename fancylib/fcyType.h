////////////////////////////////////////////////////////////////////////////////
/// @file  fcyType.h
/// @brief 描述fancylib中各种基础数据类型、定义通用的宏
////////////////////////////////////////////////////////////////////////////////
#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1800
#error Visual Stuidio earlier than 2013 is no long supported.
#else
#include <cstdint>
#endif

////////////////////////////////////////////////////////////////////////////////
// 基础数据类型
////////////////////////////////////////////////////////////////////////////////
/// @addtogroup fancy库基础数据类型
/// @brief fancy库中的基础数据类型定义
/// @{
typedef bool                fBool;   ///< @brief 逻辑型
typedef char                fChar;   ///< @brief 字符型
typedef wchar_t             fCharW;  ///< @brief 宽字符
typedef uint8_t             fByte;   ///< @brief 字节型
typedef int16_t             fShort;  ///< @brief 16位短整数
typedef uint16_t            fuShort; ///< @brief 16位无符号短整数
typedef int32_t             fInt;    ///< @brief 32位整数
typedef uint32_t            fuInt;   ///< @brief 32位无符号整数
typedef int64_t             fLong;   ///< @brief 64位长整数
typedef uint64_t            fuLong;  ///< @brief 64位无符号长整数
typedef float               fFloat;  ///< @brief 浮点型
typedef double              fDouble; ///< @brief 双精度浮点型
typedef fChar*              fStr;    ///< @brief C式字符串
typedef const fChar*        fcStr;   ///< @brief C式字符串常量
typedef fCharW*             fStrW;   ///< @brief C式宽字符串
typedef const fCharW*       fcStrW;  ///< @brief C式宽字符串常量
typedef fByte*              fData;   ///< @brief 内存数据指针
typedef const fByte*        fcData;  ///< @brief 常量内存数据指针
typedef fuLong              fLen;    ///< @brief 长度型
typedef fInt                fResult; ///< @brief 预定义返回值
                                     ///< @details
                                     /// fResult 定义如下 \n
                                     ///   符号位  ：     \n
                                     ///     0 - 成功     \n
                                     ///     1 - 失败     \n
                                     ///   30-16 位：     \n
                                     ///     保留，置为0  \n
                                     ///   15-0  位：     \n
                                     ///     描述错误类型
/// @}

////////////////////////////////////////////////////////////////////////////////
// 宏定义
////////////////////////////////////////////////////////////////////////////////
/// @addtogroup fancy库宏定义
/// @brief fancy库中的各种宏定义
/// @{

#ifndef NULL
/// @brief 空指针
#define NULL                0
#endif

/// @brief 强制内联宏
#define FCYINLINE           __forceinline

/// @brief     回收指针
/// @param[in] x 要回收的指针
#define FCYSAFEDEL(x)    { delete (x); (x) = NULL;   }

/// @brief     回收指针数组
/// @param[in] x 要回收的数组指针
#define FCYSAFEDELARR(x) { delete[] (x); (x) = NULL; }

/// @brief     回收引用计数接口
/// @param[in] x 要回收的接口
#define FCYSAFEKILL(x)   { if(x){ (x)->Release(); (x)=NULL; } }
/// @}

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
#define FCYMAKEERR(code)           ((fResult) (0x80000000 | ((fuInt)(code))))

/// @brief          生成一个fResult返回值
/// @param[in] sev  严重程度，0=成功，1=失败
/// @param[in] code 错误实体部分
#define FCYMAKERET(sev,code)       ((fResult) (((fuInt)(sev)<<31) | ((fuInt)(code))))

/// @brief          生成一个fResult返回值
/// @param[in] sev  严重程度，0=成功，1=失败
/// @param[in] rsv  保留值，默认取0
/// @param[in] desc 错误描述
#define FCYMAKERETEX(sev,rsv,desc) ((fResult) (((fuInt)(sev)<<31) | ((fuInt)(rsv)<<16) | ((fuInt)(desc))))

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

/// @}
