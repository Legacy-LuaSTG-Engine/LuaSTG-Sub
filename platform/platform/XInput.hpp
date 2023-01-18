#pragma once

namespace Platform::XInput
{
    constexpr int Null            = 0x0000;
    constexpr int Up              = 0x0001;
    constexpr int Down            = 0x0002;
    constexpr int Left            = 0x0004;
    constexpr int Right           = 0x0008;
    constexpr int Start           = 0x0010;
    constexpr int Back            = 0x0020;
    constexpr int LeftThumb       = 0x0040;
    constexpr int RightThumb      = 0x0080;
    constexpr int LeftShoulder    = 0x0100;
    constexpr int RightShoulder   = 0x0200;
    constexpr int A               = 0x1000;
    constexpr int B               = 0x2000;
    constexpr int X               = 0x4000;
    constexpr int Y               = 0x8000;

    void setEnable(bool state);
    bool isConnected(int index);
    int refresh();
    void update();

    bool getKeyState(int index, int key);
    float getLeftTrigger(int index);
    float getRightTrigger(int index);
    float getLeftThumbX(int index);
    float getLeftThumbY(int index);
    float getRightThumbX(int index);
    float getRightThumbY(int index);

    bool getKeyState(int key);
    float getLeftTrigger();
    float getRightTrigger();
    float getLeftThumbX();
    float getLeftThumbY();
    float getRightThumbX();
    float getRightThumbY();

    #ifdef XINPUT_DLL
    DWORD getState(DWORD dwUserIndex, XINPUT_STATE* pState);
    DWORD setState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
    DWORD getCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
    #endif
}
