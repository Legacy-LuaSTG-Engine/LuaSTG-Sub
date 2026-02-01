#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"
#include "core/ImmutableString.hpp"
#include "core/Image.hpp"

#define LUASTG_ENABLE_DIRECT2D

namespace core::Graphics
{
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
		virtual void setImage(IImage* image) = 0;

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
}
