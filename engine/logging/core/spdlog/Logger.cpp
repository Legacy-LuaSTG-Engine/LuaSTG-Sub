#include "core/Logger.hpp"
#include "spdlog/spdlog.h"

namespace core {
	void Logger::info(std::string_view const& message) {
		spdlog::info(message);
	}
	void Logger::info(std::string const& message) {
		spdlog::info(message);
	}

	void Logger::warn(std::string_view const& message) {
		spdlog::warn(message);
	}
	void Logger::warn(std::string const& message) {
		spdlog::warn(message);
	}

	void Logger::error(std::string_view const& message) {
		spdlog::error(message);
	}
	void Logger::error(std::string const& message) {
		spdlog::error(message);
	}
}
