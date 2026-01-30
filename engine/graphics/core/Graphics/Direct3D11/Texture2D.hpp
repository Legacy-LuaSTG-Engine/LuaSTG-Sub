#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/GraphicsDevice.hpp"

// Texture2D
namespace core::Graphics::Direct3D11 {
	class Texture2D final
		: public implement::ReferenceCounted<ITexture2D>
		, public IGraphicsDeviceEventListener {
	public:
		// IGraphicsDeviceEventListener

		void onGraphicsDeviceCreate() override;
		void onGraphicsDeviceDestroy() override;

		// ITexture2D

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.get(); }

		[[nodiscard]] bool isDynamic() const noexcept override { return m_dynamic; }
		[[nodiscard]] bool isPremultipliedAlpha() const noexcept override { return m_pre_mul_alpha; }
		void setPremultipliedAlpha(bool const v) override { m_pre_mul_alpha = v; }
		[[nodiscard]] Vector2U getSize() const noexcept override { return m_size; }
		bool setSize(Vector2U size) override;

		bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) override;
		void setImage(IImage* const image) override { m_image = image; }

		bool saveToFile(StringView path) override;

		void setSamplerState(ISamplerState* p_sampler) override { m_sampler = p_sampler; }
		[[nodiscard]] ISamplerState* getSamplerState() const noexcept override { return m_sampler.get(); }

		// Texture2D

		Texture2D();
		Texture2D(Texture2D const&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(Texture2D const&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;
		~Texture2D();

		[[nodiscard]] ID3D11Texture2D* GetResource() const noexcept { return m_texture.get(); }
		[[nodiscard]] ID3D11ShaderResourceView* GetView() const noexcept { return m_view.get(); }

		bool initialize(IGraphicsDevice* device, StringView path, bool mipmap);
		bool initialize(IGraphicsDevice* device, IImage* image, bool mipmap);
		bool initialize(IGraphicsDevice* device, Vector2U size, bool is_render_target);
		bool createResource();

	private:
		SmartReference<IGraphicsDevice> m_device;
		SmartReference<ISamplerState> m_sampler;
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
