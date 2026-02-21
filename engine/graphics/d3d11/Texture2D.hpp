#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class Texture2D final :
        public implement::ReferenceCounted<ITexture2D>,
        public IGraphicsDeviceEventListener {
    public:
        // ITexture2D

        void* getNativeResource() const noexcept override  { return m_texture.get(); }
        void* getNativeView() const noexcept override { return m_view.get(); }

        bool isDynamic() const noexcept override { return m_dynamic; }
        bool isVideoTexture() const noexcept override { return false; }
        bool isPremultipliedAlpha() const noexcept override { return m_pre_mul_alpha; }
        void setPremultipliedAlpha(bool const v) override { m_pre_mul_alpha = v; }
        Vector2U getSize() const noexcept override { return m_size; }

        bool setSize(Vector2U size) override;
        bool update(RectU rc, void const* data, uint32_t pitch) override;
        void setImage(IImage* const image) override { m_image = image; }

        bool saveToFile(StringView path) override;

        void setSamplerState(IGraphicsSampler* p_sampler) override { m_sampler = p_sampler; }
        IGraphicsSampler* getSamplerState() const noexcept override { return m_sampler.get(); }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // Texture2D

        Texture2D();
        Texture2D(Texture2D const&) = delete;
        Texture2D(Texture2D&&) = delete;
        ~Texture2D();

        Texture2D& operator=(Texture2D const&) = delete;
        Texture2D& operator=(Texture2D&&) = delete;

        bool initialize(IGraphicsDevice* device, StringView path, bool mipmap);
        bool initialize(IGraphicsDevice* device, IImage* image, bool mipmap);
        bool initialize(IGraphicsDevice* device, Vector2U size, bool is_render_target);
        bool createResource();

    private:
        bool createTextureAndView();
        bool createFromProvidedPath();
        bool createFromImage(IData* data);
        bool createFromImage(IImage* image);

        SmartReference<IGraphicsDevice> m_device;
        SmartReference<IGraphicsSampler> m_sampler;
        SmartReference<IImage> m_image;
        std::string m_source_path;
        win32::com_ptr<ID3D11Texture2D> m_texture;
        win32::com_ptr<ID3D11ShaderResourceView> m_view;
        Vector2U m_size{};
        bool m_dynamic{ false };
        bool m_pre_mul_alpha{ false };
        bool m_mipmap{ false };
        bool m_is_render_target{ false };
        bool m_initialized{ false };
    };
}
