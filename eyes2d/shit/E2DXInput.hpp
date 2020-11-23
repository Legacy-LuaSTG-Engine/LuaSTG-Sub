#pragma once
#include "E2DGlobal.hpp"
#include <string>

#define XINPUT_USE_9_1_0
#include <Xinput.h>
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

namespace Eyes2D {
	namespace Input {
		class EYESDLLAPI XInput {
		private:
			int m_XDeviceCount = 0;                           //检测到的设备数量
			XINPUT_STATE m_XDevices[XUSER_MAX_COUNT + 1]; //调整索引从1开始
		private:
			// 检查索引是否在允许的范围内
			bool checkIndexValid(int index);
		public:
			// 刷新设备，并返回设备数量
			int Refresh();
			// 获取设备输入，如果设备丢失则刷新设备以及设备数量
			void Update();
			// 返回设备数量
			int GetDeviceCount() { return m_XDeviceCount; }
		public:
			// 获取指定设备的按键状态
			bool GetKeyState(int index, int vkey);
			// 获取指定设备左扳机状态
			int GetTriggerStateL(int index);
			// 获取指定设备右扳机状态
			int GetTriggerStateR(int index);
			// 获取指定设备左摇杆X轴状态
			int GetThumbStateLX(int index);
			// 获取指定设备左摇杆Y轴状态
			int GetThumbStateLY(int index);
			// 获取指定设备右摇杆X轴状态
			int GetThumbStateRX(int index);
			// 获取指定设备右摇杆Y轴状态
			int GetThumbStateRY(int index);
			// 设置马达震动，需要设备支持
			bool SetMotorSpeed(int index, int low, int high);
			// 获取低速马达状态，失败则返回-1
			int GetMotorSpeedLow(int index);
			// 获取高速马达状态，失败则返回-1
			int GetMotorSpeedHigh(int index);
		public:
			XInput() {
				memset(m_XDevices, 0, (XUSER_MAX_COUNT + 1) * sizeof(XINPUT_STATE));
				Refresh();
			}
			~XInput() {
				for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
					SetMotorSpeed(i + 1, 0, 0);
				}
			}
		};
	}
}
