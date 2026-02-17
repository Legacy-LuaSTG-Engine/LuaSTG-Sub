#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IApplication {
        virtual bool onCreate() = 0;
        virtual void onBeforeUpdate() = 0;
        virtual bool onUpdate() = 0;
        virtual void onDestroy() = 0;
    };

    class ApplicationManager {
    public:
        static void run(IApplication* application);
        static IApplication* getApplication();
        static void requestExit();
        static bool isMainThread();
        static bool isUpdating();
        static void runBeforeUpdate();
        static void runUpdate();
        static bool isDelegateUpdateEnabled();
        static void setDelegateUpdateEnabled(bool enabled);
    };
}
