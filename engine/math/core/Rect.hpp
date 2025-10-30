#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/Vector2.hpp"

namespace core {
    template<typename T>
	struct Rect {
		Vector2<T> a;
		Vector2<T> b;

		Rect() noexcept = default;
		Rect(Vector2<T> const& a, Vector2<T> const& b) noexcept : a(a), b(b) {}
		Rect(T const left, T const top, T const right, T const bottom) noexcept : a(left, top), b(right, bottom) {}

		[[nodiscard]] bool operator==(Rect const& r) const noexcept { return a == r.a && b == r.b; }
		[[nodiscard]] bool operator!=(Rect const& r) const noexcept { return a != r.a || b != r.b; }

		[[nodiscard]] Rect operator+(Vector2<T> const& r) const noexcept { return Rect(a + r, b + r); }
		[[nodiscard]] Rect operator-(Vector2<T> const& r) const noexcept { return Rect(a - r, b - r); }

		//inline Rect operator*(T const r) const noexcept { return Rect(a * r, b * r); }
	};

	using RectI = Rect<int32_t>;
	using RectU = Rect<uint32_t>;
	using RectF = Rect<float>;
}
