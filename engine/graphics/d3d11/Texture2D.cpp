#include "d3d11/Texture2D.hpp"
#include "core/Logger.hpp"
#include "core/FileSystem.hpp"
#include "core/Image.hpp"
#include "utf8.hpp"
#include "WICTextureLoader11.h"
#include "DDSTextureLoader11.h"
#include "ScreenGrab11.h"

namespace {
    using std::string_view_literals::operator ""sv;
}

namespace core {
    // ITexture2D

    bool Texture2D::setSize(Vector2U const size) {
        if (!m_dynamic) {
            Logger::error("[core] [Texture2D] read-only texture cannot be resized");
            assert(false); return false;
        }
        if (!m_is_render_target) {
            Logger::error("[core] [Texture2D] this texture is managed by RenderTarget and cannot be resized");
            assert(false); return false;
        }
        onGraphicsDeviceDestroy();
        m_size = size;
        return createResource();
    }
    bool Texture2D::update(RectU const rc, void const* const data, uint32_t const pitch) {
        if (!m_dynamic) {
            Logger::error("[core] [Texture2D] read-only texture cannot by modified");
            assert(false); return false;
        }
        assert(m_device->getNativeHandle());
        win32::com_ptr<ID3D11DeviceContext> ctx;
        static_cast<ID3D11Device*>(m_device->getNativeHandle())->GetImmediateContext(ctx.put());
        assert(m_texture);
        if (!ctx || !m_texture) {
            return false;
        }
        D3D11_BOX const box = {
            .left = rc.a.x,
            .top = rc.a.y,
            .front = 0,
            .right = rc.b.x,
            .bottom = rc.b.y,
            .back = 1,
        };
        ctx->UpdateSubresource(m_texture.get(), 0, &box, data, pitch, 0);
        return true;
    }
    bool Texture2D::saveToFile(StringView const path) {
        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return false;
        }

        const std::wstring path_wide(utf8::to_wstring(path));
        if (!win32::check_hresult_as_boolean(
            DirectX::SaveWICTextureToFile(
                ctx,
                m_texture.get(),
                GUID_ContainerFormatJpeg,
                path_wide.c_str(),
                &GUID_WICPixelFormat24bppBGR
            ),
            "DirectX::SaveWICTextureToFile"sv
        )) {
            return false;
        }

        return true;
    }

    // IGraphicsDeviceEventListener

    void Texture2D::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResource();
        }
    }
    void Texture2D::onGraphicsDeviceDestroy() {
        m_texture.reset();
        m_view.reset();
    }

    // Texture2D

    Texture2D::Texture2D() = default;
    Texture2D::~Texture2D() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool Texture2D::initialize(IGraphicsDevice* const device, StringView const path, bool const mipmap) {
        assert(device);
        assert(!path.empty());
        m_device = device;
        m_source_path = path;
        m_mipmap = mipmap;
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
    bool Texture2D::initialize(IGraphicsDevice* const device, IImage* const image, bool const mipmap) {
        // TODO: image must be read-only
        assert(device);
        assert(image);
        m_device = device;
        m_image = image;
        m_size = m_image->getSize();
        m_mipmap = mipmap;
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
    bool Texture2D::initialize(IGraphicsDevice* const device, Vector2U const size, bool const is_render_target) {
        assert(device);
        assert(size.x > 0 && size.y > 0);
        m_device = device;
        m_size = size;
        m_dynamic = true;
        m_pre_mul_alpha = is_render_target;  // 默认是预乘 alpha 的
        m_is_render_target = is_render_target;
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        if (!is_render_target) {
            // 由 RenderTarget 托管时，不注册监听器
            // 普通的动态纹理需要注册监听器
            m_device->addEventListener(this);
        }
        return true;
    }
    bool Texture2D::createResource() {
        if (m_image) {
            // from in memory image
            return createFromImage(m_image.get());
        }
        else if (!m_source_path.empty()) {
            // from file
            return createFromProvidedPath();
        }
        else {
            // dynamic texture / render target
            return createTextureAndView();
        }
    }

    bool Texture2D::createTextureAndView() {
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        D3D11_TEXTURE2D_DESC texture_info{};
        texture_info.Width = m_size.x;
        texture_info.Height = m_size.y;
        texture_info.MipLevels = 1;
        texture_info.ArraySize = 1;
        texture_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        texture_info.SampleDesc.Count = 1;
        texture_info.Usage = D3D11_USAGE_DEFAULT;
        texture_info.BindFlags = D3D11_BIND_SHADER_RESOURCE | (m_is_render_target ? D3D11_BIND_RENDER_TARGET : 0u);

        if (!win32::check_hresult_as_boolean(
            device->CreateTexture2D(&texture_info, nullptr, m_texture.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_info{};
        srv_info.Format = texture_info.Format;
        srv_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_info.Texture2D.MipLevels = texture_info.MipLevels;

        if (!win32::check_hresult_as_boolean(
            device->CreateShaderResourceView(m_texture.get(), &srv_info, m_view.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        return true;
    }
    bool Texture2D::createFromProvidedPath() {
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return false;
        }

        SmartReference<IData> data;
        if (!FileSystemManager::readFile(m_source_path, data.put())) {
            Logger::error("[core] [Texture2D] read file '{}' failed", m_source_path);
            return false;
        }

        win32::com_ptr<ID3D11Resource> resource;

        // DDS

        DirectX::DDS_ALPHA_MODE dds_alpha_mode = DirectX::DDS_ALPHA_MODE_UNKNOWN;
        const HRESULT dds_result = DirectX::CreateDDSTextureFromMemoryEx(
            device, m_mipmap ? ctx : nullptr,
            static_cast<uint8_t const*>(data->data()), data->size(),
            0,
            D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
            DirectX::DDS_LOADER_IGNORE_SRGB, // TODO: linear color space
            resource.put(), m_view.put(),
            &dds_alpha_mode
        );
        if (SUCCEEDED(dds_result)) {
            m_pre_mul_alpha = dds_alpha_mode == DirectX::DDS_ALPHA_MODE_PREMULTIPLIED;
        }

        // WIC

        HRESULT wic_result = E_FAIL;
        if (FAILED(dds_result)) {
            wic_result = DirectX::CreateWICTextureFromMemoryEx(
                device, m_mipmap ? ctx : nullptr,
                static_cast<uint8_t const*>(data->data()), data->size(),
                0,
                D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
                DirectX::WIC_LOADER_DEFAULT | DirectX::WIC_LOADER_IGNORE_SRGB, // TODO: linear color space
                resource.put(), m_view.put()
            );
        }

        // info

        if (resource && !m_texture) {
            if (!win32::check_hresult_as_boolean(
                resource->QueryInterface(m_texture.put()),
                "ID3D11Resource::QueryInterface (ID3D11Texture2D)"sv
            )) {
                return false;
            }
        }

        if (m_texture) {
            D3D11_TEXTURE2D_DESC texture_info{};
            m_texture->GetDesc(&texture_info);
            m_size.x = texture_info.Width;
            m_size.y = texture_info.Height;
        }

        // Image

        bool image_result = false;
        if (FAILED(dds_result) && FAILED(wic_result)) {
            image_result = createFromImage(data.get());
        }

        // summary

        if (FAILED(dds_result) && FAILED(wic_result) && !image_result) {
            Logger::error("[core] [Texture2D] load from file '{}' failed, not a valid JPEG, PNG, WebP, QOI, BMP or DDS format.", m_source_path);
            return false;
        }

        return true;
    }
    bool Texture2D::createFromImage(IData* const data) {
        if (data == nullptr) {
            assert(false); return false;
        }

        SmartReference<IImage> image;
        if (!ImageFactory::createFromData(data, image.put())) {
            return false;
        }

        return createFromImage(image.get());
    }
    bool Texture2D::createFromImage(IImage* const image) {
        if (image == nullptr) {
            assert(false); return false;
        }
        if (const auto size = image->getSize(); size.x > D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION || size.y > D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION) {
            if (!m_source_path.empty()) {
                Logger::error("[core] [Texture2D] load from file '{}' failed, image size ({}x{}) exceeds the size limit ({}x{})",
                    m_source_path,
                    size.x, size.y,
                    D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION
                );
            }
            else {
                Logger::error("[core] [Texture2D] image size ({}x{}) exceeds the size limit ({}x{})",
                    size.x, size.y,
                    D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION
                );
            }
            assert(false); return false;
        }

        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }

        const auto ctx = static_cast<ID3D11DeviceContext*>(m_device->getCommandbuffer()->getNativeHandle());
        if (ctx == nullptr) {
            assert(false); return false;
        }

        // TODO: handle sRGB
        // TODO: generate mipmap

        D3D11_TEXTURE2D_DESC texture_info{};
        texture_info.Width = image->getSize().x;
        texture_info.Height = image->getSize().y;
        texture_info.MipLevels = 1;
        texture_info.ArraySize = 1;
        switch (image->getFormat()) {
        case ImageFormat::r8g8b8a8_normalized:
            texture_info.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case ImageFormat::b8g8r8a8_normalized:
            texture_info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;
        case ImageFormat::r16g16b16a16_float:
            texture_info.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            break;
        case ImageFormat::r32g32b32a32_float:
            texture_info.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;
        }
        texture_info.SampleDesc.Count = 1;
        texture_info.Usage = D3D11_USAGE_DEFAULT;
        texture_info.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (m_mipmap) {
            texture_info.MipLevels = 0;
            texture_info.BindFlags |= D3D11_BIND_RENDER_TARGET;
            texture_info.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        core::ScopedImageMappedBuffer image_buffer{};
        if (!image->createScopedMap(image_buffer)) {
            assert(false); return false;
        }

        D3D11_SUBRESOURCE_DATA texture_data{};
        texture_data.pSysMem = image_buffer.data;
        texture_data.SysMemPitch = image_buffer.stride;
        texture_data.SysMemSlicePitch = image_buffer.size;

        if (!win32::check_hresult_as_boolean(
            device->CreateTexture2D(&texture_info, m_mipmap ? nullptr : &texture_data, m_texture.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_info{};
        srv_info.Format = texture_info.Format;
        srv_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_info.Texture2D.MipLevels = texture_info.MipLevels;
        if (m_mipmap) {
            // See: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_tex2d_srv
            srv_info.Texture2D.MipLevels = static_cast<UINT>(-1);
        }

        if (!win32::check_hresult_as_boolean(
            device->CreateShaderResourceView(m_texture.get(), &srv_info, m_view.put()),
            "ID3D11Device::CreateTexture2D"sv
        )) {
            return false;
        }

        if (m_mipmap) {
            D3D11_BOX box{};
            box.right = texture_info.Width;
            box.bottom = texture_info.Height;
            box.back = 1;
            ctx->UpdateSubresource(m_texture.get(), 0, &box, image_buffer.data, image_buffer.stride, image_buffer.size);
            ctx->GenerateMips(m_view.get());
            ctx->Flush();
        }

        m_size.x = texture_info.Width;
        m_size.y = texture_info.Height;
        m_pre_mul_alpha = image->getAlphaMode() == ImageAlphaMode::premultiplied;
        return true;
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    bool GraphicsDevice::createTextureFromFile(StringView const path, bool const mipmap, ITexture2D** const out_texture) {
        if (out_texture == nullptr) {
            assert(false); return false;
        }
        SmartReference<Texture2D> buffer;
        buffer.attach(new Texture2D);
        if (!buffer->initialize(this, path, mipmap)) {
            return false;
        }
        *out_texture = buffer.detach();
        return true;
    }
    bool GraphicsDevice::createTextureFromImage(IImage* const image, bool const mipmap, ITexture2D** const out_texture) {
        if (image == nullptr) {
            assert(false); return false;
        }
        if (out_texture == nullptr) {
            assert(false); return false;
        }
        SmartReference<Texture2D> buffer;
        buffer.attach(new Texture2D);
        if (!buffer->initialize(this, image, mipmap)) {
            return false;
        }
        *out_texture = buffer.detach();
        return true;
    }
    bool GraphicsDevice::createTexture(Vector2U const size, ITexture2D** const out_texture) {
        if (out_texture == nullptr) {
            assert(false); return false;
        }
        SmartReference<Texture2D> buffer;
        buffer.attach(new Texture2D);
        if (!buffer->initialize(this, size, false)) {
            return false;
        }
        *out_texture = buffer.detach();
        return true;
    }
}
