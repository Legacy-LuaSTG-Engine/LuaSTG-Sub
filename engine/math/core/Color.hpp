#pragma once
#include <cstdint>
#include <cmath>
#include <limits>

namespace core {
    struct alignas(uint32_t) Color4B {
		uint8_t b{};
		uint8_t g{};
		uint8_t r{};
		uint8_t a{};

		Color4B() noexcept = default;
		explicit Color4B(uint32_t const argb) noexcept { color(argb); }
		Color4B(uint8_t const r, uint8_t const g, uint8_t const b) : b(b), g(g), r(r), a(255) {}
		Color4B(uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a) : b(b), g(g), r(r), a(a) {}

		void color(uint32_t const argb) noexcept { *reinterpret_cast<uint32_t*>(this) = argb; }
		[[nodiscard]] uint32_t color() const noexcept { return *reinterpret_cast<uint32_t const*>(this); }

		Color4B& operator=(uint32_t const argb) noexcept { color(argb); return *this; }

		[[nodiscard]] bool operator==(Color4B const& right) const noexcept { return color() == right.color(); }
		[[nodiscard]] bool operator!=(Color4B const& right) const noexcept { return color() != right.color(); }

		static Color4B black() { return Color4B(0, 0, 0); }
		static Color4B transparentBlack() { return Color4B(0, 0, 0, 0); }
		static Color4B white() { return Color4B(255, 255, 255); }
		static Color4B transparentWhite() { return Color4B(255, 255, 255, 0); }
	};

	static_assert(sizeof(Color4B) == sizeof(uint32_t));
}
