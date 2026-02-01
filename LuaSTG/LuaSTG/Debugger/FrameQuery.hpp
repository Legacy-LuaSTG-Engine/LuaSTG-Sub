#pragma once
#include "core/GraphicsDevice.hpp"

namespace luastg {
    class FrameQuery {
    public:
        void fetchData();
        double getTime();
        void begin();
        void end();

        FrameQuery(core::IGraphicsDevice* device);
        ~FrameQuery();

    private:
        struct Impl;
        Impl* m_impl{};
    };
}
