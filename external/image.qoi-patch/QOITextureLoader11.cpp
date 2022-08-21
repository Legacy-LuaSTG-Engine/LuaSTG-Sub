#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <memory>

#ifdef QOI_IMPLEMENTATION
#undef QOI_IMPLEMENTATION
#endif
#include "qoi.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "QOITextureLoader11.h"

namespace DirectX
{
    using Microsoft::WRL::ComPtr;

    struct auto_free
    {
        void*& _data;
        auto_free(void*& data) : _data(data) {}
        ~auto_free() { if (_data) std::free(_data); _data = NULL; }
    };

    // { CODE FROM DirectXTex

    void FitPowerOf2(UINT origx, UINT origy, UINT& targetx, UINT& targety, size_t maxsize)
    {
        const float origAR = float(origx) / float(origy);

        if (origx > origy)
        {
            size_t x;
            for (x = maxsize; x > 1; x >>= 1) { if (x <= targetx) break; }
            targetx = UINT(x);

            float bestScore = FLT_MAX;
            for (size_t y = maxsize; y > 0; y >>= 1)
            {
                const float score = fabsf((float(x) / float(y)) - origAR);
                if (score < bestScore)
                {
                    bestScore = score;
                    targety = UINT(y);
                }
            }
        }
        else
        {
            size_t y;
            for (y = maxsize; y > 1; y >>= 1) { if (y <= targety) break; }
            targety = UINT(y);

            float bestScore = FLT_MAX;
            for (size_t x = maxsize; x > 0; x >>= 1)
            {
                const float score = fabsf((float(x) / float(y)) - origAR);
                if (score < bestScore)
                {
                    bestScore = score;
                    targetx = UINT(x);
                }
            }
        }
    }

    // } CODE FROM DirectXTex

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
        _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept
    {
        // { CODE FROM DirectXTex

        if (texture)
        {
            *texture = nullptr;
        }
        if (textureView)
        {
            *textureView = nullptr;
        }

        if (!d3dDevice || !wicFactory || !qoiData || (!texture && !textureView))
        {
            return E_INVALIDARG;
        }

        if (textureView && !(bindFlags & D3D11_BIND_SHADER_RESOURCE))
        {
            return E_INVALIDARG;
        }

        if (!qoiDataSize)
            return E_FAIL;

        if (qoiDataSize > UINT32_MAX)
            return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);

        if (maxsize > UINT32_MAX)
            return E_INVALIDARG;

        // } CODE FROM DirectXTex

        qoi_desc desc = {};
        void* p_data = qoi_decode(qoiData, qoiDataSize, &desc, 4); // force RGBA
        if (NULL == p_data)
            return E_INVALIDARG;

        auto_free auto_free_data(p_data);

        // { CODE FROM DirectXTex

        HRESULT hr = S_OK;

        assert(desc.width > 0 && desc.height > 0);

        if (!maxsize)
        {
            // This is a bit conservative because the hardware could support larger textures than
            // the Feature Level defined minimums, but doing it this way is much easier and more
            // performant for WIC than the 'fail and retry' model used by DDSTextureLoader

            switch (d3dDevice->GetFeatureLevel())
            {
            case D3D_FEATURE_LEVEL_9_1:
            case D3D_FEATURE_LEVEL_9_2:
                maxsize = 2048u /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            case D3D_FEATURE_LEVEL_9_3:
                maxsize = 4096u /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            case D3D_FEATURE_LEVEL_10_0:
            case D3D_FEATURE_LEVEL_10_1:
                maxsize = 8192u /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            default:
                maxsize = size_t(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
                break;
            }
        }

        assert(maxsize > 0);

        UINT twidth = desc.width;
        UINT theight = desc.height;
        if (loadFlags & QOI_LOADER_FIT_POW2)
        {
            FitPowerOf2(desc.width, desc.height, twidth, theight, maxsize);
        }
        else if (desc.width > maxsize || desc.height > maxsize)
        {
            const float ar = static_cast<float>(desc.height) / static_cast<float>(desc.width);
            if (desc.width > desc.height)
            {
                twidth = static_cast<UINT>(maxsize);
                theight = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) * ar));
            }
            else
            {
                theight = static_cast<UINT>(maxsize);
                twidth = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) / ar));
            }
            assert(twidth <= maxsize && theight <= maxsize);
        }

        if (loadFlags & QOI_LOADER_MAKE_SQUARE)
        {
            twidth = std::max<UINT>(twidth, theight);
            theight = twidth;
        }

        size_t bpp = 32; // always RGBA32
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM; // always RGBA32

        // Handle sRGB formats
        if (loadFlags & QOI_LOADER_FORCE_SRGB)
        {
            format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }
        else if (!(loadFlags & QOI_LOADER_IGNORE_SRGB))
        {
            if (QOI_SRGB == desc.colorspace)
            {
                format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            }
        }

        // Allocate temporary memory for image
        const uint64_t rowBytes = (uint64_t(twidth) * uint64_t(bpp) + 7u) / 8u;
        const uint64_t numBytes = rowBytes * uint64_t(theight);

        if (rowBytes > UINT32_MAX || numBytes > UINT32_MAX)
            return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

        auto const rowPitch = static_cast<size_t>(rowBytes);
        auto const imageSize = static_cast<size_t>(numBytes);

        std::unique_ptr<uint8_t[]> temp;
        
        // Resize
        if (twidth != desc.width || theight != desc.height)
        {
            ComPtr<IWICBitmap> frame;
            hr = wicFactory->CreateBitmapFromMemory(
                desc.width, desc.height,
                GUID_WICPixelFormat32bppRGBA,
                desc.width * 4u, desc.width * desc.height * 4u, (BYTE*)p_data,
                &frame);
            if (FAILED(hr))
                return hr;

            ComPtr<IWICBitmapScaler> scaler;
            hr = wicFactory->CreateBitmapScaler(scaler.GetAddressOf());
            if (FAILED(hr))
                return hr;

            hr = scaler->Initialize(frame.Get(), twidth, theight, WICBitmapInterpolationModeFant);
            if (FAILED(hr))
                return hr;

            WICPixelFormatGUID pfScaler;
            hr = scaler->GetPixelFormat(&pfScaler);
            if (FAILED(hr))
                return hr;

            temp = std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[imageSize]);
            if (!temp)
                return E_OUTOFMEMORY;

            hr = scaler->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
            if (FAILED(hr))
                return hr;
        }

        // See if format is supported for auto-gen mipmaps (varies by feature level)
        bool autogen = false;
        if (d3dContext && textureView) // Must have context and shader-view to auto generate mipmaps
        {
            UINT fmtSupport = 0;
            hr = d3dDevice->CheckFormatSupport(format, &fmtSupport);
            if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
            {
                autogen = true;
            }
        }

        // Create texture
        D3D11_TEXTURE2D_DESC tex2d_desc = {};
        tex2d_desc.Width = twidth;
        tex2d_desc.Height = theight;
        tex2d_desc.MipLevels = (autogen) ? 0u : 1u;
        tex2d_desc.ArraySize = 1;
        tex2d_desc.Format = format;
        tex2d_desc.SampleDesc.Count = 1;
        tex2d_desc.SampleDesc.Quality = 0;
        tex2d_desc.Usage = usage;
        tex2d_desc.CPUAccessFlags = cpuAccessFlags;

        if (autogen)
        {
            tex2d_desc.BindFlags = bindFlags | D3D11_BIND_RENDER_TARGET;
            tex2d_desc.MiscFlags = miscFlags | D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
        else
        {
            tex2d_desc.BindFlags = bindFlags;
            tex2d_desc.MiscFlags = miscFlags;
        }

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = temp.get() ? temp.get() : p_data;
        initData.SysMemPitch = static_cast<UINT>(rowPitch);
        initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

        ID3D11Texture2D* tex = nullptr;
        hr = d3dDevice->CreateTexture2D(&tex2d_desc, (autogen) ? nullptr : &initData, &tex);
        if (SUCCEEDED(hr) && tex)
        {
            if (textureView)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
                SRVDesc.Format = tex2d_desc.Format;

                SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                SRVDesc.Texture2D.MipLevels = (autogen) ? unsigned(-1) : 1u;

                hr = d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);
                if (FAILED(hr))
                {
                    tex->Release();
                    return hr;
                }

                if (autogen)
                {
                    assert(d3dContext != nullptr);
                    d3dContext->UpdateSubresource(tex, 0, nullptr, initData.pSysMem, initData.SysMemPitch, initData.SysMemSlicePitch);
                    d3dContext->GenerateMips(*textureView);
                }
            }

            if (texture)
            {
                *texture = tex;
            }
            else
            {
                tex->Release();
            }
        }

        // } CODE FROM DirectXTex

        return S_OK;
    }
}
