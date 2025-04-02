#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Font.hpp"

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
		virtual Core::Graphics::IGlyphManager* GetGlyphManager() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual Core::Color4B GetBlendColor() = 0;
		virtual void SetBlendColor(Core::Color4B c) = 0;
	};
}
