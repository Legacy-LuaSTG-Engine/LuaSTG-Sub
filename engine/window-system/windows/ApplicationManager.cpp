#include "windows/ApplicationManager.hpp"
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>

namespace {
    struct ScopedTimePeriod {
        uint32_t period{};

        ScopedTimePeriod(const uint32_t target_period = 1u) {
            if (timeBeginPeriod(target_period) == TIMERR_NOERROR) {
                period = target_period;
            }
        }
        ~ScopedTimePeriod() {
            if (period > 0u) {
                timeEndPeriod(period);
            }
        }
    };

    core::IApplication* g_application{};
    HANDLE g_main_thread{};
    DWORD g_main_thread_id{};
    bool is_updating{};
    bool is_update_enabled{ true };
    bool is_delegate_update_enabled{ true };
}

namespace core {
    void ApplicationManager::run(IApplication* const application) {
        // Main thread & UI thread

        assert(application != nullptr);
        g_application = application;
        g_main_thread = GetCurrentThread();
        g_main_thread_id = GetCurrentThreadId();
        SetThreadAffinityMask(g_main_thread, 0x1);
        SetThreadPriority(g_main_thread, THREAD_PRIORITY_HIGHEST);
        const ScopedTimePeriod scoped_time_period;

        if (!application->onCreate()) {
            return;
        }

        bool running = true;
        MSG msg{};
        while (running) {
            if (is_update_enabled && !is_updating) {
                is_updating = true;
                application->onBeforeUpdate();
                is_updating = false;
            }

            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                }
                else {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }

            if (!running) {
                break;
            }

            if (is_update_enabled && !is_updating) {
                is_updating = true;
                const auto update_result = application->onUpdate();
                is_updating = false;
                if (!update_result) {
                    running = false;
                    break;
                }
            }
        }

        application->onDestroy();
    }

    IApplication* ApplicationManager::getApplication() {
        return g_application;
    }

    void ApplicationManager::requestExit() {
        PostThreadMessageW(g_main_thread_id, WM_QUIT, 0, 0);
    }

    bool ApplicationManager::isMainThread() {
        return GetCurrentThreadId() == g_main_thread_id;
    }

    bool ApplicationManager::isUpdating() {
        return is_updating;
    }

    void ApplicationManager::runBeforeUpdate() {
        if (!is_updating) {
            return;
        }
        is_updating = true;
        g_application->onBeforeUpdate();
        is_updating = false;
    }

    void ApplicationManager::runUpdate() {
        if (!is_updating) {
            return;
        }
        is_updating = true;
        const auto update_result = g_application->onUpdate();
        is_updating = false;
        if (!update_result) {
            requestExit();
        }
    }

    bool ApplicationManager::isUpdateEnabled() {
        return is_update_enabled;
    }

    void ApplicationManager::setUpdateEnabled(const bool enabled) {
        is_update_enabled = enabled;
    }

    bool ApplicationManager::isDelegateUpdateEnabled() {
        return is_delegate_update_enabled;
    }

    void ApplicationManager::setDelegateUpdateEnabled(const bool enabled) {
        is_delegate_update_enabled = enabled;
    }
}
