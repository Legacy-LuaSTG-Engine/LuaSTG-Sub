#include "d3d11/GraphicsDevice.hpp"
#include <cassert>
#include <ranges>
#include <algorithm>

namespace core {
    // IGraphicsDevice

    void* GraphicsDevice::getNativeDevice() {
        return m_device.get();
    }

    void GraphicsDevice::addEventListener(IGraphicsDeviceEventListener* const listener) {
        std::erase(m_event_listeners, listener);
        m_event_listeners.emplace_back(listener);
    }
    void GraphicsDevice::removeEventListener(IGraphicsDeviceEventListener* const listener) {
        std::erase(m_event_listeners, listener);
    }

    // GraphicsDevice

    GraphicsDevice::GraphicsDevice() = default;
    GraphicsDevice::~GraphicsDevice() = default;
}
