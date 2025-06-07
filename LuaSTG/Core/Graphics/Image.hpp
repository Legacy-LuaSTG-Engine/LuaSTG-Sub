#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Type.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IImage : IReferenceCounted {
		[[nodiscard]] virtual Vector2U getSize() const noexcept = 0;
		[[nodiscard]] virtual Color4B getPixel(Vector2U position) const noexcept = 0;
		virtual void setPixel(Vector2U position, Color4B color) noexcept = 0;
		[[nodiscard]] virtual bool clone(IImage** output_image) const = 0;
		[[nodiscard]] virtual bool saveToFile(std::string_view path) const = 0;

		[[nodiscard]] static bool create(Vector2U size, IImage** output_image);
		[[nodiscard]] static bool loadFromFile(std::string_view path, IImage** output_image);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IImage
	template<> constexpr InterfaceId getInterfaceId<IImage>() { return UUID::parse("5e4c12e0-e094-5346-b129-b9ddbb881373"); }
}
