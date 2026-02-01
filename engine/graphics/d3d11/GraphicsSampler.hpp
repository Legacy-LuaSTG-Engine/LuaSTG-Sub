#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class GraphicsSampler final :
        public implement::ReferenceCounted<IGraphicsSampler>,
        public IGraphicsDeviceEventListener {
    public:
        // IGraphicsSampler

        void* getNativeHandle() const noexcept override { return m_sampler.get(); }

        const GraphicsSamplerInfo* getInfo() const noexcept override { return &m_info; }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // GraphicsSampler

        GraphicsSampler();
        GraphicsSampler(GraphicsSampler const&) = delete;
        GraphicsSampler(GraphicsSampler&&) = delete;
        ~GraphicsSampler();

        GraphicsSampler& operator=(GraphicsSampler const&) = delete;
        GraphicsSampler& operator=(GraphicsSampler&&) = delete;

        bool initialize(IGraphicsDevice* device, const GraphicsSamplerInfo& info);
        bool createResource();

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11SamplerState> m_sampler;
        GraphicsSamplerInfo m_info{};
        bool m_initialized{ false };
    };
}
