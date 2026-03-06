#pragma once

#include <vector>
#include <string>
#include <format>

namespace core {
    struct LoggingBuffer {
        std::vector<std::string> error;
    };
}

#define L_ERROR(...) log.error.emplace_back(std::format(__VA_ARGS__))
