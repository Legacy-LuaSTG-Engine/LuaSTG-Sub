#pragma once

#include <cstddef>
#include <cstdint>

#include <d3d11_1.h>
#include <wincodec.h>

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"windowscodecs.lib")

namespace DirectX
{
#ifndef QOI_LOADER_FLAGS_DEFINED
#define QOI_LOADER_FLAGS_DEFINED

    enum QOI_LOADER_FLAGS : uint32_t
    {
        QOI_LOADER_DEFAULT = 0,
        QOI_LOADER_FORCE_SRGB = 0x1,
        QOI_LOADER_IGNORE_SRGB = 0x2,
        QOI_LOADER_FIT_POW2 = 0x20,
        QOI_LOADER_MAKE_SQUARE = 0x40,
    };

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#endif

    DEFINE_ENUM_FLAG_OPERATORS(QOI_LOADER_FLAGS);

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

    HRESULT CreateQOITextureFromMemoryEx(
        _In_ ID3D11Device* d3dDevice,
        _In_opt_ ID3D11DeviceContext* d3dContext,
        _In_ IWICImagingFactory* wicFactory,
        _In_reads_bytes_(qoiDataSize) const uint8_t* qoiData,
        _In_ size_t qoiDataSize,
        _In_ size_t maxsize,
        _In_ D3D11_USAGE usage,
        _In_ unsigned int bindFlags,
        _In_ unsigned int cpuAccessFlags,
        _In_ unsigned int miscFlags,
        _In_ QOI_LOADER_FLAGS loadFlags,
        _Outptr_opt_ ID3D11Resource** texture,
        _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept;
}
