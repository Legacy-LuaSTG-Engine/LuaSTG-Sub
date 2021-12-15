#include "Engine/f2dMonitorHelper.h"
#include <cassert>
#include <vector>
#include <Windows.h>

struct f2dMonitorHelper::Data
{
    std::vector<HMONITOR> monitor;
    std::vector<MONITORINFO> info;
    static BOOL CALLBACK callback(HMONITOR hMonitor, HDC, LPRECT, LPARAM pPraram)
    {
        f2dMonitorHelper::Data* data = (f2dMonitorHelper::Data*)pPraram;
        data->monitor.push_back(hMonitor);
        return TRUE;
    }
};

#define self (*_data)

void f2dMonitorHelper::clear()
{
    self.monitor.clear();
    self.info.clear();
}
bool f2dMonitorHelper::refresh()
{
    clear();
    if (FALSE == ::EnumDisplayMonitors(NULL, NULL, &f2dMonitorHelper::Data::callback, (LPARAM)_data))
    {
        return false;
    }
    self.info.resize(self.monitor.size());
    for (size_t idx = 0; idx < self.monitor.size(); idx++)
    {
        self.info[idx] = MONITORINFO{};
        self.info[idx].cbSize = sizeof(MONITORINFO);
        if (FALSE == GetMonitorInfoA(self.monitor[idx], (LPMONITORINFO)&self.info[idx]))
        {
            self.info[idx] = MONITORINFO{};
            self.info[idx].cbSize = sizeof(MONITORINFO);
        }
    }
    return true;
}
uint32_t f2dMonitorHelper::getCount()
{
    return (uint32_t)self.monitor.size();
}
void* f2dMonitorHelper::getHandle(uint32_t index)
{
    if (self.info.size() == 0) return nullptr;
    assert(index < self.info.size());
    return self.monitor[index];
}
int32_t f2dMonitorHelper::getX(uint32_t index)
{
    if (self.info.size() == 0) return 0;
    assert(index < self.info.size());
    return self.info[index].rcMonitor.left;
}
int32_t f2dMonitorHelper::getY(uint32_t index)
{
    if (self.info.size() == 0) return 0;
    assert(index < self.info.size());
    return self.info[index].rcMonitor.top;
}
uint32_t f2dMonitorHelper::getWidth(uint32_t index)
{
    if (self.info.size() == 0) return 0;
    assert(index < self.info.size());
    assert(self.info[index].rcMonitor.right >= self.info[index].rcMonitor.left);
    return (uint32_t)(self.info[index].rcMonitor.right - self.info[index].rcMonitor.left);
}
uint32_t f2dMonitorHelper::getHeight(uint32_t index)
{
    if (self.info.size() == 0) return 0;
    assert(index < self.info.size());
    assert(self.info[index].rcMonitor.bottom >= self.info[index].rcMonitor.top);
    return (uint32_t)(self.info[index].rcMonitor.bottom - self.info[index].rcMonitor.top);
}
bool f2dMonitorHelper::isPrimary(uint32_t index)
{
    if (self.info.size() == 0) return false;
    assert(index < self.info.size());
    return (self.info[index].dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY;
}
uint32_t f2dMonitorHelper::findFromWindow(void* window, bool primary)
{
    HMONITOR monitor_ = ::MonitorFromWindow((HWND)window,
        primary ? MONITOR_DEFAULTTOPRIMARY :MONITOR_DEFAULTTONEAREST);
    for (size_t idx = 0; idx < self.monitor.size(); idx++)
    {
        if (self.monitor[idx] == monitor_)
        {
            return idx;
        }
    }
    return 0;
}
void* f2dMonitorHelper::getFromWindow(void* window, bool primary)
{
    return getHandle(findFromWindow(window, primary));
}

f2dMonitorHelper::f2dMonitorHelper()
{
    _data = new f2dMonitorHelper::Data;
    refresh();
}
f2dMonitorHelper::~f2dMonitorHelper()
{
    delete _data;
    _data = nullptr;
}

f2dMonitorHelper& f2dMonitorHelper::get()
{
    static f2dMonitorHelper instance_;
    return instance_;
}
