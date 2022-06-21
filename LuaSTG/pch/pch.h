#pragma once

// 这里用来放置到处都会用到的，但是又不经常修改的头文件（比如C++标准库的或者第三方库的）
// 可以用来加速编译

// 日志系统
#include "spdlog/spdlog.h"

#include "Tracy.hpp"
//#include "TracyD3D11.hpp"

// 编译配置
#include "Config.h"  // 不同需求的定制配置
#include "LConfig.h" // 引擎配置
