#pragma once
#ifndef __FTYPE__H__
#define __FTYPE__H__

#include <cstdint>

typedef char const*         fcStr;   // C式字符串常量
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

#define FCYSAFEKILL(x)   { if (x) { (x)->Release(); (x) = NULL; } }

#include "fcyResult.h"

#endif //__FTYPE__H__
