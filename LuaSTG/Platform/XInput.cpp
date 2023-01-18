#include <array>
#include <algorithm>
#include <string_view>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "XInput.hpp"

namespace Platform::XInput
{
	// NOTICE: XInputEnable is deprecated, we should simulate its behavior

	struct XInputLoader
	{
		HMODULE dll_xinput{ NULL };
		//decltype(::XInputEnable)* api_XInputEnable{ NULL };
		decltype(::XInputGetState)* api_XInputGetState{ NULL };
		decltype(::XInputSetState)* api_XInputSetState{ NULL };
		decltype(::XInputGetCapabilities)* api_XInputGetCapabilities{ NULL };

		std::array<bool, XUSER_MAX_COUNT> valid{};
		std::array<XINPUT_STATE, XUSER_MAX_COUNT> state{};
		bool enable{ true };

		XInputLoader()
		{
			const std::array<std::wstring_view, 5> dll_names = {
				L"Xinput1_4.dll",   // Windows 8+
				L"xinput1_3.dll",   // DirectX SDK
				L"Xinput9_1_0.dll", // Windows Vista, Windows 7
				L"xinput1_2.dll",   // DirectX SDK
				L"xinput1_1.dll",   // DirectX SDK
			};
			for (auto& v : dll_names)
			{
				if (HMODULE dll = ::LoadLibraryW(v.data()))
				{
					dll_xinput = dll;
					//api_XInputEnable = (decltype(api_XInputEnable))
					//	::GetProcAddress(dll, "XInputEnable");
					api_XInputGetState = (decltype(api_XInputGetState))
						::GetProcAddress(dll, "XInputGetState");
					api_XInputSetState = (decltype(api_XInputSetState))
						::GetProcAddress(dll, "XInputSetState");
					api_XInputGetCapabilities = (decltype(api_XInputGetCapabilities))
						::GetProcAddress(dll, "XInputGetCapabilities");
					break;
				}
			}
		}
		~XInputLoader()
		{
			if (dll_xinput)
				::FreeLibrary(dll_xinput);
			dll_xinput = NULL;
			//api_XInputEnable = NULL;
			api_XInputGetState = NULL;
			api_XInputSetState = NULL;
			api_XInputGetCapabilities = NULL;
		}
	};
	static XInputLoader XInput;

	void setEnable(bool state)
	{
		//if (XInput.api_XInputEnable)
		//{
		//	XInput.api_XInputEnable(state ? TRUE : FALSE);
		//}
		XInput.enable = state;
		if (!state)
		{
			XInput.state.fill({});
		}
	}
	bool isConnected(int index)
	{
		if (index >= 0 && index < XUSER_MAX_COUNT)
		{
			return XInput.valid[index];
		}
		return false;
	}
	int refresh()
	{
		int count = 0;
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i += 1)
		{
			if (XInput.api_XInputGetState && ERROR_SUCCESS == XInput.api_XInputGetState(i, &XInput.state[i]))
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
				if (XInput.api_XInputGetState && ERROR_SUCCESS != XInput.api_XInputGetState(i, &XInput.state[i]))
				{
					XInput.valid[i] = false;
					XInput.state[i] = {};
				}
			}
		}
	}

	bool getKeyState(int index, int key)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return (XInput.state[index].Gamepad.wButtons & key) != 0;
		}
		return false;
	}

	float getLeftTrigger(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return std::clamp((float)XInput.state[index].Gamepad.bLeftTrigger / 255.0f, 0.0f, 1.0f);
		}
		return 0.0f;
	}
	float getRightTrigger(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return std::clamp((float)XInput.state[index].Gamepad.bRightTrigger / 255.0f, 0.0f, 1.0f);
		}
		return 0.0f;
	}

	float getLeftThumbX(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return std::clamp((float)XInput.state[index].Gamepad.sThumbLX / 32767.0f, -1.0f, 1.0f);
		}
		return 0.0f;
	}
	float getLeftThumbY(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return std::clamp((float)XInput.state[index].Gamepad.sThumbLY / 32767.0f, -1.0f, 1.0f);
		}
		return 0.0f;
	}
	float getRightThumbX(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
		{
			return std::clamp((float)XInput.state[index].Gamepad.sThumbRX / 32767.0f, -1.0f, 1.0f);
		}
		return 0.0f;
	}
	float getRightThumbY(int index)
	{
		if (XInput.enable && index >= 0 && index < XUSER_MAX_COUNT)
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

	DWORD getState(DWORD dwUserIndex, XINPUT_STATE* pState)
	{
		if (XInput.api_XInputGetState)
		{
			return XInput.api_XInputGetState(dwUserIndex, pState);
		}
		return ERROR_NOT_SUPPORTED;
	}
	DWORD setState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
	{
		if (XInput.api_XInputSetState)
		{
			return XInput.api_XInputSetState(dwUserIndex, pVibration);
		}
		return ERROR_NOT_SUPPORTED;
	}
	DWORD getCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
	{
		if (XInput.api_XInputGetCapabilities)
		{
			return XInput.api_XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
		}
		return ERROR_NOT_SUPPORTED;
	}
}
