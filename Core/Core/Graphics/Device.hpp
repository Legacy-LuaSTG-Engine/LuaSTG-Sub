#pragma once
#include "Core/Type.hpp"

namespace Core::Graphics
{
	struct IDeviceEventListener
	{
		virtual void onDeviceCreate() = 0;
		virtual void onDeviceDestroy() = 0;
	};

	struct DeviceMemoryUsageStatistics
	{
		struct DeviceMemoryUsage
		{
			uint64_t budget;
			uint64_t current_usage;
			uint64_t available_for_reservation;
			uint64_t current_reservation;
		};
		DeviceMemoryUsage local{};
		DeviceMemoryUsage non_local{};
	};

	enum class Filter
	{
		Point,
		PointMinLinear,
		PointMagLinear,
		PointMipLinear,
		LinearMinPoint,
		LinearMagPoint,
		LinearMipPoint,
		Linear,
		Anisotropic,
	};

	enum class TextureAddressMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border,
		MirrorOnce,
	};

	enum class BorderColor
	{
		Black,
		OpaqueBlack,
		TransparentWhite,
		White,
	};

	struct SamplerState
	{
		Filter filer;
		TextureAddressMode address_u;
		TextureAddressMode address_v;
		TextureAddressMode address_w;
		float mip_lod_bias;
		uint32_t max_anisotropy;
		float min_lod;
		float max_lod;
		BorderColor border_color;
		SamplerState()
			: filer(Filter::Linear)
			, address_u(TextureAddressMode::Clamp)
			, address_v(TextureAddressMode::Clamp)
			, address_w(TextureAddressMode::Clamp)
			, mip_lod_bias(0.0f)
			, max_anisotropy(1u)
			, min_lod(-FLT_MAX)
			, max_lod(FLT_MAX)
			, border_color(BorderColor::Black)
		{}
		SamplerState(Filter filter_, TextureAddressMode address_)
			: filer(filter_)
			, address_u(address_)
			, address_v(address_)
			, address_w(address_)
			, mip_lod_bias(0.0f)
			, max_anisotropy(1u)
			, min_lod(-FLT_MAX)
			, max_lod(FLT_MAX)
			, border_color(BorderColor::Black)
		{}
	};

	struct ISamplerState : public IObject
	{
	};

	struct ITexture2D : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual bool isDynamic() = 0;
		virtual bool isPremultipliedAlpha() = 0;
		virtual void setPremultipliedAlpha(bool v) = 0;
		virtual Vector2U getSize() = 0;
		virtual bool setSize(Vector2U size) = 0;

		virtual bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) = 0;

		virtual bool saveToFile(StringView path) = 0;

		virtual void setSamplerState(ISamplerState* p_sampler) = 0;
		// Might be nullptr
		virtual ISamplerState* getSamplerState() = 0;
	};

	struct IRenderTarget : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual bool setSize(Vector2U size) = 0;
		virtual ITexture2D* getTexture() = 0;
	};

	struct IDepthStencilBuffer : public IObject
	{
		virtual void* getNativeHandle() = 0;

		virtual Vector2U getSize() = 0;
	};

	struct IDevice : public IObject
	{
		virtual void addEventListener(IDeviceEventListener* e) = 0;
		virtual void removeEventListener(IDeviceEventListener* e) = 0;

		virtual DeviceMemoryUsageStatistics getMemoryUsageStatistics() = 0;

		virtual uint32_t getGpuCount() = 0;
		virtual StringView getGpuName(uint32_t index) = 0;

		virtual void* getNativeHandle() = 0;

		virtual bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texutre) = 0;
		//virtual bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texutre) = 0;
		virtual bool createTexture(Vector2U size, ITexture2D** pp_texutre) = 0;

		virtual bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt) = 0;
		virtual bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds) = 0;

		virtual bool createSamplerState(SamplerState const& def, ISamplerState** pp_sampler) = 0;

		static bool create(StringView prefered_gpu, IDevice** p_device);
	};
}
