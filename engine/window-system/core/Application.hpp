#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IApplication : IReferenceCounted {
        virtual bool onCreate() = 0;
        virtual bool onUpdate() = 0;
        virtual void onDestroy() = 0;
    };

    class ApplicationManager {
    public:
        static void run(IApplication* application);
    };
}
