#pragma once
#include "Graphic/DeviceState.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d11_4.h>

namespace slow::Graphic {
    class Texture2D : public Object<ITexture2D> {
    private:
        D3D11_TEXTURE2D_DESC _info;
        ComPtr <ID3D11Texture2D> _tex2d;
        ComPtr <ID3D11ShaderResourceView> _srv;
    public:
        EResourceType getType() override { return EResourceType::Texture2D; }
        handle_t getHandle() override { return (handle_t)_srv.Get(); }
        uint32_t getWidth() override { return _info.Width; }
        uint32_t getHeight() override { return _info.Height; }
        uint32_t getDepth() override { return 1; }
    public:
        Texture2D(ID3D11Texture2D* tex2d, ID3D11ShaderResourceView* srv) : _info({}), _tex2d(tex2d), _srv(srv) {
            _tex2d->GetDesc(&_info);
        };
        ~Texture2D() override = default;
    };
}
