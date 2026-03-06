#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"

namespace core {
    CORE_INTERFACE IWindow;

    CORE_INTERFACE IDisplay : IReferenceCounted {
        virtual void* getNativeHandle() = 0;
        virtual void getFriendlyName(IImmutableString** output) = 0;
        virtual Vector2U getSize() = 0;
        virtual Vector2I getPosition() = 0;
        virtual RectI getRect() = 0;
        virtual Vector2U getWorkAreaSize() = 0;
        virtual Vector2I getWorkAreaPosition() = 0;
        virtual RectI getWorkAreaRect() = 0;
        virtual bool isPrimary() = 0;
        virtual float getDisplayScale() = 0;

        static bool getAll(size_t* count, IDisplay** output);
        static bool getPrimary(IDisplay** output);
        static bool getNearestFromWindow(IWindow* window, IDisplay** output);
    };

    CORE_INTERFACE_ID(IDisplay, "9432a56d-e3d2-5173-b313-a9581b373155")
}
