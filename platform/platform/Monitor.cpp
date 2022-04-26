#include "Monitor.hpp"

namespace platform
{
	BOOL CALLBACK MonitorList::callback(HMONITOR hMonitor, HDC, LPRECT, LPARAM pPraram)
	{
		MonitorList* self = (MonitorList*)pPraram;
		self->monitors.emplace_back(MonitorData{
			.hMonitor = hMonitor,
			.tInfo = {
				.cbSize = sizeof(MONITORINFO),
				.rcMonitor = {},
				.rcWork = {},
				.dwFlags = 0,
			},
		});
		return TRUE;
	}

	BOOL MonitorList::Refresh()
	{
		monitors.clear();
		if (!EnumDisplayMonitors(NULL, NULL, &callback, (LPARAM)this))
		{
			return false;
		}
		for (auto& v : monitors)
		{
			if (!GetMonitorInfoA(v.hMonitor, &v.tInfo))
			{
				return false;
			}
		}
		return true;
	}
	size_t MonitorList::GetCount()
	{
		return monitors.size();
	}
	HMONITOR MonitorList::GetHandle(size_t index)
	{
		assert(!monitors.empty() && index < monitors.size());
		return monitors[index].hMonitor;
	}
	RECT MonitorList::GetRect(size_t index)
	{
		assert(!monitors.empty() && index < monitors.size());
		return monitors[index].tInfo.rcMonitor;
	}
	BOOL MonitorList::IsPrimary(size_t index)
	{
		assert(!monitors.empty() && index < monitors.size());
		return (monitors[index].tInfo.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY;
	}

	size_t MonitorList::FindFromWindow(HWND window, bool default_to_primary)
	{
		assert(!monitors.empty());
		HMONITOR monitor_ = MonitorFromWindow(window, default_to_primary ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
		for (size_t idx = 0; idx < monitors.size(); idx++)
		{
			if (monitors[idx].hMonitor == monitor_)
			{
				return idx;
			}
		}
		return size_t(-1);
	}
	HMONITOR MonitorList::GetFromWindow(HWND window, bool default_to_primary)
	{
		size_t const index = FindFromWindow(window, default_to_primary);
		if (index != size_t(-1))
		{
			return monitors[index].hMonitor;
		}
		else
		{
			return NULL;
		}
	}

	BOOL _MoveWindowToCenter(HWND window, RECT const& area)
	{
		RECT rect = {};
		if (!GetWindowRect(window, &rect))
		{
			return FALSE;
		}
		LONG const sx = area.left;
		LONG const sy = area.top;
		LONG const sw = area.right - area.left;
		LONG const sh = area.bottom - area.top;
		LONG const ww = rect.right - rect.left;
		LONG const wh = rect.bottom - rect.top;
		return SetWindowPos(window, NULL, sx + (sw / 2) - (ww / 2), sy + (sh / 2) - (wh / 2), ww, wh, SWP_NOZORDER);
	}
	BOOL _ResizeWindowToFullScreen(HWND window, RECT const& area)
	{
		return SetWindowPos(window, NULL, area.left, area.top, area.right - area.left, area.bottom - area.top, SWP_NOZORDER);
	}

	BOOL MonitorList::MoveWindowToCenter(size_t index, HWND window)
	{
		assert(!monitors.empty() && index < monitors.size());
		if (monitors.empty() || index >= monitors.size())
		{
			return FALSE;
		}
		return _MoveWindowToCenter(window, monitors[index].tInfo.rcMonitor);
	}
	BOOL MonitorList::ResizeWindowToFullScreen(size_t index, HWND window)
	{
		assert(!monitors.empty() && index < monitors.size());
		if (monitors.empty() || index >= monitors.size())
		{
			return FALSE;
		}
		return _ResizeWindowToFullScreen(window, monitors[index].tInfo.rcMonitor);
	}

	BOOL MonitorList::MoveWindowToCenter(HWND window)
	{
		RECT area = {};
		if (!GetRectFromWindow(window, &area))
		{
			return FALSE;
		}
		return _MoveWindowToCenter(window, area);
	}
	BOOL MonitorList::ResizeWindowToFullScreen(HWND window)
	{
		RECT area = {};
		if (!GetRectFromWindow(window, &area))
		{
			return FALSE;
		}
		return _ResizeWindowToFullScreen(window, area);
	}
	BOOL MonitorList::GetRectFromWindow(HWND window, RECT* rect)
	{
		HMONITOR hMonitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
		if (!hMonitor)
		{
			assert(false);
			return FALSE;
		}
		MONITORINFO tInfo{ .cbSize = sizeof(MONITORINFO) };
		if (!GetMonitorInfoA(hMonitor, &tInfo))
		{
			return FALSE;
		}
		*rect = tInfo.rcMonitor;
		return TRUE;
	}

	MonitorList::MonitorList()
	{
	}
	MonitorList::~MonitorList()
	{
	}

	MonitorList& MonitorList::get()
	{
		static MonitorList instance;
		return instance;
	}
}
