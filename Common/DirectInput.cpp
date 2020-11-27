#include "Common/DirectInput.h"

#include <vector>

#include <Windows.h>
#include <wrl.h>
#include <Xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Common/DebugLog.hpp"

// MAGIC CODE
BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput);

namespace native
{
    #define G_DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const GUID name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
    G_DEFINE_GUID(g_GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_RxAxis,  0xA36D02F4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_RyAxis,  0xA36D02F5,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_RzAxis,  0xA36D02E3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_Slider,  0xA36D02E4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_Button,  0xA36D02F0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_Key,     0x55728220,0xD33C,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_POV,     0xA36D02F2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    G_DEFINE_GUID(g_GUID_Unknown, 0xA36D02F3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
    #undef G_DEFINE_GUID
    
    constexpr DWORD g_dwControllerBufferSize = 64;
    static DIOBJECTDATAFORMAT g_dfDIJoystickObject[44] = {
        { &g_GUID_XAxis ,  0, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_YAxis ,  4, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_ZAxis ,  8, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_RxAxis, 12, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_RyAxis, 16, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_RzAxis, 20, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_Slider, 24, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_Slider, 28, 0x80FFFF03, DIDOI_ASPECTPOSITION },
        { &g_GUID_POV   , 32, 0x80FFFF10,                    0 },
        { &g_GUID_POV   , 36, 0x80FFFF10,                    0 },
        { &g_GUID_POV   , 40, 0x80FFFF10,                    0 },
        { &g_GUID_POV   , 44, 0x80FFFF10,                    0 },
        { NULL          , 48, 0x80FFFF0C,                    0 },
        { NULL          , 49, 0x80FFFF0C,                    0 },
        { NULL          , 50, 0x80FFFF0C,                    0 },
        { NULL          , 51, 0x80FFFF0C,                    0 },
        { NULL          , 52, 0x80FFFF0C,                    0 },
        { NULL          , 53, 0x80FFFF0C,                    0 },
        { NULL          , 54, 0x80FFFF0C,                    0 },
        { NULL          , 55, 0x80FFFF0C,                    0 },
        { NULL          , 56, 0x80FFFF0C,                    0 },
        { NULL          , 57, 0x80FFFF0C,                    0 },
        { NULL          , 58, 0x80FFFF0C,                    0 },
        { NULL          , 59, 0x80FFFF0C,                    0 },
        { NULL          , 60, 0x80FFFF0C,                    0 },
        { NULL          , 61, 0x80FFFF0C,                    0 },
        { NULL          , 62, 0x80FFFF0C,                    0 },
        { NULL          , 63, 0x80FFFF0C,                    0 },
        { NULL          , 64, 0x80FFFF0C,                    0 },
        { NULL          , 65, 0x80FFFF0C,                    0 },
        { NULL          , 66, 0x80FFFF0C,                    0 },
        { NULL          , 67, 0x80FFFF0C,                    0 },
        { NULL          , 68, 0x80FFFF0C,                    0 },
        { NULL          , 69, 0x80FFFF0C,                    0 },
        { NULL          , 70, 0x80FFFF0C,                    0 },
        { NULL          , 71, 0x80FFFF0C,                    0 },
        { NULL          , 72, 0x80FFFF0C,                    0 },
        { NULL          , 73, 0x80FFFF0C,                    0 },
        { NULL          , 74, 0x80FFFF0C,                    0 },
        { NULL          , 75, 0x80FFFF0C,                    0 },
        { NULL          , 76, 0x80FFFF0C,                    0 },
        { NULL          , 77, 0x80FFFF0C,                    0 },
        { NULL          , 78, 0x80FFFF0C,                    0 },
        { NULL          , 79, 0x80FFFF0C,                    0 },
    };
    static DIDATAFORMAT g_dfDIJoystick = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_ABSAXIS,
        sizeof(DIJOYSTATE),
        44,
        g_dfDIJoystickObject,
    };
    
    struct DirectInput::_Data
    {
        HWND window = NULL;
        HMODULE dll = NULL;
        Microsoft::WRL::ComPtr<IDirectInput8W> dinput;
        std::vector<DIDEVICEINSTANCEW> gamepad_device;
        std::vector<Microsoft::WRL::ComPtr<IDirectInputDevice8W>> gamepad;
        std::vector<AxisRange> gamepad_prop;
        std::vector<DIJOYSTATE> gamepad_state;
    };
    
    static BOOL CALLBACK _listGamepads(LPCDIDEVICEINSTANCEW device, LPVOID data)
    {
        DebugLog(L"InstanceName:%s ProductName:%s UsagePage:%u Usage:%u\n",
            device->tszInstanceName, device->tszProductName,
            device->wUsagePage, device->wUsage);
        #define case_print(x) case x: { DebugLog(L"    " L#x L"\n"); break; }
        switch (device->dwDevType & 0xFF)
        {
            case_print(DI8DEVTYPE_KEYBOARD);
            case_print(DI8DEVTYPE_MOUSE);
            case_print(DI8DEVTYPE_SCREENPOINTER);
            case_print(DI8DEVTYPE_JOYSTICK);
            case_print(DI8DEVTYPE_GAMEPAD);
            case_print(DI8DEVTYPE_FLIGHT);
            case_print(DI8DEVTYPE_DRIVING);
            case_print(DI8DEVTYPE_SUPPLEMENTAL);
            case_print(DI8DEVTYPE_1STPERSON);
            case_print(DI8DEVTYPE_DEVICECTRL);
            case_print(DI8DEVTYPE_DEVICE);
            case_print(DI8DEVTYPE_REMOTE);
            default: { DebugLog(L"    DI8DEVTYPE_UNKNOWN\n"); break; }
        };
        #undef case_print
        if (IsXInputDevice(&device->guidProduct))
        {
            DebugLog(L"    XInput support\n");
        }
        else
        {
            ((std::vector<DIDEVICEINSTANCEW>*)data)->push_back(*device);
        }
        return DIENUM_CONTINUE; // DIENUM_STOP
    };
    
    template<typename T>
    inline T clamp(T v, T a, T b)
    {
        if (a > b)
        {
            const T c = a;
            a = b;
            b = c;
        }
        v = (v > a) ? v : a;
        v = (v < b) ? v : b;
        return v;
    }
    inline bool _initGamepad(HWND window, IDirectInputDevice8W* device, DirectInput::AxisRange& range)
    {
        HRESULT hr = 0;
        
        hr = device->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        if (hr != DI_OK)
        {
            DebugLog(L"game controller SetCooperativeLevel failed\n");
        }
        hr = device->SetDataFormat(&g_dfDIJoystick);
        if (hr != DI_OK)
        {
            DebugLog(L"game controller SetDataFormat failed\n");
        }
        DIPROPDWORD bufferProperty;
        bufferProperty.diph.dwSize = sizeof(DIPROPDWORD);
        bufferProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        bufferProperty.diph.dwObj = 0;
        bufferProperty.diph.dwHow = DIPH_DEVICE;
        bufferProperty.dwData = g_dwControllerBufferSize;
        hr = device->SetProperty(DIPROP_BUFFERSIZE, &bufferProperty.diph);
        if (hr != DI_OK)
        {
            DebugLog(L"game controller SetProperty failed\n");
        }
        
        DIPROPRANGE axisRange;
        axisRange.diph.dwSize= sizeof(DIPROPRANGE);
        axisRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        axisRange.diph.dwHow = DIPH_BYOFFSET;
        
        // DO NOT USING SetProperty(DIPROP_RANGE) because it will fxxk XInput
        /*
        // set
        axisRange.lMin = -32768;
        axisRange.lMax = 32767;
        
        // X
        axisRange.diph.dwObj = DIJOFS_X;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // Y
        axisRange.diph.dwObj = DIJOFS_Y;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // Z
        axisRange.diph.dwObj = DIJOFS_Z;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);

        // RZ
        axisRange.diph.dwObj = DIJOFS_RX;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // RY
        axisRange.diph.dwObj = DIJOFS_RY;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        // RZ
        axisRange.diph.dwObj = DIJOFS_RZ;
        hr = device->SetProperty(DIPROP_RANGE, &axisRange.diph);
        //*/
        
        // get
        axisRange.lMin = 0;
        axisRange.lMax = 0;
        
        // X
        axisRange.diph.dwObj = DIJOFS_X;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.XMin = axisRange.lMin;
            range.XMax = axisRange.lMax;
        }
        // Y
        axisRange.diph.dwObj = DIJOFS_Y;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.YMin = axisRange.lMin;
            range.YMax = axisRange.lMax;
        }
        // Z
        axisRange.diph.dwObj = DIJOFS_Z;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.ZMin = axisRange.lMin;
            range.ZMax = axisRange.lMax;
        }
        
        // RX
        axisRange.diph.dwObj = DIJOFS_RX;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RxMin = axisRange.lMin;
            range.RxMax = axisRange.lMax;
        }
        // RY
        axisRange.diph.dwObj = DIJOFS_RY;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RyMin = axisRange.lMin;
            range.RyMax = axisRange.lMax;
        }
        // Z
        axisRange.diph.dwObj = DIJOFS_RZ;
        hr = device->GetProperty(DIPROP_RANGE, &axisRange.diph);
        if (hr == DI_OK || hr == S_FALSE)
        {
            range.RzMin = axisRange.lMin;
            range.RzMax = axisRange.lMax;
        }
        
        hr = device->Acquire();
        if (!(hr == DI_OK || hr == S_FALSE))
        {
            DebugLog(L"game controller first Acquire failed\n");
        }
        
        return true;
    }
    inline void _updateGamepad(IDirectInputDevice8W* device, DIJOYSTATE& state, size_t idx)
    {
        DIDEVICEOBJECTDATA data[g_dwControllerBufferSize];
        DWORD data_n = g_dwControllerBufferSize;
        
        HRESULT hr = 0;
        hr = device->Acquire(); // get device access
        if (hr == DI_OK || hr == S_FALSE)
        {
            hr = device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &data_n, 0);
            if (hr == DI_OK || hr == DI_BUFFEROVERFLOW)
            {
                if (data_n > 0)
                {
                    DebugLog("Gamepad[%u] recive %u data\n", idx, data_n);
                    // process data
                    for (size_t i = 0; i < data_n; i += 1)
                    {
                        // I know, I know...
                        #pragma warning(disable : 4644)
                        switch (data[i].dwOfs)
                        {
                        case DIJOFS_X:
                            state.lX = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_Y:
                            state.lY = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_Z:
                            state.lZ = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RX:
                            state.lRx = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RY:
                            state.lRy = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_RZ:
                            state.lRz = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_SLIDER(0):
                            state.rglSlider[0] = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_SLIDER(1):
                            state.rglSlider[1] = (LONG)data[i].dwData;
                            break;
                        case DIJOFS_POV(0):
                            state.rgdwPOV[0] = data[i].dwData;
                            break;
                        case DIJOFS_POV(1):
                            state.rgdwPOV[1] = data[i].dwData;
                            break;
                        case DIJOFS_POV(2):
                            state.rgdwPOV[2] = data[i].dwData;
                            break;
                        case DIJOFS_POV(3):
                            state.rgdwPOV[3] = data[i].dwData;
                            break;
                        default:
                            if (data[i].dwOfs >= DIJOFS_BUTTON(0) && data[i].dwOfs <= DIJOFS_BUTTON(31))
                            {
                                state.rgbButtons[data[i].dwOfs - DIJOFS_BUTTON(0)] = ((data[i].dwData & 0x80) != 0);
                            }
                            break;
                        }
                        #pragma warning(default : 4644)
                    }
                }
            }
            else
            {
                DebugLog(L"Gamepad[%u] GetDeviceData failed\n", idx);
            }
        }
        else
        {
            DebugLog(L"Gamepad[%u] Acquire failed\n", idx);
        }
    }
    
    #define getself() _Data& self = *_data;
    
    uint32_t DirectInput::count()
    {
        getself();
        return (uint32_t)self.gamepad.size();
    }
    uint32_t DirectInput::refresh()
    {
        getself();
        clear(); // clear first
        if (self.dinput)
        {
            HRESULT hr = 0;
            // list all Gamepads
            // *
            // DI8DEVCLASS_ALL
            // DI8DEVCLASS_GAMECTRL
            // DI8DEVCLASS_KEYBOARD
            // DI8DEVCLASS_POINTER
            // *
            // DIEDFL_ALLDEVICES
            // DIEDFL_ATTACHEDONLY
            hr = self.dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, &_listGamepads, &self.gamepad_device, DI8DEVCLASS_ALL);
            if (hr != DI_OK)
            {
                DebugLog(L"EnumDevices failed\n");
            }
            // create Gamepad devices
            for (auto& v : self.gamepad_device)
            {
                Microsoft::WRL::ComPtr<IDirectInputDevice8W> device;
                hr = self.dinput->CreateDevice(v.guidInstance, device.GetAddressOf(), NULL);
                if (hr == DI_OK)
                {
                    AxisRange range;
                    _initGamepad(self.window, device.Get(), range);
                    self.gamepad.push_back(device);
                    self.gamepad_prop.push_back(range);
                }
                else
                {
                    DebugLog(L"CreateDevice game controller failed\n");
                }
            }
            // resize state buffer
            self.gamepad_state.resize(self.gamepad.size());
            reset();
        }
        else
        {
            DebugLog(L"dinput8 NULL exception\n");
        }
        return (uint32_t)self.gamepad.size();
    }
    void DirectInput::update()
    {
        getself();
        for (size_t idx = 0; idx < self.gamepad.size(); idx += 1)
        {
            _updateGamepad(self.gamepad[idx].Get(), self.gamepad_state[idx], idx);
        }
    }
    void DirectInput::reset()
    {
        getself();
        for (size_t idx = 0; idx < self.gamepad.size(); idx += 1)
        {
            auto& range = self.gamepad_prop[idx];
            auto& state = self.gamepad_state[idx];
            
            #define centered(_X, _A, _B) state._X = (range._B - range._A) / 2;
            centered(lX, XMin, XMax);
            centered(lY, YMin, YMax);
            centered(lZ, ZMin, ZMax);
            centered(lRx, RxMin, RxMax);
            centered(lRy, RyMin, RyMax);
            centered(lRz, RzMin, RzMax);
            #undef centered
            
            ZeroMemory(&state.rglSlider, sizeof(state.rglSlider));
            
            state.rgdwPOV[0] = 0xFFFFFFFF;
            state.rgdwPOV[1] = 0xFFFFFFFF;
            state.rgdwPOV[2] = 0xFFFFFFFF;
            state.rgdwPOV[3] = 0xFFFFFFFF;
            
            ZeroMemory(&state.rgbButtons, sizeof(state.rgbButtons));
        }
    }
    void DirectInput::clear()
    {
        getself();
        self.gamepad_device.clear();
        self.gamepad.clear();
        self.gamepad_prop.clear();
        self.gamepad_state.clear();
    }
    bool DirectInput::getRawState(uint32_t index, DirectInput::RawState* state)
    {
        getself();
        if (index < self.gamepad_state.size())
        {
            CopyMemory(state, &self.gamepad_state[index], sizeof(DirectInput::RawState));
            return true;
        }
        return false;
    }
    bool DirectInput::getState(uint32_t index, DirectInput::State* state)
    {
        getself();
        if (index < self.gamepad_state.size())
        {
            auto& raw = self.gamepad_state[index];
            auto& range = self.gamepad_prop[index];
            auto& _state = *state;
            
            _state.wButtons = 0;
            
            if (raw.rgbButtons[0]) { _state.wButtons |= XINPUT_GAMEPAD_X; }
            if (raw.rgbButtons[1]) { _state.wButtons |= XINPUT_GAMEPAD_A; }
            if (raw.rgbButtons[2]) { _state.wButtons |= XINPUT_GAMEPAD_B; }
            if (raw.rgbButtons[3]) { _state.wButtons |= XINPUT_GAMEPAD_Y; }
            
            if (raw.rgbButtons[4]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER; }
            if (raw.rgbButtons[5]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER; }
            
            if (raw.rgbButtons[6]) { _state.bLeftTrigger  = 255; } else { _state.bLeftTrigger  = 0; }
            if (raw.rgbButtons[7]) { _state.bRightTrigger = 255; } else { _state.bRightTrigger = 0; }
            
            if (raw.rgbButtons[8]) { _state.wButtons |= XINPUT_GAMEPAD_BACK; }
            if (raw.rgbButtons[9]) { _state.wButtons |= XINPUT_GAMEPAD_START; }
            
            if (raw.rgbButtons[10]) { _state.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB; }
            if (raw.rgbButtons[11]) { _state.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB; }
            
            const DWORD pov0 = raw.rgdwPOV[0];
            if ( ((pov0 >= 0) && (pov0 < 4500)) || ((pov0 > 31500) && (pov0 < 36000)) ) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_UP; }
            else if ((pov0 >  4500) && (pov0 < 13500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; }
            else if ((pov0 > 13500) && (pov0 < 22500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN; }
            else if ((pov0 > 22500) && (pov0 < 31500)) { _state.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT; }
            else if (pov0 ==  4500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_UP    | XINPUT_GAMEPAD_DPAD_RIGHT); }
            else if (pov0 == 13500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN); }
            else if (pov0 == 22500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_DOWN  | XINPUT_GAMEPAD_DPAD_LEFT); }
            else if (pov0 == 31500) { _state.wButtons |= (XINPUT_GAMEPAD_DPAD_LEFT  | XINPUT_GAMEPAD_DPAD_UP); }
            
            {
                const float cent = (float)(range.XMin + range.XMax) * 0.5f;
                const float half = (float)(range.XMax - range.XMin) * 0.5f;
                const float numv = ((float)raw.lX - cent) / half;
                _state.sThumbLX = (SHORT)clamp(numv * 32768.0f, -32768.0f, 32767.0f);
            }
            {
                const float cent = (float)(range.YMin + range.YMax) * 0.5f;
                const float half = (float)(range.YMax - range.YMin) * 0.5f;
                const float numv = ((float)raw.lY - cent) / half;
                _state.sThumbLY = (SHORT)clamp(numv * -32768.0f, -32768.0f, 32767.0f);
            }
            
            {
                const float cent = (float)(range.ZMin + range.ZMax) * 0.5f;
                const float half = (float)(range.ZMax - range.ZMin) * 0.5f;
                const float numv = ((float)raw.lZ - cent) / half;
                _state.sThumbRX = (SHORT)clamp(numv * 32768.0f, -32768.0f, 32767.0f);
            }
            {
                const float cent = (float)(range.RzMin + range.RzMax) * 0.5f;
                const float half = (float)(range.RzMax - range.RzMin) * 0.5f;
                const float numv = ((float)raw.lRz - cent) / half;
                _state.sThumbRY = (SHORT)clamp(numv * -32768.0f, -32768.0f, 32767.0f);
            }
            
            return true;
        }
        return false;
    }
    bool DirectInput::updateTargetWindow(ptrdiff_t window)
    {
        getself();
        self.window = (HWND)window;
        if (self.window == NULL)
        {
            DebugLog(L"NULL window exception\n");
            return false;
        }
        refresh();
        return true;
    }
    
    DirectInput::DirectInput(ptrdiff_t window)
    {
        _data = new _Data;
        if (_data == nullptr)
        {
            throw;
        }
        getself();
        
        self.window = (HWND)window;
        if (self.window == NULL)
        {
            DebugLog(L"window is NULL\n");
        }
        self.dll = LoadLibraryW(L"Dinput8.dll");
        if (self.dll == NULL)
        {
            DebugLog(L"load Dinput8.dll failed\n");
            return;
        }
        typedef HRESULT (CALLBACK *f_DirectInput8Create)(HINSTANCE, DWORD, const IID&, LPVOID*, LPUNKNOWN);
        f_DirectInput8Create f = (f_DirectInput8Create)GetProcAddress(self.dll, "DirectInput8Create");
        if (f == NULL)
        {
            DebugLog(L"GetProcAddress failed, can not find DirectInput8Create\n");
            return;
        }
        HRESULT hr = 0;
        hr = f(GetModuleHandleW(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8W, (LPVOID*)(self.dinput.GetAddressOf()), NULL);
        if (hr != DI_OK)
        {
            DebugLog(L"DirectInput8Create failed\n");
            return;
        }
    }
    DirectInput::~DirectInput()
    {
        getself();
        
        clear();
        self.dinput.Reset();
        if (self.dll != NULL)
        {
            FreeLibrary(self.dll);
            self.dll = NULL;
        }
        if (self.window != NULL)
        {
            self.window = NULL;
        }
        
        delete _data;
        _data = nullptr;
    }
};

//********** MAGIC CODE **********//

#include <wbemidl.h>
#include <oleauto.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { (x)->Release(); (x) = NULL; }
#endif

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains 
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput 
//-----------------------------------------------------------------------------
BOOL IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
    IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    bool                    bIsXinputDevice= false;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

    // CoInit if needed
    hr = CoInitialize(NULL);
    bool bCleanupCOM = SUCCEEDED(hr);

    // So we can call VariantClear() later, even if we never had a successful IWbemClassObject::Get().
    VariantInit(&var);

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
                    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;

                    // Compare the VID/PID to the DInput device
                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );
                    if( dwVidPid == pGuidProductFromDirectInput->Data1 )
                    {
                        bIsXinputDevice = true;
                        goto LCleanup;
                    }
                }
            }
            VariantClear(&var);
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    VariantClear(&var);
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
        CoUninitialize();

    return bIsXinputDevice;
}
