#pragma once
#include <vector>

namespace platform
{
	class MonitorList
	{
	private:
		struct MonitorData
		{
			HMONITOR hMonitor{};
			MONITORINFO tInfo{};
		};
	private:
		std::vector<MonitorData> monitors;
	private:
		static BOOL CALLBACK callback(HMONITOR hMonitor, HDC, LPRECT, LPARAM pPraram);
	public:
		BOOL Refresh();
		size_t GetCount();
		HMONITOR GetHandle(size_t index);
		RECT GetRect(size_t index);
		BOOL IsPrimary(size_t index);
		size_t FindFromWindow(HWND window, bool default_to_primary = false);
		HMONITOR GetFromWindow(HWND window, bool default_to_primary = false);
	public:
		BOOL MoveWindowToCenter(size_t index, HWND window);
		BOOL ResizeWindowToFullScreen(size_t index, HWND window);
	public:
		static BOOL MoveWindowToCenter(HWND window);
		static BOOL ResizeWindowToFullScreen(HWND window);
		static BOOL GetRectFromWindow(HWND window, RECT* rect);
	public:
		MonitorList();
		~MonitorList();
	public:
		static MonitorList& get();
	};
}
