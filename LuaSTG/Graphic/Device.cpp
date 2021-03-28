#include "Graphic/Device.h"
#include <cassert>
#include <string>
#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>

namespace slow::Graphic {
    using f_CreateDXGIFactory1 = HRESULT (WINAPI*)(
        REFIID riid,
        void** ppFactory
    );
    using f_D3D11CreateDevice = HRESULT (WINAPI*)(
        IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        ID3D11Device** ppDevice,
        D3D_FEATURE_LEVEL* pFeatureLevel,
        ID3D11DeviceContext** ppImmediateContext
    );
    
    using Microsoft::WRL::ComPtr;
}

#define self (*implememt)

namespace slow::Graphic {
    struct DeviceContext::Implement {
        ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
        ComPtr<ID3D11RasterizerState> d3d11RasterizerState;
        ComPtr<ID3D11SamplerState> d3d11SamplerStatePS[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        ComPtr<ID3D11DepthStencilState> d3d11DepthStencilState;
        ComPtr<ID3D11BlendState> d3d11BlendState;
        FLOAT d3d11BlendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        
        void reset() {
            d3d11DeviceContext.Reset();
            resetState();
        };
        
        void resetState() {
            d3d11RasterizerState.Reset();
            for (auto& v : d3d11SamplerStatePS) {
                v.Reset();
            }
            d3d11DepthStencilState.Reset();
            d3d11BlendState.Reset();
            d3d11BlendFactor[0] = 0.0f;
            d3d11BlendFactor[1] = 0.0f;
            d3d11BlendFactor[2] = 0.0f;
            d3d11BlendFactor[3] = 0.0f;
        };
    };
    
    void DeviceContext::setRasterizerState(IRasterizerState* p) {
        if (self.d3d11RasterizerState.Get() != (ID3D11RasterizerState*) p->getHandle()) {
            self.d3d11RasterizerState = (ID3D11RasterizerState*) p->getHandle();
            if (self.d3d11DeviceContext) {
                self.d3d11DeviceContext->RSSetState(self.d3d11RasterizerState.Get());
            }
        }
    }
    
    void DeviceContext::setPixelShaderSampler(uint32_t slot, ISamplerState* p) {
        assert(slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
        if (self.d3d11SamplerStatePS[slot].Get() != (ID3D11SamplerState*) p->getHandle()) {
            self.d3d11SamplerStatePS[slot] = (ID3D11SamplerState*) p->getHandle();
            if (self.d3d11DeviceContext) {
                ID3D11SamplerState* const sampler_[1] = {self.d3d11SamplerStatePS[slot].Get()};
                self.d3d11DeviceContext->PSSetSamplers(slot, 1, sampler_);
            }
        }
    }
    
    void DeviceContext::setDepthStencilState(IDepthStencilState* p) {
        if (self.d3d11DepthStencilState.Get() != (ID3D11DepthStencilState*) p->getHandle()) {
            self.d3d11DepthStencilState = (ID3D11DepthStencilState*) p->getHandle();
            if (self.d3d11DeviceContext) {
                self.d3d11DeviceContext->OMSetDepthStencilState(self.d3d11DepthStencilState.Get(), 0);
            }
        }
    }
    
    void DeviceContext::setBlendState(IBlendState* p) {
        if (self.d3d11BlendState.Get() != (ID3D11BlendState*) p->getHandle()) {
            self.d3d11BlendState = (ID3D11BlendState*) p->getHandle();
            if (self.d3d11DeviceContext) {
                self.d3d11DeviceContext->OMSetBlendState(self.d3d11BlendState.Get(), self.d3d11BlendFactor, 0xFFFFFFFF);
            }
        }
    }
    
    void DeviceContext::setBlendFactor(float r, float g, float b, float a) {
        if (self.d3d11BlendFactor[0] != r ||
            self.d3d11BlendFactor[1] != g ||
            self.d3d11BlendFactor[2] != b ||
            self.d3d11BlendFactor[3] != a) {
            self.d3d11BlendFactor[0] = r;
            self.d3d11BlendFactor[1] = g;
            self.d3d11BlendFactor[2] = b;
            self.d3d11BlendFactor[3] = a;
            if (self.d3d11DeviceContext) {
                self.d3d11DeviceContext->OMSetBlendState(self.d3d11BlendState.Get(), self.d3d11BlendFactor, 0xFFFFFFFF);
            }
        }
    }
    
    DeviceContext::DeviceContext() {
        implememt = new Implement;
    }
    
    DeviceContext::~DeviceContext() {
        delete implememt;
    }
}

namespace slow::Graphic {
    struct Device::Implement {
        // window
        HWND window = nullptr;
        // dll
        HMODULE dxgi = nullptr;
        HMODULE d3d11 = nullptr;
        f_CreateDXGIFactory1 CreateDXGIFactory1 = nullptr;
        f_D3D11CreateDevice D3D11CreateDevice = nullptr;
        // DXGI
        ComPtr<IDXGIFactory1> dxgiFactory1;
        ComPtr<IDXGIFactory2> dxgiFactory2;
        ComPtr<IDXGISwapChain> dxgiSwapChain;
        ComPtr<IDXGISwapChain1> dxgiSwapChain1;
        // Direct3D11
        ComPtr<ID3D11Device> d3d11Device;
        ComPtr<ID3D11Device1> d3d11Device1;
        ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
        ComPtr<ID3D11DeviceContext1> d3d11DeviceContext1;
        ComPtr<ID3D11RenderTargetView> d3d11BackBuffer;
        ComPtr<ID3D11DepthStencilView> d3d11DepthStencil;
        // class
        DeviceContext vContext;
    };
    
    handle_t Device::getHandle() {
        return (handle_t) self.d3d11Device.Get();
    }
    
    DeviceContext& Device::getContext() {
        return self.vContext;
    }
    
    bool Device::autoResizeSwapChain() {
        // check
        if (!self.dxgiSwapChain) {
            return false;
        }
        
        // get window size
        RECT rect_ = {};
        if (FALSE == ::GetClientRect(self.window, &rect_)) {
            return false;
        }
        UINT new_width_ = rect_.right - rect_.left;
        UINT new_height_ = rect_.bottom - rect_.top;
        new_width_ = (new_width_ > 0) ? new_width_ : 1;
        new_height_ = (new_height_ > 0) ? new_height_ : 1;
        
        HRESULT hr;
        
        // get size
        UINT old_width_;
        UINT old_height_;
        if (self.dxgiSwapChain1) {
            DXGI_SWAP_CHAIN_DESC1 lastinfo_ = {};
            hr = self.dxgiSwapChain1->GetDesc1(&lastinfo_);
            if (hr != S_OK) {
                return false;
            }
            old_width_ = lastinfo_.Width;
            old_height_ = lastinfo_.Height;
        }
        else {
            DXGI_SWAP_CHAIN_DESC lastinfo_ = {};
            hr = self.dxgiSwapChain->GetDesc(&lastinfo_);
            if (hr != S_OK) {
                return false;
            }
            old_width_ = lastinfo_.BufferDesc.Width;
            old_height_ = lastinfo_.BufferDesc.Height;
        }
        
        // resize
        if (new_width_ != old_width_ || new_height_ != old_height_) {
            return resizeSwapChain(new_width_, new_height_);
        }
        
        return true;
    }
    
    bool Device::resizeSwapChain(uint32_t width, uint32_t height) {
        // check
        if (width == 0 || height == 0) {
            return false;
        }
        if (!self.dxgiSwapChain) {
            return false;
        }
        
        // clear
        self.vContext.implememt->resetState();
        if (self.d3d11DeviceContext) {
            self.d3d11DeviceContext->ClearState();
        }
        self.d3d11BackBuffer.Reset();
        self.d3d11DepthStencil.Reset();
        
        HRESULT hr;
        
        // resize
        if (self.dxgiSwapChain1) {
            DXGI_SWAP_CHAIN_DESC1 lastinfo_ = {};
            hr = self.dxgiSwapChain1->GetDesc1(&lastinfo_);
            if (hr != S_OK) {
                return false;
            }
            hr = self.dxgiSwapChain1->ResizeBuffers(lastinfo_.BufferCount, width, height, lastinfo_.Format,
                                                    lastinfo_.Flags);
            if (hr != S_OK) {
                return false;
            }
        }
        else {
            DXGI_SWAP_CHAIN_DESC lastinfo_ = {};
            hr = self.dxgiSwapChain->GetDesc(&lastinfo_);
            if (hr != S_OK) {
                return false;
            }
            hr = self.dxgiSwapChain->ResizeBuffers(lastinfo_.BufferCount, width, height, lastinfo_.BufferDesc.Format,
                                                   lastinfo_.Flags);
            if (hr != S_OK) {
                return false;
            }
        }
        
        // create backbuffer
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer_;
        hr = self.dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer_.GetAddressOf()));
        if (hr != S_OK) {
            return false;
        }
        hr = self.d3d11Device->CreateRenderTargetView(backbuffer_.Get(), nullptr, self.d3d11BackBuffer.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        // create depth stencil
        Microsoft::WRL::ComPtr<ID3D11Texture2D> dsbuffer_;
        D3D11_TEXTURE2D_DESC dsinfo_ = {};
        {
            dsinfo_.Width = width;
            dsinfo_.Height = height;
            dsinfo_.MipLevels = 1;
            dsinfo_.ArraySize = 1;
            dsinfo_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsinfo_.SampleDesc.Count = 1;
            dsinfo_.SampleDesc.Quality = 0;
            dsinfo_.Usage = D3D11_USAGE_DEFAULT;
            dsinfo_.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            dsinfo_.CPUAccessFlags = 0;
            dsinfo_.MiscFlags = 0;
        }
        hr = self.d3d11Device->CreateTexture2D(&dsinfo_, nullptr, dsbuffer_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        hr = self.d3d11Device->CreateDepthStencilView(dsbuffer_.Get(), nullptr, self.d3d11DepthStencil.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        return true;
    }
    
    void Device::setSwapChain() {
        if (self.d3d11DeviceContext) {
            ID3D11RenderTargetView* const rts_[1] = {self.d3d11BackBuffer.Get()};
            self.d3d11DeviceContext->OMSetRenderTargets(1, rts_, self.d3d11DepthStencil.Get());
        }
    }
    
    void Device::clearRenderTarget(float r, float g, float b, float a) {
        if (self.d3d11DeviceContext) {
            ID3D11RenderTargetView* rts_[1] = {nullptr};
            self.d3d11DeviceContext->OMGetRenderTargets(1, rts_, nullptr);
            if (rts_[0]) {
                const FLOAT color_[4] = {r, g, b, a};
                self.d3d11DeviceContext->ClearRenderTargetView(rts_[0], color_);
                rts_[0]->Release();
            }
        }
    }
    
    void Device::clearDepthBuffer(float depth, uint8_t stencil) {
        if (self.d3d11DeviceContext) {
            ID3D11DepthStencilView* ds_ = nullptr;
            self.d3d11DeviceContext->OMGetRenderTargets(0, nullptr, &ds_);
            if (ds_) {
                self.d3d11DeviceContext->ClearDepthStencilView(ds_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
                                                               stencil);
                ds_->Release();
            }
        }
    }
    
    bool Device::updateSwapChain(bool vsync) {
        HRESULT hr;
        
        if (self.dxgiSwapChain) {
            hr = self.dxgiSwapChain->Present(vsync ? 1 : 0, 0);
            if (hr != S_OK && hr != DXGI_STATUS_OCCLUDED) {
                return false;
            }
        }
        else {
            // no swap chain
            return false;
        }
        
        return true;
    }
    
    bool Device::bind(handle_t window) {
        unbind();
        
        // setup window
        self.window = (HWND) window;
        
        // load module
        self.dxgi = ::LoadLibraryW(L"dxgi.dll");
        self.d3d11 = ::LoadLibraryW(L"d3d11.dll");
        if (self.dxgi == nullptr || self.d3d11 == nullptr) {
            return false;
        }
        self.CreateDXGIFactory1 = (f_CreateDXGIFactory1) ::GetProcAddress(self.dxgi, "CreateDXGIFactory1");
        self.D3D11CreateDevice = (f_D3D11CreateDevice) ::GetProcAddress(self.d3d11, "D3D11CreateDevice");
        if (self.CreateDXGIFactory1 == nullptr || self.D3D11CreateDevice == nullptr) {
            return false;
        }
        
        HRESULT hr;
        
        // create dxgi
        hr = self.CreateDXGIFactory1(IID_PPV_ARGS(self.dxgiFactory1.GetAddressOf()));
        if (hr != S_OK) {
            return false;
        }
        self.dxgiFactory1.As(&self.dxgiFactory2);
        ComPtr<IDXGIAdapter1> dxgiAdapter1_;
        ComPtr<IDXGIFactory6> dxgiFactory6_;
        self.dxgiFactory1.As(&dxgiFactory6_);
        if (dxgiFactory6_) {
            dxgiFactory6_->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                      IID_PPV_ARGS(dxgiAdapter1_.GetAddressOf()));
        }
        if (!dxgiAdapter1_) {
            hr = self.dxgiFactory1->EnumAdapters1(0, dxgiAdapter1_.GetAddressOf());
            if (hr != S_OK) {
                return false;
            }
        }
        {
            DXGI_ADAPTER_DESC1 desc_ = {};
            hr = dxgiAdapter1_->GetDesc1(&desc_);
            if (hr == S_OK) {
                std::wstring msg = L"current adapter: ";
                msg += desc_.Description;
                msg += L"\n";
                ::OutputDebugStringW(msg.c_str());
            }
        }
        
        // create d3d11
        UINT d3d11Flags_ = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        #ifdef _DEBUG
        d3d11Flags_ |= D3D11_CREATE_DEVICE_DEBUG;
        #endif
        const D3D_FEATURE_LEVEL target_levels_[4] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL feature_level_ = D3D_FEATURE_LEVEL_9_1;
        hr = self.D3D11CreateDevice(
            dxgiAdapter1_.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
            d3d11Flags_, target_levels_, 4, D3D11_SDK_VERSION,
            self.d3d11Device.GetAddressOf(), &feature_level_, self.d3d11DeviceContext.GetAddressOf());
        if (hr != S_OK) {
            hr = self.D3D11CreateDevice(
                dxgiAdapter1_.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                d3d11Flags_, target_levels_ + 1, 3, D3D11_SDK_VERSION,
                self.d3d11Device.GetAddressOf(), &feature_level_, self.d3d11DeviceContext.GetAddressOf());
            if (hr != S_OK) {
                return false;
            }
        }
        self.d3d11Device.As(&self.d3d11Device1);
        self.d3d11DeviceContext.As(&self.d3d11DeviceContext1);
        
        // create swapchain
        if (self.dxgiFactory2 && self.d3d11Device1 && self.d3d11DeviceContext1) {
            DXGI_SWAP_CHAIN_DESC1 scinfo_ = {};
            {
                scinfo_.Width = 1;
                scinfo_.Height = 1;
                scinfo_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                scinfo_.Stereo = FALSE;
                scinfo_.SampleDesc.Count = 1;
                scinfo_.SampleDesc.Quality = 0;
                scinfo_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                scinfo_.BufferCount = 2;
                scinfo_.Scaling = DXGI_SCALING_STRETCH;
                scinfo_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                scinfo_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                scinfo_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                //scinfo_.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                //scinfo_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            }
            hr = self.dxgiFactory2->CreateSwapChainForHwnd(self.d3d11Device1.Get(), self.window, &scinfo_, nullptr,
                                                           nullptr,
                                                           self.dxgiSwapChain1.GetAddressOf());
            if (hr == S_OK) {
                hr = self.dxgiSwapChain1.As(&self.dxgiSwapChain);
                if (hr != S_OK) {
                    self.dxgiSwapChain1.Reset(); // error?
                }
            }
        }
        if (!self.dxgiSwapChain) {
            DXGI_SWAP_CHAIN_DESC scinfo_ = {};
            {
                scinfo_.BufferDesc.Width = 1;
                scinfo_.BufferDesc.Height = 1;
                scinfo_.BufferDesc.RefreshRate.Numerator = 0;
                scinfo_.BufferDesc.RefreshRate.Denominator = 1;
                scinfo_.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                scinfo_.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                scinfo_.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
                scinfo_.SampleDesc.Count = 1;
                scinfo_.SampleDesc.Quality = 0;
                scinfo_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                scinfo_.BufferCount = 2;
                scinfo_.OutputWindow = self.window;
                scinfo_.Windowed = TRUE;
                scinfo_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                scinfo_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            }
            hr = self.dxgiFactory1->CreateSwapChain(self.d3d11Device.Get(), &scinfo_,
                                                    self.dxgiSwapChain.GetAddressOf());
            if (hr != S_OK) {
                return false;
            }
        }
        
        // unbind window
        hr = self.dxgiFactory1->MakeWindowAssociation(self.window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER |
                                                                   DXGI_MWA_NO_PRINT_SCREEN);
        if (hr != S_OK) {
            return false;
        }
        
        // bind context
        self.vContext.implememt->reset();
        self.vContext.implememt->d3d11DeviceContext = self.d3d11DeviceContext;
        
        return true;
    }
    
    void Device::unbind() {
        // unbind context
        self.vContext.implememt->reset();
        
        // destroy swapchain
        if (self.d3d11DeviceContext) {
            self.d3d11DeviceContext->ClearState();
        }
        self.d3d11BackBuffer.Reset();
        self.dxgiSwapChain.Reset();
        self.dxgiSwapChain1.Reset();
        
        // destroy dx
        self.dxgiFactory1.Reset();
        self.dxgiFactory2.Reset();
        self.d3d11Device.Reset();
        self.d3d11Device1.Reset();
        self.d3d11DeviceContext.Reset();
        self.d3d11DeviceContext1.Reset();
        self.d3d11DepthStencil.Reset();
        
        // dll
        if (self.dxgi) ::FreeLibrary(self.dxgi);
        if (self.d3d11) ::FreeLibrary(self.d3d11);
        self.dxgi = nullptr;
        self.d3d11 = nullptr;
        self.CreateDXGIFactory1 = nullptr;
        self.D3D11CreateDevice = nullptr;
        
        // window
        self.window = nullptr;
    }
    
    bool Device::validate() {
        if (!self.d3d11Device || !self.d3d11DeviceContext || !self.dxgiSwapChain) {
            return false;
        }
        return true;
    }
    
    Device::Device() {
        implememt = new Implement;
    }
    
    Device::~Device() {
        unbind();
        delete implememt;
    }
    
    Device& Device::get() {
        static Device instance;
        return instance;
    }
}

#include "Graphic/DeviceStateObject.h"

namespace slow::Graphic {
    bool Device::createRasterizerState(const DRasterizerState& def, IRasterizerState** pp) {
        if (!self.d3d11Device) {
            return false;
        }
        D3D11_RASTERIZER_DESC rs_;
        ZeroMemory(&rs_, sizeof(D3D11_RASTERIZER_DESC));
        rs_.FillMode = (D3D11_FILL_MODE) def.fillMode;
        rs_.CullMode = (D3D11_CULL_MODE) def.cullMode;
        rs_.FrontCounterClockwise = def.frontCounterClockwise ? TRUE : FALSE;
        rs_.DepthClipEnable = def.depthClipEnable ? TRUE : FALSE;
        rs_.ScissorEnable = def.scissorEnable ? TRUE : FALSE;
        rs_.MultisampleEnable = def.multiSampleEnable ? TRUE : FALSE;
        rs_.AntialiasedLineEnable = def.antialiasingLineEnable ? TRUE : FALSE;
        ComPtr<ID3D11RasterizerState> obj_;
        HRESULT hr = self.d3d11Device->CreateRasterizerState(&rs_, obj_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        try {
            auto* iobj_ = new RasterizerState(def, obj_.Get());
            *pp = dynamic_cast<IRasterizerState*>(iobj_);
            return true;
        }
        catch (...) {
            return false;
        }
    }
    
    bool Device::createSamplerState(const DSamplerState& def, ISamplerState** pp) {
        if (!self.d3d11Device) {
            return false;
        }
        D3D11_SAMPLER_DESC sampler_ = {};
        ZeroMemory(&sampler_, sizeof(D3D11_SAMPLER_DESC));
        sampler_.Filter = (D3D11_FILTER) def.filter;
        sampler_.AddressU = (D3D11_TEXTURE_ADDRESS_MODE) def.addressU;
        sampler_.AddressV = (D3D11_TEXTURE_ADDRESS_MODE) def.addressV;
        sampler_.AddressW = (D3D11_TEXTURE_ADDRESS_MODE) def.addressW;
        sampler_.MaxAnisotropy = def.maxAnisotropy;
        sampler_.BorderColor[0] = def.borderColor[0];
        sampler_.BorderColor[1] = def.borderColor[1];
        sampler_.BorderColor[2] = def.borderColor[2];
        sampler_.BorderColor[3] = def.borderColor[3];
        sampler_.MinLOD = -D3D11_FLOAT32_MAX;
        sampler_.MaxLOD = D3D11_FLOAT32_MAX;
        ComPtr<ID3D11SamplerState> obj_;
        HRESULT hr = self.d3d11Device->CreateSamplerState(&sampler_, obj_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        try {
            auto* iobj_ = new SamplerState(def, obj_.Get());
            *pp = dynamic_cast<ISamplerState*>(iobj_);
            return true;
        }
        catch (...) {
            return false;
        }
    }
    
    bool Device::createDepthStencilState(const DDepthStencilState& def, IDepthStencilState** pp) {
        if (!self.d3d11Device) {
            return false;
        }
        D3D11_DEPTH_STENCIL_DESC ds_ = {};
        ZeroMemory(&ds_, sizeof(D3D11_DEPTH_STENCIL_DESC));
        ds_.DepthEnable = def.depthEnable ? TRUE : FALSE;
        ds_.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK) def.depthWriteEnable;
        ds_.DepthFunc = (D3D11_COMPARISON_FUNC) def.depthFunction;
        ComPtr<ID3D11DepthStencilState> obj_;
        HRESULT hr = self.d3d11Device->CreateDepthStencilState(&ds_, obj_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        try {
            auto* iobj_ = new DepthStencilState(def, obj_.Get());
            *pp = dynamic_cast<IDepthStencilState*>(iobj_);
            return true;
        }
        catch (...) {
            return false;
        }
    }
    
    bool Device::createBlendState(const DBlendState& def, IBlendState** pp) {
        if (!self.d3d11Device) {
            return false;
        }
        D3D11_RENDER_TARGET_BLEND_DESC blend0_ = {};
        ZeroMemory(&blend0_, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
        blend0_.BlendEnable = def.enable ? TRUE : FALSE;
        blend0_.SrcBlend = (D3D11_BLEND) def.outputColor;
        blend0_.DestBlend = (D3D11_BLEND) def.bufferColor;
        blend0_.BlendOp = (D3D11_BLEND_OP) def.colorOperate;
        blend0_.SrcBlendAlpha = (D3D11_BLEND) def.outputAlpha;
        blend0_.DestBlendAlpha = (D3D11_BLEND) def.bufferAlpha;
        blend0_.BlendOpAlpha = (D3D11_BLEND_OP) def.alphaOperate;
        blend0_.RenderTargetWriteMask = (UINT8) def.writeEnable;
        D3D11_BLEND_DESC blend_ = {};
        ZeroMemory(&blend_, sizeof(D3D11_BLEND_DESC));
        blend_.AlphaToCoverageEnable = FALSE;
        blend_.IndependentBlendEnable = FALSE;
        blend_.RenderTarget[0] = blend0_;
        blend_.RenderTarget[1] = blend0_;
        blend_.RenderTarget[2] = blend0_;
        blend_.RenderTarget[3] = blend0_;
        blend_.RenderTarget[4] = blend0_;
        blend_.RenderTarget[5] = blend0_;
        blend_.RenderTarget[6] = blend0_;
        blend_.RenderTarget[7] = blend0_;
        ComPtr<ID3D11BlendState> obj_;
        HRESULT hr = self.d3d11Device->CreateBlendState(&blend_, obj_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        try {
            auto* iobj_ = new BlendState(def, obj_.Get());
            *pp = dynamic_cast<IBlendState*>(iobj_);
            return true;
        }
        catch (...) {
            return false;
        }
    }
}

#include "Graphic/DeviceResourceObject.h"
#include "WICTextureLoader/WICTextureLoader11.h"

namespace slow::Graphic {
    inline bool to_wstring(const std::string& u8str, std::wstring& wstr) {
        const int need_ = ::MultiByteToWideChar(CP_UTF8, 0,
                                                u8str.c_str(), u8str.length(),
                                                nullptr, 0);
        if (need_ < 1) {
            return false;
        }
        wstr.resize(need_);
        const int cvret_ = ::MultiByteToWideChar(CP_UTF8, 0,
                                                 u8str.c_str(), u8str.length(),
                                                 wstr.data(), need_);
        if (cvret_ < 1) {
            wstr.clear();
            return false;
        }
        return true;
    }
    
    bool Device::createTexture2D(const char* path, ITexture2D** pp) {
        if (path == nullptr || pp == nullptr) {
            return false;
        }
        if (!self.d3d11Device) {
            return false;
        }
        std::wstring wpath_;
        if (!to_wstring(path, wpath_)) {
            return false;
        }
        ComPtr<ID3D11Resource> texres_;
        ComPtr<ID3D11Texture2D> tex2d_;
        ComPtr<ID3D11ShaderResourceView> tex2d_srv_;
        HRESULT hr = DirectX::CreateWICTextureFromFileEx(
            self.d3d11Device.Get(),
            wpath_.c_str(),
            0,
            D3D11_USAGE_DEFAULT, // D3D11_USAGE_IMMUTABLE ?
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
            0,
            texres_.GetAddressOf(),
            tex2d_srv_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        hr = texres_.As(&tex2d_);
        if (hr != S_OK) {
            return false;
        }
        try {
            auto* iobj_ = new Texture2D(tex2d_.Get(), tex2d_srv_.Get());
            *pp = dynamic_cast<ITexture2D*>(iobj_);
            return true;
        }
        catch (...) {
            return false;
        }
    }
}
