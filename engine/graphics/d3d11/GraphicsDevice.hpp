#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <vector>
#include "win32/base.hpp"
#include <d3d11_4.h>

namespace core {
    class GraphicsDevice final :
        public implement::ReferenceCounted<IGraphicsDevice>
    {
    public:
        // IGraphicsDevice

        void* getNativeDevice() override;

        void addEventListener(IGraphicsDeviceEventListener* listener) override;
        void removeEventListener(IGraphicsDeviceEventListener* listener) override;

        bool createVertexBuffer(uint32_t size_in_bytes, IVertexBuffer** output_buffer) override;
        bool createIndexBuffer(uint32_t size_in_bytes, IIndexBuffer** output_buffer) override;
        bool createConstantBuffer(uint32_t size_in_bytes, IConstantBuffer** output_buffer) override;

        // GraphicsDevice

        GraphicsDevice();
        GraphicsDevice(GraphicsDevice const&) = delete;
        GraphicsDevice(GraphicsDevice&&) = delete;
        GraphicsDevice& operator=(GraphicsDevice const&) = delete;
        GraphicsDevice& operator=(GraphicsDevice&&) = delete;
        ~GraphicsDevice();

    private:
        win32::com_ptr<ID3D11Device> m_device;
        win32::com_ptr<ID3D11DeviceContext> m_device_context;
        std::vector<IGraphicsDeviceEventListener*> m_event_listeners;
        bool m_initialized{};
    };
}
