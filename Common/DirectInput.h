#pragma once
#include <cstdint>

namespace native
{
    class DirectInput
    {
    public:
        struct AxisRange
        {
            int32_t  XMin,  YMin,  ZMin;
            int32_t  XMax,  YMax,  ZMax;
            int32_t RxMin, RyMin, RzMin;
            int32_t RxMax, RyMax, RzMax;
            int32_t Slider0Min, Slider1Min;
            int32_t Slider0Max, Slider1Max;
        };
        struct RawState {
            int32_t  lX;
            int32_t  lY;
            int32_t  lZ;
            int32_t  lRx;
            int32_t  lRy;
            int32_t  lRz;
            int32_t  rglSlider[2];
            uint32_t rgdwPOV[4];
            uint8_t  rgbButtons[32];
        };
        struct State
        {
            uint16_t wButtons;
            uint8_t  bLeftTrigger;
            uint8_t  bRightTrigger;
            int16_t  sThumbLX;
            int16_t  sThumbLY;
            int16_t  sThumbRX;
            int16_t  sThumbRY;
        };
    private:
        struct _Data;
        _Data* _data;
    public:
        uint32_t refresh();
        void update();
        void reset();
        void clear();
    public: // Keyboard & Mouse
        bool getKeyboardKeyState(int32_t code);
        bool getMouseKeyState(int32_t code);
        int32_t getMouseMoveDeltaX();
        int32_t getMouseMoveDeltaY();
        int32_t getMouseWheelDelta();
    public: // Game Controller
        uint32_t count();
        bool getAxisRange(uint32_t index, AxisRange* range);
        bool getRawState(uint32_t index, RawState* state);
        bool getState(uint32_t index, State* state);
        const wchar_t* getDeviceName(uint32_t index);
        const wchar_t* getProductName(uint32_t index);
        bool isXInputDevice(uint32_t index);
    public:
        bool updateTargetWindow(ptrdiff_t window);
    public:
        DirectInput(ptrdiff_t window);
        ~DirectInput();
    };
};
