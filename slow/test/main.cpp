#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <clocale>
#include "slow/slow.hpp"
#include <Windows.h>
#include <wrl.h>

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")

#include <d3d11_1.h>

void test_slow_FileArchive() {
    using namespace slow;
    Pointer<FileArchive> zip;
    if (FileArchive::create(&zip)) {
        const char* path = u8"测试-zip的.jpg/测试zip.zip";
        if (zip->open(u8"测试-zip的.jpg/测试zip.zip")) {
            std::printf("load archive: %s\n", path);
            
            Pointer<FileList> flist;
            
            zip->setMode(FileArchiveMode::Root);
            flist.reset();
            if (zip->listFile(u8"", &flist)) {
                std::printf("Root mode:\n");
                for (int i = 0; i < flist->getSize(); i++) {
                    std::printf("[%d][%s] %s\n", i, flist->getType(i) == FileType::File ? "F" : "D", flist->getName(i));
                }
            }
            
            zip->setMode(FileArchiveMode::Relative);
            flist.reset();
            if (zip->listFile(u8"", &flist)) {
                std::printf("Relative mode:\n");
                for (int i = 0; i < flist->getSize(); i++) {
                    std::printf("[%d][%s] %s\n", i, flist->getType(i) == FileType::File ? "F" : "D", flist->getName(i));
                }
            }
            
            zip->setMode(FileArchiveMode::Associate);
            flist.reset();
            if (zip->listFile(u8"", &flist)) {
                std::printf("Associate mode:\n");
                for (int i = 0; i < flist->getSize(); i++) {
                    std::printf("[%d][%s] %s\n", i, flist->getType(i) == FileType::File ? "F" : "D", flist->getName(i));
                }
            }
        }
    }
}

void test_slow_FileManager() {
    using namespace slow;
    Pointer<FileManager> fmgr;
    if (FileManager::create(&fmgr)) {
        Pointer<FileList> flst;
        if (fmgr->listFile(L"", &flst)) {
            for (int i = 0; i < flst->getSize(); i++) {
                std::printf("[%d][%s] %s\n", i, flst->getType(i) == FileType::File ? "F" : "D", flst->getName(i));
            }
        }
        flst.reset();
        if (fmgr->listFile(L"测试 folder", &flst)) {
            for (int i = 0; i < flst->getSize(); i++) {
                std::printf("[%d][%s] %s\n", i, flst->getType(i) == FileType::File ? "F" : "D", flst->getName(i));
            }
        }
        flst.reset();
        if (fmgr->listFile(L"测试 folder/", &flst)) {
            for (int i = 0; i < flst->getSize(); i++) {
                std::printf("[%d][%s] %s\n", i, flst->getType(i) == FileType::File ? "F" : "D", flst->getName(i));
            }
        }
        Pointer<Data> data;
        if (fmgr->loadFile(u8"some 文本文件.txt", &data)) {
            std::cout << data->data() << std::endl;
        }
        data.reset();
        if (fmgr->loadFile(u8"some 文本文件 2.txt", &data)) {
            std::cout << data->data() << std::endl;
        }
        else {
            data.reset();
            std::printf(u8"加载失败，添加搜索路径后再尝试\n");
            fmgr->addSearchPath(u8"测试 folder/test 文件夹");
            if (fmgr->loadFileEx(u8"some 文本文件 2.txt", &data)) {
                std::cout << data->data() << std::endl;
            }
        }
    }
}

void test_slow_MonitorHelper() {
    using namespace slow;
    Pointer<MonitorHelper> mm;
    if (MonitorHelper::create(&mm)) {
        for (uint32_t i = 0; i < mm->getSize(); i++) {
            auto info = mm->getInfo(i);
            std::printf("%dx%d\n", info.work.right - info.work.left, info.work.bottom - info.work.top);
        }
    }
}

namespace slow {
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
    
    using namespace slow;
    
    class Device {
    private:
        // relate window
        Pointer<Window> _window;
        // version flag
        bool _dxgi_d3d_new = false;
        // dxgi 1.1 & d3d 11.0
        Microsoft::WRL::ComPtr<IDXGIFactory1> _dxgi_factory;
        Microsoft::WRL::ComPtr<IDXGISwapChain> _dxgi_swapchain;
        Microsoft::WRL::ComPtr<ID3D11Device> _d3d_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _d3d_device_context;
        // dxgi 1.2 & d3d 11.1
        Microsoft::WRL::ComPtr<IDXGIFactory2> _dxgi_factory_1;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> _dxgi_swapchain_1;
        Microsoft::WRL::ComPtr<ID3D11Device1> _d3d_device_1;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _d3d_device_context_1;
    private:
        bool _create_dxgi_factory() {
            HRESULT hr = S_OK;
            hr = ::CreateDXGIFactory1(IID_PPV_ARGS(_dxgi_factory.GetAddressOf()));
            if (S_OK != hr) {
                DebugOutput(L"[E] CreateDXGIFactory1 failed: %X", hr);
                return false;
            }
            DebugOutput(L"[I] CreateDXGIFactory1 succeed, create IDXGIFactory1 succeed");
            if (S_OK != _dxgi_factory.As(&_dxgi_factory_1)) {
                DebugOutput(L"[W] check IDXGIFactory2 failed, this device doens't support DXGI 1.2");
            }
            DebugOutput(L"[I] check IDXGIFactory2 succeed");
            return true;
        }
        bool _create_d3d_device() {
            HRESULT hr = S_OK;
            
            Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
            hr = _dxgi_factory->EnumAdapters1(0, adapter.GetAddressOf());
            if (S_OK != hr) {
                DebugOutput(L"[E] IDXGIFactory1::EnumAdapters1 failed: %X", hr);
                return false;
            }
            DebugOutput(L"[I] IDXGIFactory1::EnumAdapters1 succeed, get IDXGIAdapter1 0 succeed");
            
            UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
            #ifndef NDEBUG
            flag |= D3D11_CREATE_DEVICE_DEBUG;
            flag |= D3D11_CREATE_DEVICE_DEBUGGABLE;
            #endif
            const D3D_FEATURE_LEVEL target_level_new[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
            };
            const D3D_FEATURE_LEVEL target_level_old[] = {
                D3D_FEATURE_LEVEL_11_0,
            };
            D3D_FEATURE_LEVEL valid_level = D3D_FEATURE_LEVEL_9_1;
            
            hr = ::D3D11CreateDevice(
                    adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                    flag, target_level_new, 2, D3D11_SDK_VERSION,
                    _d3d_device.GetAddressOf(), &valid_level, _d3d_device_context.GetAddressOf());
            #ifndef NDEBUG
            if (S_OK != hr) {
                DebugOutput(L"[E] D3D11CreateDevice failed: %X, try disable flag D3D11_CREATE_DEVICE_DEBUGGABLE", hr);
                flag ^= D3D11_CREATE_DEVICE_DEBUGGABLE;
                hr = ::D3D11CreateDevice(
                    adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                    flag, target_level_new, 2, D3D11_SDK_VERSION,
                    _d3d_device.GetAddressOf(), &valid_level, _d3d_device_context.GetAddressOf());
            }
            #endif
            if (S_OK != hr) {
                DebugOutput(L"[E] D3D11CreateDevice failed: %X, try disable feature level D3D_FEATURE_LEVEL_11_1", hr);
                hr = ::D3D11CreateDevice(
                    adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                    flag, target_level_old, 1, D3D11_SDK_VERSION,
                    _d3d_device.GetAddressOf(), &valid_level, _d3d_device_context.GetAddressOf());
            }
            if (S_OK != hr) {
                DebugOutput(L"[E] D3D11CreateDevice failed: %X");
                return false;
            }
            DebugOutput(L"[I] D3D11CreateDevice succeed, create ID3D11Device and ID3D11DeviceContext succeed");
            if (valid_level != D3D_FEATURE_LEVEL_11_1) {
                DebugOutput(L"[W] this device doens't support feature level D3D_FEATURE_LEVEL_11_1");
            }
            
            if (S_OK == _d3d_device.As(&_d3d_device_1)) {
                _d3d_device_1->GetImmediateContext1(_d3d_device_context_1.GetAddressOf());
                DebugOutput(L"[I] check ID3D11Device1 and ID3D11DeviceContext1 succeed");
            }
            else {
                DebugOutput(L"[W] this device doens't support DirectX 11.1");
            }
            
            return true;
        }
        void _check_dxgi_d3d_version() {
            if (_dxgi_factory_1.Get() && _d3d_device_1.Get()) {
                DebugOutput(L"[I] this device support DXGI 1.2 and DirectX 11.1, will enable new feature");
                _dxgi_d3d_new = true;
            }
        }
        bool _create_dxgi_swapchain() {
            HRESULT hr = S_OK;
            if (_dxgi_d3d_new) {
                DXGI_SWAP_CHAIN_DESC1 desc;
                desc.Width = 640,
                desc.Height = 480,
                desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                desc.Stereo = FALSE;
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.BufferCount = 2;
                desc.Scaling = DXGI_SCALING_NONE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
                desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                
                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fdesc;
                fdesc.RefreshRate.Numerator = 0;
                fdesc.RefreshRate.Denominator = 1;
                fdesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                fdesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
                fdesc.Windowed = TRUE;
                
                hr = _dxgi_factory_1->CreateSwapChainForHwnd(
                        _d3d_device_1.Get(), (HWND)_window->getNativeWindow(),
                        &desc, &fdesc, nullptr,
                        _dxgi_swapchain_1.GetAddressOf());
                if (S_OK != hr) {
                    DebugOutput(L"[E] IDXGIFactory2::CreateSwapChainForHwnd failed: %X, "
                                L"try replace DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL to DXGI_SWAP_EFFECT_DISCARD", hr);
                    desc.BufferCount = 1;
                    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                    hr = _dxgi_factory_1->CreateSwapChainForHwnd(
                            _d3d_device_1.Get(), (HWND)_window->getNativeWindow(),
                            &desc, &fdesc, nullptr,
                            _dxgi_swapchain_1.GetAddressOf());
                }
                if (S_OK != hr) {
                    DebugOutput(L"[E] IDXGIFactory2::CreateSwapChainForHwnd failed: %X", hr);
                    return false;
                }
                DebugOutput(L"[I] IDXGIFactory2::CreateSwapChainForHwnd succeed, create IDXGISwapChain1 succeed");
                if (S_OK != _dxgi_swapchain_1.As(&_dxgi_swapchain)) {
                    DebugOutput(L"[W] check IDXGISwapChain failed");
                }
            }
            else {
                DXGI_SWAP_CHAIN_DESC desc;
                
                desc.BufferDesc.Width = 640;
                desc.BufferDesc.Height = 480;
                desc.BufferDesc.RefreshRate.Numerator = 0;
                desc.BufferDesc.RefreshRate.Denominator = 1;
                desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
                
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.BufferCount = 1;
                desc.OutputWindow = (HWND)_window->getNativeWindow();
                desc.Windowed = TRUE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                
                hr = _dxgi_factory->CreateSwapChain(_d3d_device.Get(), &desc, _dxgi_swapchain.GetAddressOf());
                if (S_OK != hr) {
                    DebugOutput(L"[E] IDXGIFactory1::CreateSwapChain failed: %X", hr);
                    return false;
                }
                DebugOutput(L"[I] IDXGIFactory1::CreateSwapChain succeed, create IDXGISwapChain succeed");
            }
            return true;
        }
    public:
        bool open(Window* window) {
            _window = window;
            if (!_create_dxgi_factory()) {
                return false;
            }
            if (!_create_d3d_device()) {
                return false;
            }
            _check_dxgi_d3d_version();
            if (!_create_dxgi_swapchain()) {
                return false;
            }
            return true;
        }
        void close() {
            _d3d_device_context_1.Reset();
            if (_d3d_device_context.Get()) {
                _d3d_device_context->ClearState();
                _d3d_device_context.Reset();
            }
        }
    public:
        Device() {}
        ~Device() {}
    };
};

void test_slow() {
    using namespace slow;
    
    static int t = 0;
    
    class Application : public FrameworkCallback {
    private:
        Pointer<Framework> _framework;
        slow::Device _device;
    public:
        bool onCreate() {
            auto window = _framework->getWindow();
            window->setStyle(WindowStyle::Normal);
            _device.open(window);
            return true;
        }
        void onDestroy() {
            
        }
        bool onUpdate() {
            Sleep(16);
            //printf("timer %d\n", t);
            t++;
            return true;
        }
        void onDraw() {
            
        }
    public:
        bool create() {
            _framework.reset();
            if (!slowFramework(&_framework)) {
                return false;
            }
            _framework->setCallback((FrameworkCallback*)this);
            if (!_framework->create()) {
                return false;
            }
            return true;
        }
        void run() {
            if (nullptr != *_framework) {
                _framework->run();
            }
        }
        void destory() {
            if (nullptr != *_framework) {
                _framework->destroy();
                _framework.reset();
            }
        }
    public:
        Application() {}
        ~Application() {}
    } app;
    
    if (app.create()) {
        app.run();
        app.destory();
    }
}

int main() {
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    
    std::setlocale(LC_ALL, "zh_CN.UTF-8");
    
    test_slow();
    
    std::system("pause");
    
    _CrtDumpMemoryLeaks();
    return 0;
}
