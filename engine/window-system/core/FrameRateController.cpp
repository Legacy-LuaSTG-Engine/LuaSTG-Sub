#include "core/FrameRateController.hpp"

namespace core {
    double FrameRateStatistics::getDuration(const size_t index) const noexcept {
        const auto normalized_index = index % durations.size();
        return durations[(current_index + durations.size() - normalized_index) % durations.size()];
    }

    double FrameRateStatistics::getAverage(const size_t count) const noexcept {
        if (count == 0 || frame_count == 0) {
            return 0.0;
        }
        double result = 0;
        const auto m = count < frame_count ? count : static_cast<size_t>(frame_count);
        const auto n = m < durations.size() ? m : durations.size();
        size_t l{};
        for (size_t i = 0; i < n; i += 1) {
            const auto t = getDuration(i);
            if (t >= 0.0) {
                result += t;
                l += 1;
            }
        }
        return result / static_cast<double>(l);
    }

    void FrameRateStatistics::reset() noexcept {
        durations.fill(-1.0);
        duration_average = 0.0;
        duration_min = 0.0;
        duration_max = 0.0;
        duration_total = 0.0;
        frame_count = 0;
        current_index = 0u;
    }

    void FrameRateStatistics::update(const double duration) noexcept {
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
}
