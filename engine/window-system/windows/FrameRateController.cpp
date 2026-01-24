#include "windows/FrameRateController.hpp"

namespace {
    void sleep(uint32_t ms);
    int64_t queryPerformanceFrequency();
    int64_t queryPerformanceCounter();
}

namespace core {
    // IFrameRateController

    bool FrameRateController::arrived() const noexcept {
        const auto current = queryPerformanceCounter();
        return current >= m_last + m_interval;
    }
    double FrameRateController::update() noexcept {
        // current
        int64_t current = queryPerformanceCounter();

        // overflow
        if (current < m_last) {
            const auto fallback_duration = 1.0 / m_frame_rate;
            m_last = current;
            m_statistics.reset();
            m_statistics.update(fallback_duration);
            return fallback_duration;
        }

        // inaccurate wait
        const auto threshold = m_last + m_interval - (m_frequency * 2 / 1000);
        while (current < threshold) {
            sleep(0);
            current = queryPerformanceCounter();
        }

        // accurate wait
        const auto finish_line = m_last + m_interval;
        while (current < finish_line) {
            current = queryPerformanceCounter();
        }

        // arrived
        const auto duration = static_cast<double>(current - m_last) / static_cast<double>(m_frequency);
        m_last = current;
        m_statistics.update(duration);
        return duration;
    }
    void FrameRateController::setFrameRate(double frame_rate) noexcept {
        m_frame_rate = frame_rate > 1.0 ? frame_rate : 1.0;
        m_interval = static_cast<int64_t>(static_cast<double>(m_frequency) * (1.0 / frame_rate));
    }

    // FrameRateController

    FrameRateController::FrameRateController(const double frame_rate) noexcept {
        m_frequency = queryPerformanceFrequency();
        m_last = queryPerformanceCounter();
        setFrameRate(frame_rate);
    }
}

namespace core {
    IFrameRateController* IFrameRateController::getInstance() {
        static FrameRateController instance;
        return &instance;
    }
}

namespace core {
    ScopeTimer::ScopeTimer(double* const value) : m_value(value) {
        m_last = queryPerformanceCounter();
    }
    ScopeTimer::~ScopeTimer() {
        if (m_value != nullptr) {
            const auto frequency = queryPerformanceFrequency();
            const auto current = queryPerformanceCounter();
            *m_value = static_cast<double>(current - m_last) / static_cast<double>(frequency);
        }
    }
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
    void sleep(const uint32_t ms) {
        Sleep(ms);
    }
    int64_t queryPerformanceFrequency() {
        LARGE_INTEGER value{};
        QueryPerformanceFrequency(&value);
        return value.QuadPart;
    }
    int64_t queryPerformanceCounter() {
        LARGE_INTEGER value{};
        QueryPerformanceCounter(&value);
        return value.QuadPart;
    }
}
