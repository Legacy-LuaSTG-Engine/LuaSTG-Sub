#pragma comment(lib, "Xinput9_1_0.lib")

#include "E2DXInput.hpp"

namespace Eyes2D {
	namespace Input {
		bool XInput::checkIndexValid(int index) {
			if ((index > XUSER_MAX_COUNT) || (index > m_XDeviceCount) || (index < 1)) {
				return false;//超出范围
			}
			else {
				return true;
			}
		}

		int XInput::Refresh() {
			memset(m_XDevices, 0, (XUSER_MAX_COUNT + 1) * sizeof(XINPUT_STATE));//重置原有的信息
			int devCount = 0;
			for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
			{
				XINPUT_STATE state;
				memset(&state, 0, sizeof(XINPUT_STATE));
				if (XInputGetState(i, &state) == ERROR_SUCCESS) {
					m_XDevices[i + 1] = state;
					devCount++;
				}
				else {
					break;
				}
			}
			m_XDeviceCount = devCount;
			return devCount;
		}

		void XInput::Update() {
			DWORD j = (DWORD)m_XDeviceCount;//先转换成DWORD，处理<运算符符号不匹配
			for (DWORD i = 0; i < j; i++)
			{
				XINPUT_STATE state;
				memset(&state, 0, sizeof(XINPUT_STATE));
				if (XInputGetState(i, &state) == ERROR_SUCCESS) {
					m_XDevices[i + 1] = state;
				}
				else {
					Refresh();//失败则刷新状态
					break;
				}
			}
		}

		bool XInput::GetKeyState(int index, int vkey) {
			if (!checkIndexValid(index)) { return false; }
			return (m_XDevices[index].Gamepad.wButtons & vkey);
		}

		int XInput::GetTriggerStateL(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.bLeftTrigger;
		}

		int XInput::GetTriggerStateR(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.bRightTrigger;
		}

		int XInput::GetThumbStateLX(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.sThumbLX;
		}

		int XInput::GetThumbStateLY(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.sThumbLY;
		}

		int XInput::GetThumbStateRX(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.sThumbRX;
		}

		int XInput::GetThumbStateRY(int index) {
			if (!checkIndexValid(index)) { return false; }
			return m_XDevices[index].Gamepad.sThumbRY;
		}

		bool XInput::SetMotorSpeed(int index, int low, int high) {
			if (!checkIndexValid(index)) { return false; }
			//索引要-1
			XINPUT_VIBRATION var;
			if (low < 0 || high < 0) {
				//针对一项进行更改
				XINPUT_CAPABILITIES features;
				memset(&features, 0, sizeof(XINPUT_CAPABILITIES));
				if (XInputGetCapabilities(index - 1, 0, &features) != ERROR_SUCCESS) {
					return false;
				}
				var = features.Vibration;
				if (low >= 0) {
					var.wLeftMotorSpeed = low;
				}
				if (high >= 0) {
					var.wRightMotorSpeed = high;
				}
			}
			else {
				var.wLeftMotorSpeed = low;
				var.wRightMotorSpeed = high;
			}
			return (XInputSetState(index - 1, &var) == ERROR_SUCCESS) ? true : false;
		}

		int XInput::GetMotorSpeedLow(int index) {
			if (!checkIndexValid(index)) { return -1; }
			XINPUT_CAPABILITIES features;
			memset(&features, 0, sizeof(XINPUT_CAPABILITIES));
			if (XInputGetCapabilities(index - 1, 0, &features) != ERROR_SUCCESS) {
				return -1;
			}
			return features.Vibration.wLeftMotorSpeed;
		}

		int XInput::GetMotorSpeedHigh(int index) {
			if (!checkIndexValid(index)) { return -1; }
			XINPUT_CAPABILITIES features;
			memset(&features, 0, sizeof(XINPUT_CAPABILITIES));
			if (XInputGetCapabilities(index - 1, 0, &features) != ERROR_SUCCESS) {
				return -1;
			}
			return features.Vibration.wRightMotorSpeed;
		}
	}
}
