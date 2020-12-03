#include "slow/slow.hpp"
#include "RefImpl.hpp"
#include <cassert>
#include "Win32Common.h"

namespace slow {
    class FrameworkImpl : public RefImpl<Framework>, public WindowCallback {
    private:
        FrameworkCallback* _callback;
        Pointer<FileManager> _file_manager;
        Pointer<Window> _window;
        bool _exit;
    private:
        void _onUpdate() {
            if (nullptr != _callback) {
                if (!_callback->onUpdate()) {
                    _exit = true;
                }
                _callback->onDraw();
            }
        }
        void onTimer() {
            _onUpdate();
        }
        void onSize(uint32_t width, uint32_t height) {
        }
    public:
        void setCallback(FrameworkCallback* callback) {
            _callback = callback;
        }
        
        FileManager* getFileManager() {
            return *_file_manager;
        }
        Window* getWindow() {
            return *_window;
        }
        
        bool create() {
            // create component
            _file_manager.reset();
            if (!FileManager::create(&_file_manager)) {
                return false;
            }
            _window.reset();
            if (!Window::create(&_window)) {
                return false;
            }
            // create window
            if (!_window->open("Slow Framework", Vector2I { 800, 600 }, WindowStyle::Normal, WindowLayer::Normal)) {
                return false;
            }
            _window->setCallback(this);
            // callback
            if (nullptr != _callback) {
                if (!_callback->onCreate()) {
                    return false;
                }
            }
            return true;
        }
        void run() {
            while (!_exit) {
                if (!_window->dispatchMessage()) {
                    exit();
                }
                _onUpdate();
            }
        }
        void destroy() {
            if (nullptr != _callback) {
                _callback->onDestroy();
                _callback = nullptr;
            }
            _window.reset();
            _file_manager.reset();
        }
        
        void exit() {
            _exit = true;
        }
    public:
        FrameworkImpl() : _callback(nullptr), _exit(false) {
            DWORD flag = COINIT_MULTITHREADED;
            #ifdef NDEBUG
            flag |= COINIT_SPEED_OVER_MEMORY;
            #endif
            CoInitializeEx(nullptr, flag);
            timeBeginPeriod(1);
        }
        virtual ~FrameworkImpl() {
            destroy();
            timeEndPeriod(1);
            CoUninitialize();
        }
    };
};

using namespace slow;

bool slowFramework(Framework** output) {
    assert(output != nullptr);
    if (output != nullptr) {
        try {
            FrameworkImpl* p = new FrameworkImpl;
            *output = (Framework*)p;
            return true;
        }
        catch (...) {}
    }
    return false;
}
