#pragma once
#include "core/ImmutableString.hpp"

#ifdef log
#undef log
#endif

namespace core {
    enum class GraphicsSystemFeature {
        windows_dxgi_present_allow_tearing,
    };

    class GraphicsDeviceManager {
    public:
        static bool refresh();
        static bool isFeatureSupported(GraphicsSystemFeature feature);
        static uint32_t getPhysicalDeviceCount();
        static StringView getPhysicalDeviceName(uint32_t index);
        static void log();
    };
}
