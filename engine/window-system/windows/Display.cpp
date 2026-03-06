#include "windows/Display.hpp"
#include "core/Window.hpp"
#include <vector>
#include <format>
#include "utf8.hpp"
#include "win32/win32.hpp"
#include "win32/abi.hpp"

namespace {
    template<typename T = MONITORINFO>
    T getMonitorInfo(const HMONITOR monitor) {
        static_assert(std::is_same_v<T, MONITORINFO> || std::is_same_v<T, MONITORINFOEXW>);
        assert(monitor);
        T info{};
        info.cbSize = sizeof(info);
        [[maybe_unused]] auto const result = GetMonitorInfoW(monitor, &info);
        assert(result);
        return info;
    }
}

namespace core {
    // IDisplay

    void* Display::getNativeHandle() {
        return win32_monitor;
    }
    void Display::getFriendlyName(IImmutableString** const output) {
        auto const info = getMonitorInfo<MONITORINFOEXW>(win32_monitor);

        UINT path_count{};
        UINT mode_count{};
        if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_DATABASE_CURRENT, &path_count, &mode_count)) {
            std::vector<DISPLAYCONFIG_PATH_INFO> path_list(path_count);
            std::vector<DISPLAYCONFIG_MODE_INFO> mode_list(mode_count);
            DISPLAYCONFIG_TOPOLOGY_ID topology_id{};
            if (ERROR_SUCCESS == QueryDisplayConfig(QDC_DATABASE_CURRENT, &path_count, path_list.data(), &mode_count, mode_list.data(), &topology_id)) {
                for (auto const& path : path_list) {
                    DISPLAYCONFIG_SOURCE_DEVICE_NAME source_device_name{};
                    source_device_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                    source_device_name.header.size = sizeof(source_device_name);
                    source_device_name.header.adapterId = path.sourceInfo.adapterId;
                    source_device_name.header.id = path.sourceInfo.id;
                    if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&source_device_name.header)) {
                        if (std::wstring_view(info.szDevice) == std::wstring_view(source_device_name.viewGdiDeviceName)) {
                            DISPLAYCONFIG_TARGET_DEVICE_NAME target_device_name{};
                            target_device_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                            target_device_name.header.size = sizeof(target_device_name);
                            target_device_name.header.adapterId = path.targetInfo.adapterId;
                            target_device_name.header.id = path.targetInfo.id;
                            if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&target_device_name.header)) {
                                std::string buffer;
                                if (target_device_name.flags.edidIdsValid) {
                                    auto const manufacture_id = ((target_device_name.edidManufactureId & 0xff) << 8) | ((target_device_name.edidManufactureId & 0xff00) >> 8);
                                    auto const letter1 = ((manufacture_id & 0x7c00) >> 10) - 1;
                                    auto const letter2 = ((manufacture_id & 0x3e0) >> 5) - 1;
                                    auto const letter3 = (manufacture_id & 0x1f) - 1;
                                    buffer.push_back('A' + static_cast<char>(letter1));
                                    buffer.push_back('A' + static_cast<char>(letter2));
                                    buffer.push_back('A' + static_cast<char>(letter3));
                                }
                                if (!std::wstring_view(target_device_name.monitorFriendlyDeviceName).empty()) {
                                    if (!buffer.empty()) {
                                        buffer.push_back(' ');
                                    }
                                    buffer.append(utf8::to_string(target_device_name.monitorFriendlyDeviceName));
                                }
                                else if (target_device_name.flags.edidIdsValid) {
                                    buffer.append(std::format("{:04X}", target_device_name.edidProductCodeId));
                                }
                                IImmutableString::create(buffer, output);
                                return;
                            }
                        }
                    }
                }
            }
        }

        IImmutableString::create(utf8::to_string(info.szDevice), output);
    }
    Vector2U Display::getSize() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcMonitor;
        return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
    }
    Vector2I Display::getPosition() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcMonitor;
        return Vector2I(rc.left, rc.top);
    }
    RectI Display::getRect() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcMonitor;
        return RectI(rc.left, rc.top, rc.right, rc.bottom);
    }
    Vector2U Display::getWorkAreaSize() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcWork;
        return Vector2U(static_cast<uint32_t>(rc.right - rc.left), static_cast<uint32_t>(rc.bottom - rc.top));
    }
    Vector2I Display::getWorkAreaPosition() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcWork;
        return Vector2I(rc.left, rc.top);
    }
    RectI Display::getWorkAreaRect() {
        auto const info = getMonitorInfo(win32_monitor);
        auto const& rc = info.rcWork;
        return RectI(rc.left, rc.top, rc.right, rc.bottom);
    }
    bool Display::isPrimary() {
        auto const info = getMonitorInfo(win32_monitor);
        return !!(info.dwFlags & MONITORINFOF_PRIMARY);
    }
    float Display::getDisplayScale() {
        return win32::getDpiScalingForMonitor(win32_monitor);
    }

    // Display

    Display::Display(HMONITOR const monitor) : win32_monitor(monitor) {
    }
    Display::~Display() = default;
}

namespace core {
    bool IDisplay::getAll(size_t* const count, IDisplay** const output) {
        assert(count != nullptr);
        assert(*count == 0 || (*count > 0 && output != nullptr));
        struct Context {
            std::vector<HMONITOR> list;
            static BOOL CALLBACK callback(HMONITOR const monitor, HDC, LPRECT, LPARAM const data) {
                auto const context = reinterpret_cast<Context*>(data);
                context->list.emplace_back(monitor);
                return TRUE;
            }
        };
        Context context{};
        if (!EnumDisplayMonitors(nullptr, nullptr, &Context::callback, reinterpret_cast<LPARAM>(&context))) {
            return false;
        }
        *count = context.list.size();
        if (output) {
            for (size_t i = 0; i < context.list.size(); i += 1) {
                output[i] = new Display(context.list.at(i));
            }
        }
        return true;
    }
    bool IDisplay::getPrimary(IDisplay** const output) {
        assert(output != nullptr);
        struct Context {
            HMONITOR primary{};
            static BOOL CALLBACK callback(HMONITOR const monitor, HDC, LPRECT, LPARAM const data) {
                auto const context = reinterpret_cast<Context*>(data);
                if (auto const info = getMonitorInfo(monitor); info.dwFlags & MONITORINFOF_PRIMARY) {
                    context->primary = monitor;
                }
                return TRUE;
            };
        };
        Context context{};
        if (!EnumDisplayMonitors(nullptr, nullptr, &Context::callback, reinterpret_cast<LPARAM>(&context))) {
            return false;
        }
        *output = new Display(context.primary);
        return true;
    }
    bool IDisplay::getNearestFromWindow(IWindow* const window, IDisplay** const output) {
        assert(window != nullptr);
        assert(output != nullptr);
        auto const monitor = MonitorFromWindow(static_cast<HWND>(window->getNativeHandle()), MONITOR_DEFAULTTOPRIMARY);
        if (!monitor) {
            return false;
        }
        *output = new Display(monitor);
        return true;
    }
}
