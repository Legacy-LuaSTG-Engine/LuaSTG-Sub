#pragma once
#include "core/Graphics/Device.hpp"

namespace luastg {
    class FrameQuery {
    public:
        void fetchData();
        double getTime();
        void begin();
        void end();

        FrameQuery(core::Graphics::IDevice* device);
        ~FrameQuery();

    private:
        struct Impl;
        Impl* m_impl{};
    };
}
