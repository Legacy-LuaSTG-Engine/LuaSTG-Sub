#pragma once
#include "Core/Type.hpp"

namespace LuaSTG::Core
{
    struct WindowEventListener
    {
        virtual void onCreate(void* window) {}
        virtual void onDestroy() {}
    };

    struct ApplicationEventListener
    {
        virtual void onUpdate() {}
    };

    struct IApplication : public IObject
    {
        virtual void addApplicationEventListener(ApplicationEventListener* e) = 0;
        virtual void removeApplicationEventListener(ApplicationEventListener * e) = 0;

        virtual void addWindowEventListener(WindowEventListener* e) = 0;
        virtual void removeWindowEventListener(WindowEventListener* e) = 0;
    };
}
