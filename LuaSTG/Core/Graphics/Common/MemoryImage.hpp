#pragma once
#include "core/Graphics/Image.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <vector>

namespace core {
	class Image final : public implement::ReferenceCounted<IImage> {
	public:
		// IImage

		[[nodiscard]] Vector2U getSize() const noexcept override;
		[[nodiscard]] Color4B getPixel(Vector2U position) const noexcept override;
		void setPixel(Vector2U position, Color4B color) noexcept override;
		[[nodiscard]] bool clone(IImage** output_image) const override;
		[[nodiscard]] bool saveToFile(std::string_view path) const override;

		// Image

		Image() = default;
		Image(Image const&) = delete;
		Image(Image&&) = delete;
		~Image() override = default;

		Image& operator=(Image const&) = delete;
		Image& operator=(Image&&) = delete;

		[[nodiscard]] bool setSize(Vector2U size);

	private:
		std::vector<Color4B> m_pixels;
		Vector2U m_size;
	};
}
