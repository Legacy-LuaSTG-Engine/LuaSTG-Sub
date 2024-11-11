#pragma once

// 调试库
#include <crtdbg.h>
#include <cassert>

// 数值与算法库
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <limits>
#include <numbers>
#include <random>

// 语言支持库
#include <memory>
#include <memory_resource>
#include <functional>

// 容器库
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

// 输入输出、文件系统库
#include <fstream>
#include <filesystem>

// 调试工具
#include "spdlog/spdlog.h"
#include "tracy/Tracy.hpp"

// 引擎公共头文件
#include "Config.h"          // 自定义编译配置
#include "LConfig.h"         // 引擎配置
#include "LMathConstant.hpp" // 常用数学常量
