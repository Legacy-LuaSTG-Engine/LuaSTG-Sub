#pragma once
#include <cstdint>
#include <cmath>
#include <limits>
#include "core/Vector3.hpp"

namespace core {
    template<typename T>
	struct Box {
		Vector3<T> a;
		Vector3<T> b;

		Box() noexcept = default;
		Box(Vector3<T> const& a, Vector3<T> const& b) noexcept : a(a), b(b) {}
		Box(T const left, T const top, T const front, T const right, T const bottom, T const back) noexcept : a(left, top, front), b(right, bottom, back) {}

		[[nodiscard]] bool operator==(Box const& r) const noexcept { return a == r.a && b == r.b; }
		[[nodiscard]] bool operator!=(Box const& r) const noexcept { return a != r.a || b != r.b; }
	};

	using BoxI = Box<int32_t>;
	using BoxU = Box<uint32_t>;
	using BoxF = Box<float>;
}
