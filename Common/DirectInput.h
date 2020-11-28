#pragma once
#include <cstdint>

namespace native
{
    class DirectInput
    {
    public:
        struct AxisRange
        {
            int32_t XMin = 0, YMin = 0, ZMin = 0;
            int32_t XMax = 0, YMax = 0, ZMax = 0;
            int32_t RxMin = 0, RyMin = 0, RzMin = 0;
            int32_t RxMax = 0, RyMax = 0, RzMax = 0;
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
        uint32_t count();
        uint32_t refresh();
        void update();
        void reset();
        void clear();
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
