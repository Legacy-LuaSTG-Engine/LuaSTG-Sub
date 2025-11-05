#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"
#include "core/ImmutableString.hpp"

#define LUASTG_ENABLE_DIRECT2D

namespace core::Graphics
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

	struct ISamplerState : IReferenceCounted
	{
	};

	struct ITexture2D : IReferenceCounted
	{
		virtual void* getNativeHandle() const noexcept = 0;

		virtual bool isDynamic() const noexcept = 0;
		virtual bool isPremultipliedAlpha() const noexcept = 0;
		virtual void setPremultipliedAlpha(bool v) = 0;
		virtual Vector2U getSize() const noexcept = 0;
		virtual bool setSize(Vector2U size) = 0;

		virtual bool uploadPixelData(RectU rc, void const* data, uint32_t pitch) = 0;
		virtual void setPixelData(IData* p_data) = 0;

		virtual bool saveToFile(StringView path) = 0;

		virtual void setSamplerState(ISamplerState* p_sampler) = 0;
		// Might be nullptr
		virtual ISamplerState* getSamplerState() const noexcept = 0;
	};

	struct IRenderTarget : IReferenceCounted
	{
		virtual void* getNativeHandle() const noexcept = 0;
		virtual void* getNativeBitmapHandle() const noexcept = 0;

		virtual bool setSize(Vector2U size) = 0;
		virtual ITexture2D* getTexture() const noexcept = 0;
	};

	struct IDepthStencilBuffer : IReferenceCounted
	{
		virtual void* getNativeHandle() const noexcept = 0;

		virtual bool setSize(Vector2U size) = 0;
		virtual Vector2U getSize() const noexcept = 0;
	};

	struct IBuffer : IReferenceCounted {
		virtual bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) = 0;
		virtual bool unmap() = 0;
	};

	struct IDevice : IReferenceCounted
	{
		virtual void addEventListener(IDeviceEventListener* e) = 0;
		virtual void removeEventListener(IDeviceEventListener* e) = 0;

		virtual DeviceMemoryUsageStatistics getMemoryUsageStatistics() = 0;

		virtual bool recreate() = 0;
		virtual void setPreferenceGpu(StringView preferred_gpu) = 0;
		virtual uint32_t getGpuCount() = 0;
		virtual StringView getGpuName(uint32_t index) = 0;
		virtual StringView getCurrentGpuName() const noexcept = 0;

		virtual void* getNativeHandle() = 0;
		virtual void* getNativeRendererHandle() = 0;

		virtual bool createVertexBuffer(uint32_t size_in_bytes, IBuffer** output) = 0;
		virtual bool createIndexBuffer(uint32_t size_in_bytes, IBuffer** output) = 0;
		virtual bool createConstantBuffer(uint32_t size_in_bytes, IBuffer** output) = 0;

		virtual bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** pp_texture) = 0;
		//virtual bool createTextureFromMemory(void const* data, size_t size, bool mipmap, ITexture2D** pp_texture) = 0;
		virtual bool createTexture(Vector2U size, ITexture2D** pp_texture) = 0;

		virtual bool createRenderTarget(Vector2U size, IRenderTarget** pp_rt) = 0;
		virtual bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** pp_ds) = 0;

		virtual bool createSamplerState(SamplerState const& info, ISamplerState** pp_sampler) = 0;

		static bool create(StringView preferred_gpu, IDevice** p_device);
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.ISamplerState
	template<> constexpr InterfaceId getInterfaceId<Graphics::ISamplerState>() { return UUID::parse("e3d354b2-5ba5-5ead-8e63-0d3516b45c05"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.ITexture2D
	template<> constexpr InterfaceId getInterfaceId<Graphics::ITexture2D>() { return UUID::parse("5477054a-61c9-5071-9339-a9959e538a21"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IRenderTarget
	template<> constexpr InterfaceId getInterfaceId<Graphics::IRenderTarget>() { return UUID::parse("2753f336-07b6-5e69-95b1-46f1125531fa"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IDepthStencilBuffer
	template<> constexpr InterfaceId getInterfaceId<Graphics::IDepthStencilBuffer>() { return UUID::parse("2c92426c-9703-57aa-88e6-073957b0d92c"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IBuffer
	template<> constexpr InterfaceId getInterfaceId<Graphics::IBuffer>() { return UUID::parse("089911ec-bd44-519e-a1e6-41a0e8e4626c"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IDevice
	template<> constexpr InterfaceId getInterfaceId<Graphics::IDevice>() { return UUID::parse("f6b65f2d-4307-597e-bc16-f3504ef89def"); }
}
