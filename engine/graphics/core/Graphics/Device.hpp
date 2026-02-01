#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"
#include "core/ImmutableString.hpp"
#include "core/Image.hpp"
#include "core/GraphicsSampler.hpp"

#define LUASTG_ENABLE_DIRECT2D

namespace core::Graphics
{
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

		virtual void setSamplerState(IGraphicsSampler* p_sampler) = 0;
		// Might be nullptr
		virtual IGraphicsSampler* getSamplerState() const noexcept = 0;
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
	// https://www.luastg-sub.com/core.ITexture2D
	template<> constexpr InterfaceId getInterfaceId<Graphics::ITexture2D>() { return UUID::parse("5477054a-61c9-5071-9339-a9959e538a21"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IRenderTarget
	template<> constexpr InterfaceId getInterfaceId<Graphics::IRenderTarget>() { return UUID::parse("2753f336-07b6-5e69-95b1-46f1125531fa"); }
}
