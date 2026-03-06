#pragma once
#include "GameResource/ResourceBase.hpp"
#include "core/Graphics/Font.hpp"

namespace luastg
{
	enum class FontAlignHorizontal  // 水平对齐
	{
		Left,
		Center,
		Right
	};

	enum class FontAlignVertical  // 垂直对齐
	{
		Top,
		Middle,
		Bottom
	};

	struct IResourceFont : IResourceBase
	{
		virtual core::Graphics::IGlyphManager* GetGlyphManager() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual core::Color4B GetBlendColor() = 0;
		virtual void SetBlendColor(core::Color4B c) = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceFont
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceFont>() { return UUID::parse("73b6f11f-cd6c-523f-b534-e57f9ca54526"); }
}
