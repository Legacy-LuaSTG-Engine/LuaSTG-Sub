#pragma once
#include <cstdint>
#include <array>

namespace core {
    struct FrameRateStatistics {
        std::array<double, 240> durations{};
        double duration_average{};
        double duration_min{};
        double duration_max{};
        double duration_total{};
        uint64_t frame_count;
        size_t current_index{};

        double getDuration(size_t index) const noexcept;

        double getAverage(size_t count) const noexcept;

        void reset() noexcept;

        void update(double duration) noexcept;
    };

    struct IFrameRateController {
        virtual bool arrived() const noexcept = 0;
        virtual double update() noexcept = 0;
        virtual double getFrameRate() const noexcept = 0;
        virtual void setFrameRate(double frame_rate) noexcept = 0;
        virtual const FrameRateStatistics* getStatistics() const noexcept = 0;

        static IFrameRateController* getInstance();
    };

    class ScopeTimer {
    public:
        ScopeTimer(double* value = nullptr);
        ~ScopeTimer();

    private:
        double* m_value{};
        int64_t m_last{};
    };
}
