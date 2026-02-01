#pragma once
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"
#include "core/ImmutableString.hpp"
#include "core/Image.hpp"
#include "core/Texture2D.hpp"

#define LUASTG_ENABLE_DIRECT2D

namespace core::Graphics
{
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
	// https://www.luastg-sub.com/core.IRenderTarget
	template<> constexpr InterfaceId getInterfaceId<Graphics::IRenderTarget>() { return UUID::parse("2753f336-07b6-5e69-95b1-46f1125531fa"); }
}
