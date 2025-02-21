#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Device.hpp"

// Texture2D
namespace Core::Graphics::Direct3D11 {
	class Device;

	class Texture2D final
		: public Object<ITexture2D>
		, public IDeviceEventListener {
	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// ITexture2D

		[[nodiscard]] void* getNativeHandle() const noexcept override { return m_view.Get(); }

		[[nodiscard]] bool isDynamic() const noexcept override { return m_dynamic; }
		[[nodiscard]] bool isPremultipliedAlpha() const noexcept override { return m_pre_mul_alpha; }
		void setPremultipliedAlpha(bool const v) override { m_pre_mul_alpha = v; }
		[[nodiscard]] Vector2U getSize() const noexcept override { return m_size; }
		bool setSize(Vector2U size) override;

		bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) override;
		void setPixelData(IData* p_data) override { m_data = p_data; }

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

		[[nodiscard]] ID3D11Texture2D* GetResource() const noexcept { return m_texture.Get(); }
		[[nodiscard]] ID3D11ShaderResourceView* GetView() const noexcept { return m_view.Get(); }

		bool initialize(Device* device, StringView path, bool mipmap);
		bool initialize(Device* device, Vector2U size, bool is_render_target);
		bool createResource();

	private:
		ScopeObject<Device> m_device;
		ScopeObject<ISamplerState> m_sampler;
		ScopeObject<IData> m_data;
		std::string m_source_path;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_view;
		Vector2U m_size{};
		bool m_dynamic{ false };
		bool m_pre_mul_alpha{ false };
		bool m_mipmap{ false };
		bool m_is_render_target{ false };
		bool m_initialized{ false };
	};
}
