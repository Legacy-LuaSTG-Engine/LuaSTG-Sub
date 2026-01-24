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

        double getDuration(const size_t index) const noexcept {
            const auto normalized_index = index % durations.size();
            return durations[(current_index + durations.size() - normalized_index) % durations.size()];
        }

        double getAverage(const size_t count) const noexcept {
            if (count == 0) {
                return 0.0;
            }
            double result = 0;
            const auto n = count < durations.size() ? count : durations.size();
            for (size_t i = 0; i < n; i += 1) {
                result += getDuration(i);
            }
            return result / static_cast<double>(n);
        }

        void reset() noexcept {
            durations.fill(0.0);
            duration_average = 0.0;
            duration_min = 0.0;
            duration_max = 0.0;
            duration_total = 0.0;
            frame_count = 0;
            current_index = 0u;
        }

        void update(const double duration) noexcept {
            durations[current_index % durations.size()] = duration;
            duration_average = 0;
            duration_min = 3600.0;
            duration_max = -3600.0;
            duration_total += duration;
            frame_count += 1;
            current_index = (current_index + 1) % durations.size();
            for (const auto v : durations) {
                duration_average += v;
                if (v < duration_min) duration_min = v;
                if (v > duration_max) duration_max = v;
            }
            const auto factor = 1.0 / static_cast<double>(durations.size());
            duration_average *= factor;
        }
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
