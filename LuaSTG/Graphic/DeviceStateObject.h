#pragma once
#include "Graphic/DeviceState.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d11_4.h>

namespace slow::Graphic
{
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
