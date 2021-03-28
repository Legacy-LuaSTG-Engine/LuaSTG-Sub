#pragma once
#include "Graphic/DeviceState.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d11_4.h>

namespace slow::Graphic
{
    class SamplerState : public Object<ISamplerState>
    {
    private:
        ComPtr<ID3D11SamplerState> _obj;
        DSamplerState _def;
    public:
        handle_t getHandle() { return (handle_t)_obj.Get(); };
        DSamplerState getDefinition() { return _def; };
    public:
        SamplerState(const DSamplerState& d, ID3D11SamplerState* p) : _obj(p), _def(d) {};
        virtual ~SamplerState() {};
    };
    
    class DepthStencilState : public Object<IDepthStencilState>
    {
    private:
        ComPtr<ID3D11DepthStencilState> _obj;
        DDepthStencilState _def;
    public:
        handle_t getHandle() { return (handle_t)_obj.Get(); };
        DDepthStencilState getDefinition() { return _def; };
    public:
        DepthStencilState(const DDepthStencilState& d, ID3D11DepthStencilState* p) : _obj(p), _def(d) {};
        virtual ~DepthStencilState() {};
    };
    
    class BlendState : public Object<IBlendState>
    {
    private:
        ComPtr<ID3D11BlendState> _obj;
        DBlendState _def;
    public:
        handle_t getHandle() { return (handle_t)_obj.Get(); };
        DBlendState getDefinition() { return _def; };
    public:
        BlendState(const DBlendState& d, ID3D11BlendState* p) : _obj(p), _def(d) {};
        virtual ~BlendState() {};
    };
};
