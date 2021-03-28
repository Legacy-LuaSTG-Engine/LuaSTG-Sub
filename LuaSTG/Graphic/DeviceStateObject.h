#pragma once
#include "Graphic/DeviceState.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d11_4.h>

namespace slow::Graphic
{
    class RasterizerState : public Object<IRasterizerState>
    {
    private:
        ComPtr<ID3D11RasterizerState> _obj;
        DRasterizerState _def;
    public:
        handle_t getHandle() override { return (handle_t)_obj.Get(); };
        DRasterizerState getDefinition() override { return _def; };
    public:
        RasterizerState(const DRasterizerState& d, ID3D11RasterizerState* p) : _obj(p), _def(d) {};
        ~RasterizerState() override = default;
    };
    
    class SamplerState : public Object<ISamplerState>
    {
    private:
        ComPtr<ID3D11SamplerState> _obj;
        DSamplerState _def;
    public:
        handle_t getHandle() override { return (handle_t)_obj.Get(); };
        DSamplerState getDefinition() override { return _def; };
    public:
        SamplerState(const DSamplerState& d, ID3D11SamplerState* p) : _obj(p), _def(d) {};
        ~SamplerState() override = default;
    };
    
    class DepthStencilState : public Object<IDepthStencilState>
    {
    private:
        ComPtr<ID3D11DepthStencilState> _obj;
        DDepthStencilState _def;
    public:
        handle_t getHandle() override { return (handle_t)_obj.Get(); };
        DDepthStencilState getDefinition() override { return _def; };
    public:
        DepthStencilState(const DDepthStencilState& d, ID3D11DepthStencilState* p) : _obj(p), _def(d) {};
        ~DepthStencilState() override = default;
    };
    
    class BlendState : public Object<IBlendState>
    {
    private:
        ComPtr<ID3D11BlendState> _obj;
        DBlendState _def;
    public:
        handle_t getHandle() override { return (handle_t)_obj.Get(); };
        DBlendState getDefinition() override { return _def; };
    public:
        BlendState(const DBlendState& d, ID3D11BlendState* p) : _obj(p), _def(d) {};
        ~BlendState() override = default;
    };
};
