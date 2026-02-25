#include "windows/FrameRateController.hpp"

namespace {
    void sleep(uint32_t ms);
    int64_t queryPerformanceFrequency();
    int64_t queryPerformanceCounter();
}

namespace core {
    // IFrameRateController

    bool FrameRateController::arrived() const noexcept {
        const int64_t target = m_baseline + m_frame_count * m_interval;
        const auto current = queryPerformanceCounter();
        return current >= target;
    }
    double FrameRateController::update() noexcept {
        // current
        const int64_t target = m_baseline + m_frame_count * m_interval;
        int64_t current = queryPerformanceCounter();

        // overflow
        if (current < m_last) {
            const auto fallback_duration = 1.0 / m_frame_rate;
            m_baseline = current;
            m_last = current;
            m_frame_count = 0;
            m_statistics.reset();
            m_statistics.update(fallback_duration);
            return fallback_duration;
        }

        // error reset
        if ((current - target) > (m_interval * 10)) {
            const auto duration = static_cast<double>(current - m_last) / static_cast<double>(m_frequency);
            m_baseline = current;
            m_last = current;
            m_frame_count = 0;
            m_statistics.reset();
            m_statistics.update(duration);
            return duration;
        }

        // inaccurate wait
        const auto sleep_error = m_frequency * 2 / 1000; // 2ms
        const auto sleep_ms = (target - sleep_error - current) * 1000 / m_frequency; // performance counter -> milliseconds
        if (sleep_ms > 0) {
            sleep(static_cast<uint32_t>(sleep_ms));
        }

        // accurate wait
        while (current < target) {
            current = queryPerformanceCounter();
        }

        // arrived
        const auto duration = static_cast<double>(current - m_last) / static_cast<double>(m_frequency);
        m_last = current;
        m_frame_count += 1;
        m_statistics.update(duration);
        return duration;
    }
    void FrameRateController::setFrameRate(const double frame_rate) noexcept {
        if (m_frame_rate == frame_rate) {
            return;
        }
        m_frame_rate = frame_rate > 1.0 ? frame_rate : 1.0;
        m_interval = static_cast<int64_t>(static_cast<double>(m_frequency) * (1.0 / frame_rate));
        m_baseline = queryPerformanceCounter();
        m_last = m_baseline;
        m_frame_count = 0;
    }

    // FrameRateController

    FrameRateController::FrameRateController(const double frame_rate) noexcept {
        m_frequency = queryPerformanceFrequency();
        setFrameRate(frame_rate);
        m_statistics.reset();
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
