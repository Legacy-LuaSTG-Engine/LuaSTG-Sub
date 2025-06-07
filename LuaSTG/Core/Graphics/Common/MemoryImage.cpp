#include "core/Graphics/Common/MemoryImage.hpp"
#include "core/SmartReference.hpp"

namespace core {
	Vector2U Image::getSize() const noexcept {
		return m_size;
	}
	Color4B Image::getPixel(Vector2U const position) const noexcept {
		assert(position.x < m_size.x);
		assert(position.y < m_size.y);
		auto const index = position.y * m_size.y + position.x;
		return m_pixels[index];
	}
	void Image::setPixel(Vector2U const position, Color4B const color) noexcept {
		assert(position.x < m_size.x);
		assert(position.y < m_size.y);
		auto const index = position.y * m_size.y + position.x;
		m_pixels[index] = color;
	}
	bool Image::clone(IImage** const output_image) const {
		if (output_image == nullptr) {
			assert(false);
			return false;
		}
		try {
			SmartReference<Image> image;
			image.attach(new Image());
			image->m_pixels = m_pixels;
			image->m_size = m_size;
			*output_image = image.detach();
			return true;
		}
		catch (std::exception const&) {
			// TODO: logging
		}
		return false;
	}

	bool Image::setSize(Vector2U const size) {
		if (size.x == 0 || size.y == 0) {
			assert(false);
			return false;
		}
		try {
			m_pixels.resize(size.x * size.y);
			m_size = size;
			return true;
		}
		catch (std::exception const&) {
			// TODO: logging
		}
		return false;
	}
}
namespace core {
	bool IImage::create(Vector2U const size, IImage** const output_image) {
		if (size.x == 0 || size.y == 0) {
			assert(false);
			return false;
		}
		if (output_image == nullptr) {
			assert(false);
			return false;
		}
		try {
			SmartReference<Image> image;
			image.attach(new Image());
			if (!image->setSize(size)) {
				return false;
			}
			*output_image = image.detach();
			return true;
		}
		catch (std::exception const&) {
			// TODO: logging
		}
		return false;
	}
	bool IImage::loadFromFile(std::string_view const path, IImage** const output_image) {
		if (output_image == nullptr) {
			assert(false);
			return false;
		}
		try {
			SmartReference<Image> image;
			image.attach(new Image());
			if (!image->loadFromFile(path)) {
				return false;
			}
			*output_image = image.detach();
			return true;
		}
		catch (std::exception const&) {
			// TODO: logging
		}
		return false;
	}
}
