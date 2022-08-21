#pragma once
#ifndef __FTYPE__H__
#define __FTYPE__H__

#include <cstdint>

typedef ptrdiff_t           fHandle; // 可储存指针或者句柄
typedef char*               fStr;    // C式字符串
typedef char const*         fcStr;   // C式字符串常量
typedef wchar_t*            fStrW;   // C式宽字符串
typedef wchar_t const*      fcStrW;  // C式宽字符串常量
typedef uint8_t*            fData;   // 内存数据指针
typedef uint8_t const*      fcData;  // 常量内存数据指针
typedef int32_t             fResult; // 预定义返回值
                                     // fResult 定义如下
                                     //   符号位  ：
                                     //     0 - 成功
                                     //     1 - 失败
                                     //   30-16 位：
                                     //     保留，置为0
                                     //   15-0  位：
                                     //     描述错误类型

#define FCYSAFEDEL(x)    { delete (x); (x) = NULL;   }
#define FCYSAFEDELARR(x) { delete[] (x); (x) = NULL; }
#define FCYSAFEKILL(x)   { if (x) { (x)->Release(); (x) = NULL; } }

#include "fcyResult.h"

#endif //__FTYPE__H__
