#pragma once
#ifndef __FTYPE__H__
#define __FTYPE__H__

#include <cstdint>

typedef bool                fBool;   // 逻辑型
typedef char                fChar;   // 字符型
typedef wchar_t             fCharW;  // 宽字符
typedef uint8_t             fByte;   // 字节型
typedef int16_t             fShort;  // 16位短整数
typedef uint16_t            fuShort; // 16位无符号短整数
typedef int32_t             fInt;    // 32位整数
typedef uint32_t            fuInt;   // 32位无符号整数
typedef int64_t             fLong;   // 64位长整数
typedef uint64_t            fuLong;  // 64位无符号长整数
typedef float               fFloat;  // 浮点型
typedef double              fDouble; // 双精度浮点型
typedef ptrdiff_t           fHandle; // 可储存指针或者句柄
typedef fChar*              fStr;    // C式字符串
typedef const fChar*        fcStr;   // C式字符串常量
typedef fCharW*             fStrW;   // C式宽字符串
typedef const fCharW*       fcStrW;  // C式宽字符串常量
typedef fByte*              fData;   // 内存数据指针
typedef const fByte*        fcData;  // 常量内存数据指针
typedef fuLong              fLen;    // 长度型
typedef fInt                fResult; // 预定义返回值
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
