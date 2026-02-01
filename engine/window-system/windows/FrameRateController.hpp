#pragma once
#include "core/FrameRateController.hpp"
#include <array>

namespace core {
    class FrameRateController : public IFrameRateController {
    public:
        // IFrameRateController

        bool arrived() const noexcept override;
        double update() noexcept override;
        double getFrameRate() const noexcept override { return m_frame_rate; }
        void setFrameRate(double frame_rate) noexcept override;
        const FrameRateStatistics* getStatistics() const noexcept override { return &m_statistics; }

        // FrameRateController

        FrameRateController(double frame_rate = 60.0) noexcept;

    private:
        double m_frame_rate{ 60.0 };
        int64_t m_frequency{};
        int64_t m_interval{};
        int64_t m_last{};
        FrameRateStatistics m_statistics;
    };
}
