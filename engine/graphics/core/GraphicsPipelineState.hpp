#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    enum class GraphicsCullMode : uint8_t {
        none,
        front,
        back,
    };

    enum class GraphicsFrontFace : uint8_t {
        clockwise,
        counter_clockwise,
    };

    struct GraphicsRasterizerState {
        float depth_bias_constant_factor{};
        float depth_bias_clamp{};
        float depth_bias_slope_factor{};
        GraphicsCullMode cull_mode{};
        GraphicsFrontFace front_face{};
    };
}
