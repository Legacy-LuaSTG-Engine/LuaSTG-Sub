#pragma once
#include "core/Display.hpp"
#include "core/implement/ReferenceCounted.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace core {
    class Display final : public implement::ReferenceCounted<IDisplay> {
    public:
        // IDisplay

        void* getNativeHandle() override;
        void getFriendlyName(IImmutableString** output) override;
        Vector2U getSize() override;
        Vector2I getPosition() override;
        RectI getRect() override;
        Vector2U getWorkAreaSize() override;
        Vector2I getWorkAreaPosition() override;
        RectI getWorkAreaRect() override;
        bool isPrimary() override;
        float getDisplayScale() override;

        // Display

        explicit Display(HMONITOR monitor);
        Display(Display const&) = delete;
        Display(Display&&) = delete;
        ~Display() override;

        Display& operator=(Display const&) = delete;
        Display& operator=(Display&&) = delete;

    private:
        HMONITOR win32_monitor{};
    };
}
