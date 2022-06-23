#pragma once

// 程序运行时调试
#include <crtdbg.h>

// 标准库
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <ctime>
#include <functional>
#include <algorithm>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <regex>
#include <array>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <numbers>

// 调试
#include "spdlog/spdlog.h"
#include "Tracy.hpp"

// 引擎公共头文件
#include "Config.h"          // 自定义编译配置
#include "LConfig.h"         // 引擎配置
#include "LMathConstant.hpp" // 常用数学常量
