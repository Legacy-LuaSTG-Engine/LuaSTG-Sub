#pragma once
#include "Core/Type.hpp"

namespace LuaSTG::Core
{
    struct IWindowEventListener
    {
        virtual void onCreate(void* window) {}
        virtual void onDestroy() {}
    };

    struct IApplicationEventListener
    {
        virtual void onUpdate() {}
    };

    struct IApplication : public IObject
    {
        virtual void addApplicationEventListener(IApplicationEventListener* e) = 0;
        virtual void removeApplicationEventListener(IApplicationEventListener * e) = 0;

        virtual void addWindowEventListener(IWindowEventListener* e) = 0;
        virtual void removeWindowEventListener(IWindowEventListener* e) = 0;
    };
}
