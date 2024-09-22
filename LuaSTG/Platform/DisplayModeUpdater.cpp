#include <cassert>
#include <string>
#include <format>
#include <vector>
#include <algorithm>
#include "utf8.hpp"
#include <Windows.h>
#include "DisplayModeUpdater.hpp"

#define ReportError(...) void(0);

namespace Platform
{
	bool DisplayModeUpdater::Enter(HWND window, UINT width, UINT height)
	{
		Leave();

		assert(window);
		assert(width > 0 && height > 0);

		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
		assert(monitor);

		MONITORINFOEXW monitor_info{};
		monitor_info.cbSize = sizeof(monitor_info);
		if (!GetMonitorInfoW(monitor, &monitor_info)) {
			ReportError("GetMonitorInfoW");
			return false;
		}

		DISPLAY_DEVICEW temp_device{};
		temp_device.cb = sizeof(temp_device);
		last_device = {};
		for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &temp_device, 0); i += 1) {
			if (std::wstring_view(temp_device.DeviceName) == std::wstring_view(monitor_info.szDevice)) {
				last_device = temp_device;
				break;
			}
			temp_device = {};
			temp_device.cb = sizeof(temp_device);
		}
		assert(last_device.cb > 0);
		if (last_device.cb == 0) {
			ReportError(std::format("EnumDisplayDevicesW ({})", utf8::to_string(monitor_info.szDevice)));
			return false;
		}

		last_mode = {};
		last_mode.dmSize = sizeof(last_mode);
		if (!EnumDisplaySettingsExW(last_device.DeviceName, ENUM_CURRENT_SETTINGS, &last_mode, 0)) {
			ReportError("EnumDisplaySettingsExW");
			return false;
		}

		std::vector<DEVMODEW> modes;
		DEVMODEW temp_mode{};
		temp_mode.dmSize = sizeof(temp_mode);
		for (DWORD i = 0; EnumDisplaySettingsExW(last_device.DeviceName, i, &temp_mode, 0); i += 1) {
			modes.push_back(temp_mode);
			temp_mode = {};
			temp_mode.dmSize = sizeof(temp_mode);
		}

		auto is_same_aspect_ratio = [this](DEVMODEW const& m) -> bool
		{
			DWORD const width = m.dmPelsWidth * last_mode.dmPelsHeight / m.dmPelsHeight;
			return 2 >= std::abs(static_cast<int>(width) - static_cast<int>(last_mode.dmPelsWidth));
		};

		auto is_size_larger = [&](DEVMODEW const& m) -> bool
		{
			return m.dmPelsWidth >= width && m.dmPelsHeight >= height;
		};

#define u_s2 .

		auto is_same_mode_basic = [](DEVMODEW const& l, DEVMODEW const& r) -> bool
		{
			return l u_s2 dmPosition.x == r u_s2 dmPosition.x
				&& l u_s2 dmPosition.y == r u_s2 dmPosition.y
				&& l u_s2 dmDisplayOrientation == r u_s2 dmDisplayOrientation
				&& l.dmBitsPerPel == r.dmBitsPerPel
				&& l.dmPelsWidth == r.dmPelsWidth
				&& l.dmPelsHeight == r.dmPelsHeight
				&& l.dmDisplayFrequency == r.dmDisplayFrequency
				;
		};

		auto is_auto_scaling = [](DEVMODEW const& m) -> bool
		{
			return m u_s2 dmDisplayFixedOutput == DMDFO_DEFAULT;
		};

#undef u_s2

		auto is_high_refresh_rate = [](DEVMODEW const& m) -> bool
		{
			return m.dmDisplayFrequency >= 58;
		};

		for (auto it = modes.begin(); it != modes.end();) {
			if (is_same_aspect_ratio(*it)) {
				it++;
			}
			else {
				it = modes.erase(it);
			}
		}

		for (auto it = modes.begin(); it != modes.end();) {
			if (is_size_larger(*it)) {
				it++;
			}
			else {
				it = modes.erase(it);
			}
		}

		for (auto it = modes.begin(); it != modes.end();) {
			if (is_auto_scaling(*it)) {
				it++;
			}
			else {
				it = modes.erase(it);
			}
		}

		for (auto it = modes.begin(); it != modes.end();) {
			if (it->dmBitsPerPel >= 32) {
				it++;
			}
			else {
				it = modes.erase(it);
			}
		}

		std::ranges::sort(modes, [width, height](DEVMODEW const& l, DEVMODEW const& r) -> bool
		{
			double const s0 = double(width) * double(height);
			double const sl = double(l.dmPelsWidth) * double(l.dmPelsHeight);
			double const sr = double(r.dmPelsWidth) * double(r.dmPelsHeight);
			double const vl = sl / s0;
			double const vr = sr / s0;
			if (vl != vr) {
				return vl < vr;
			}
			else {
				return l.dmDisplayFrequency > r.dmDisplayFrequency;
			}
		});

		if (modes.empty()) {
			return false;
		}

		if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettingsExW(last_device.DeviceName, &modes.at(0), nullptr, 0, nullptr)) {
			ReportError("ChangeDisplaySettingsExW");
			return false;
		}

		is_scope = true;
		return true;
	}
	void DisplayModeUpdater::Leave()
	{
		if (is_scope) {
			is_scope = false;
			ChangeDisplaySettingsExW(last_device.DeviceName, &last_mode, nullptr, 0, nullptr);
		}
	}

	DisplayModeUpdater::DisplayModeUpdater() = default;
	DisplayModeUpdater::~DisplayModeUpdater()
	{
		Leave();
	}
}
