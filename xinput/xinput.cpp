#include "xinput/xinput.hpp"
#include <array>
#include <algorithm>
#include <string_view>
#include <Windows.h>
#include <Xinput.h>

namespace xinput
{
    struct XInputInstance
    {
        typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD, XINPUT_STATE*);
        typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD, XINPUT_VIBRATION*);
        typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
        
        HMODULE _dll = nullptr;
        PFN_XInputGetState GetState = nullptr;
        PFN_XInputSetState SetState = nullptr;
        PFN_XInputGetCapabilities GetCapabilities = nullptr;
        std::array<bool, XUSER_MAX_COUNT> valid;
        std::array<XINPUT_STATE, XUSER_MAX_COUNT> state;
        
        XInputInstance()
        {
            const std::array<std::wstring_view, 5> dll_names = {
                L"Xinput1_4.dll",   // Windows 8+
                L"xinput1_3.dll",   // DirectX SDK
                L"Xinput9_1_0.dll", // Windows Vista, Windows 7
                L"xinput1_2.dll",   // DirectX SDK
                L"xinput1_1.dll",   // DirectX SDK
            };
            for (auto v : dll_names)
            {
                if (HMODULE dll = ::LoadLibraryW(v.data()))
                {
                    _dll = dll;
                    GetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
                    SetState = (PFN_XInputSetState)::GetProcAddress(dll, "XInputSetState");
                    GetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
                    break;
                }
            }
        }
        ~XInputInstance()
        {
            if (_dll) ::FreeLibrary(_dll);
            _dll = nullptr;
            GetState = nullptr;
            SetState = nullptr;
            GetCapabilities = nullptr;
        }
        
        static XInputInstance& get()
        {
            static XInputInstance instance;
            return instance;
        }
    };
    
    #define XInput XInputInstance::get()
    
    int refresh()
    {
        int count = 0;
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (ERROR_SUCCESS == XInput.GetState(i, &XInput.state[i]))
            {
                XInput.valid[i] = true;
                count += 1;
            }
            else
            {
                XInput.valid[i] = false;
                XInput.state[i] = {};
            }
        }
        return count;
    }
    void update()
    {
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                if (ERROR_SUCCESS != XInput.GetState(i, &XInput.state[i]))
                {
                    XInput.valid[i] = false;
                    XInput.state[i] = {};
                }
            }
        }
    }
    
    bool getKeyState(int index, int key)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return (XInput.state[index].Gamepad.wButtons & key) != 0;
        }
        return false;
    }
    
    float getLeftTrigger(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.bLeftTrigger / 255.0f, 0.0f, 1.0f);
        }
        return 0.0f;
    }
    float getRightTrigger(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.bRightTrigger / 255.0f, 0.0f, 1.0f);
        }
        return 0.0f;
    }
    
    float getLeftThumbX(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.sThumbLX / 32767.0f, -1.0f, 1.0f);
        }
        return 0.0f;
    }
    float getLeftThumbY(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.sThumbLY / 32767.0f, -1.0f, 1.0f);
        }
        return 0.0f;
    }
    float getRightThumbX(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.sThumbRX / 32767.0f, -1.0f, 1.0f);
        }
        return 0.0f;
    }
    float getRightThumbY(int index)
    {
        if (index >= 0 && index < XUSER_MAX_COUNT)
        {
            return std::clamp((float)XInput.state[index].Gamepad.sThumbRY / 32767.0f, -1.0f, 1.0f);
        }
        return 0.0f;
    }
    
    bool getKeyState(int key)
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getKeyState(i, key);
            }
        }
        return false;
    }
    
    float getLeftTrigger()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getLeftTrigger(i);
            }
        }
        return 0.0f;
    }
    float getRightTrigger()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getRightTrigger(i);
            }
        }
        return 0.0f;
    }
    float getLeftThumbX()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getLeftThumbX(i);
            }
        }
        return 0.0f;
    }
    float getLeftThumbY()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getLeftThumbY(i);
            }
        }
        return 0.0f;
    }
    float getRightThumbX()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getRightThumbX(i);
            }
        }
        return 0.0f;
    }
    float getRightThumbY()
    {
        for (int i = 0; i < XUSER_MAX_COUNT; i += 1)
        {
            if (XInput.valid[i])
            {
                return getRightThumbY(i);
            }
        }
        return 0.0f;
    }
}
